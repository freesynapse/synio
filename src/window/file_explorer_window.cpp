
#include "file_explorer_window.h"

#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

#include "../utils/prefix_tree.h"
#include "../utils/timer.h"


//
FileExplorerWindow::FileExplorerWindow(const frame_t &_frame, 
                                       const std::string &_id, 
                                       bool _border,
                                       const std::string &_input_prompt) :
    FileBufferWindow(_frame, _id, _border, false, false)
{

    // input
    m_inputPrompt = _input_prompt;
    m_isBrowsing = false;
    m_inputFieldOffset = ivec2_t(m_inputPrompt.length(), m_frame.nrows - 1);
    if (!m_isBrowsing)
        m_cursor.set_offset(m_inputFieldOffset);
    
    // set up the renderer with an adjusted frame
    m_renderFrame = m_frame;
    m_renderFrame.v1.y -= 2;    // space for input field
    m_renderFrame.update_dims();
    m_formatter = BufferFormatter(&m_renderFrame);

    //
    getCurrentDirContents();

    // m_lineBuffer now contains the current dir listing -- set line ptrs
    m_currentListingPtr = m_lineBuffer.m_head;
    m_pageFirstLine = m_lineBuffer.m_head;
    m_currentLine = create_line("");

}

//---------------------------------------------------------------------------------------
void FileExplorerWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{
    if (!m_isBrowsing)
    {
         
        if (_ctrl_action != CtrlKeyAction::NONE)
        {
            switch (_ctrl_action)
            {
                case CtrlKeyAction::CTRL_LEFT:          break;
                case CtrlKeyAction::CTRL_RIGHT:         break;
                case CtrlKeyAction::CTRL_UP:            break;
                case CtrlKeyAction::CTRL_DOWN:          break;
                default: LOG_WARNING("ctrl keycode %d : %s", _c, ctrlActionStr(_ctrl_action)); break;
                // default: break;
            }
        }
        else
        {
            switch (_c)
            {
                // cursor movement in input field
                case KEY_LEFT:  moveCursor(-1, 0);          break;
                case KEY_RIGHT: moveCursor(1, 0);           break;
                case KEY_HOME:  moveCursorToLineBegin();    break;
                case KEY_END:   moveCursorToLineEnd();      break;

                case KEY_DC:
                    deleteCharAtCursor();
                    break;

                case KEY_BACKSPACE:
                    deleteCharBeforeCursor();
                    break;

                case 8:     // <Ctrl> + <Backspace>
                    deleteToPrevColDelim();
                    break;

                case 9:     // <TAB>
                    toggle_mode_();
                    break;
                
                case 10:    // <ENTER>
                    m_selectedFilename = std::string(m_currentLine->__debug_str);
                    break;

                default:
                    insertCharAtCursor((char)_c);
                    break;

            }
        }

    }
    else    // in browsing mode (m_isBrowsing == true)
        NOT_IMPLEMENTED();

}

//---------------------------------------------------------------------------------------
void FileExplorerWindow::redraw()
{
    if (!m_isWindowVisible)
        return;

    static int listing_x_offset = Config::FILE_DIALOG_LISTING_SPACING;

    // print listing
    m_formatter.render(m_apiWindowPtr, m_pageFirstLine, NULL, listing_x_offset);
    
    api->horizontalDivider(m_apiWindowPtr, 0, m_frame.nrows - 2, m_frame.ncols);
    api->wprint(m_apiWindowPtr, 0, m_frame.nrows - 1, "%s", m_inputPrompt.c_str());
    
    //
    api->printBufferLine(m_apiWindowPtr, m_inputFieldOffset.x, m_inputFieldOffset.y, 
                         m_currentLine);

   //if (!m_isBrowsing)
   //    m_cursor.set_offset(m_inputFieldOffset);
   //else
   //    m_cursor.set_offset({ 0 });
   //    // moveCursor(m_inputFieldOffset.x, m_inputFieldOffset.y);

    updateCursor();

}

//---------------------------------------------------------------------------------------
void FileExplorerWindow::getCurrentDirContents()
{
    
    
    m_currentDir = "/var/log";
    // m_currentDir = "/bin";


    DIR *dir_stream;
    struct dirent *dir;
    struct stat fstat;
    prefix_node_t ptree;

    dir_stream = opendir(m_currentDir.c_str());
    size_t max_fname_len = 0;
    std::string longfname = "";

    if (dir_stream == NULL)
    {
        // PrefixTree::insert_string(&ptree, "(ERROR: could not open dir '" + m_currentDir + "')");
        LOG_ERROR("could not open dir '%s')", m_currentDir.c_str());
    }
    else
    {
        // { Timer t("dir and fill prefix tree", true);
        
        while ((dir = readdir(dir_stream)) != NULL)
        {
            std::string fname = std::string(dir->d_name);
            std::string file_path = m_currentDir + '/' + fname;

            if (stat(file_path.c_str(), &fstat) != 0)
            {
                LOG_ERROR("couldn't retrieve file stats: %s", strerror(errno));
            }
            else
            {
                // insert in prefix tree (for autocomplete) and vector (for rendering)
                // TODO : do this in separate thread (for later)
                // PrefixTree::insert_string(&ptree, fname);
                
                FileEntry fe(dir->d_name, fstat.st_mode, fstat.st_size);
                m_currentDirListing.push_back(fe);

                if (fname.length() > max_fname_len)
                {
                    max_fname_len = fname.length();
                    longfname = fname;
                }

            }
        }
        // }
    }
    closedir(dir_stream);
    // LOG_RAW("longest filename = %s (%zu)", longfname.c_str(), longfname.length());

    // sort vector strings (case insensitive + smaller size comes first)
    // { Timer t("sort", true);
    std::sort(m_currentDirListing.begin(), m_currentDirListing.end(), 
             [](const FileEntry &_a, const FileEntry &_b) -> bool
             {
               for (size_t c = 0; c < _a.name.size() && c < _b.name.size(); c++) {
                   if (std::tolower(_a.name[c]) != std::tolower(_b.name[c]))
                       return (std::tolower(_a.name[c]) < std::tolower(_b.name[c]));
               }
               return (_a.name.size() < _b.name.size()); 
             }
           );
    // }
    // calculate listing column count based on longest filename
    int spacing = Config::FILE_DIALOG_LISTING_SPACING;
    int ncols = (m_frame.ncols - spacing) / (max_fname_len + spacing);
    int col_width = m_frame.ncols / ncols;
    int total_width = spacing * ncols + col_width * ncols;
    LOG_RAW("n cols = %d (%f)", ncols, (float)(m_frame.ncols - spacing) / (max_fname_len + spacing));

    // calculate number of whole rows
    int remainder = m_currentDirListing.size() % ncols;
    int nrows = m_currentDirListing.size() / ncols + (remainder ? 1 : 0);
    LOG_RAW("n rows = (%zu) %d (%d)", m_currentDirListing.size(), nrows, remainder);
    
    char line[1024];
    memset(line, 0, 1024);
    size_t nfiles = m_currentDirListing.size();
    for (int row = 0; row < nrows; row++)
    {
        memset(line, ' ', total_width);
        for (int col = 0; col < ncols; col++)
        {
            int file_idx = col * nrows + row;
            if (file_idx >= nfiles)
                continue;
            
            auto &fname = m_currentDirListing[file_idx].name;

            // copy filename into correct x loc in this line
            memcpy(line+(col*col_width ), (char *)fname.c_str(), fname.length());
        }
        m_lineBuffer.push_back(line, total_width);
    }

}

