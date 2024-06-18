
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
    // default state
    m_isBrowsing = true;

    // state-dependet init
    m_inputPrompt = _input_prompt;
    m_inputFieldOffset = ivec2_t(m_inputPrompt.length(), m_frame.nrows - 1);
    // m_browsingCursorOffset = ivec2_t(Config::FILE_DIALOG_LISTING_SPACING, 0);

    // set cursor limits
    if (!m_isBrowsing)
        m_cursor.set_offset(m_inputFieldOffset);
    // else
    //     m_cursor.set_offset(m_browsingCursorOffset);

    
    // set up the renderer with an adjusted frame
    m_renderFrame = m_frame;
    m_renderFrame.v1.y -= 2;    // space for input field
    m_renderFrame.update_dims();
    m_formatter = BufferFormatter(&m_renderFrame);
    if (m_isBrowsing)
        m_cursor.set_frame(m_renderFrame);

    //
    getCurrentDirContents();

    // m_lineBuffer now contains the current dir listing -- set line ptrs
    m_currentLine = m_lineBuffer.m_head;
    m_pageFirstLine = m_lineBuffer.m_head;
    m_inputLine = create_line("");

    moveCursor(Config::FILE_DIALOG_LISTING_SPACING, 0);

}

//---------------------------------------------------------------------------------------
FileExplorerWindow::~FileExplorerWindow()
{
    delete m_inputLine;

}

//---------------------------------------------------------------------------------------
void FileExplorerWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{
    if (m_isBrowsing)
    {
        if (_ctrl_action != CtrlKeyAction::NONE)
        {
            switch (_ctrl_action)
            {
                //case CtrlKeyAction::CTRL_LEFT:  moveCursor(0, 0); break;
                //case CtrlKeyAction::CTRL_RIGHT: moveCursor(0, 0); break;
                //case CtrlKeyAction::CTRL_UP:    moveCursor(0, 0); break;
                //case CtrlKeyAction::CTRL_DOWN:  moveCursor(0, 0); break;
                // default: LOG_WARNING("ctrl keycode %d : %s", _c, ctrlActionStr(_ctrl_action)); break;
                default: moveCursor(0, 0); break;
            }
        }
        else
        {
            switch (_c)
            {
                // cursor movement in input field
                case KEY_LEFT:  clear_input_(); moveCursorColumn(-1);       break;
                case KEY_RIGHT: clear_input_(); moveCursorColumn( 1);       break;
                case KEY_UP:    clear_input_(); moveCursor(0, -1);          break;
                case KEY_DOWN:  clear_input_(); moveCursor(0,  1);          break;
                case KEY_PPAGE: clear_input_(); movePageUp();               break;
                case KEY_NPAGE: clear_input_(); movePageDown();             break;
                case KEY_HOME:  clear_input_(); moveCursorToLineBegin();    break;
                case KEY_END:   clear_input_(); moveCursorToLineEnd();      break;
                case 8:         clear_input_(); break;
                case KEY_BACKSPACE:
                    popCharFromInput();
                    break;

                case 9:     // <TAB>
                    // autocomplete?
                    break;
                
                case 10:    // <ENTER>
                    m_selectedFilename = std::string(m_currentLine->__debug_str);
                    break;

                default:
                    pushCharToInput(_c);
                    break;

            }
        }
    }

    else
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
    
    // print input
    //
    api->clearBufferLine(m_apiWindowPtr, m_inputFieldOffset.y, m_frame.v1.x);
    api->wprint(m_apiWindowPtr, 0, m_frame.nrows - 1, "%s", m_inputPrompt.c_str());

    // 1st get manual input if available
    if (m_inputLine->len > 0)
    {
        api->printBufferLine(m_apiWindowPtr, m_inputFieldOffset.x, m_inputFieldOffset.y,
                             m_inputLine);
        api->printCursorBlock(m_apiWindowPtr, m_inputFieldOffset.x + m_inputLine->len, 
                              m_inputFieldOffset.y);
    }
    // otherwise get it from the last selected entry in the dir listing
    else
    {
        int fname_offset = get_listing_offset_from_x_(m_cursor.cx());
        std::string input = m_currentDirListing[fname_offset].name;
        api->enableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_SEL_TEXT));
        api->wprint(m_apiWindowPtr, m_inputFieldOffset.x, m_inputFieldOffset.y, "%s", 
                    input.c_str());
        api->printCursorBlock(m_apiWindowPtr, m_inputFieldOffset.x + input.length(), 
                              m_inputFieldOffset.y);
        api->disableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_SEL_TEXT));
    }

    updateCursor();

}

//---------------------------------------------------------------------------------------
void FileExplorerWindow::pushCharToInput(char _c)
{
    // only allowed characters (for now)
    if (Config::ALLOWED_CHAR_SET.find(_c) == Config::ALLOWED_CHAR_SET.end())
        return;

    m_inputLine->insert_char(_c, m_inputLine->len);
    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void FileExplorerWindow::popCharFromInput()
{
    if (m_inputLine->len == 0)
        return;

    m_inputLine->delete_at(m_inputLine->len);
    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void FileExplorerWindow::moveCursorColumn(int _dcol)
{
    int next_col = m_currentCol + _dcol;
    
    // first column -- move up one line and to last column
    if (next_col < 0 && m_currentLine->prev != NULL)
    {
        int to_col = m_ncols - 1;
        moveCursor(to_col * m_colWidth, -1);
        m_currentCol = to_col;
    }
    // last column -- move down one line and to first column
    else if (next_col >= m_ncols && m_currentLine->next != NULL)
    {
        int to_col = 0;
        moveCursor(-m_currentCol * m_colWidth, 1);
        m_currentCol = to_col;
    }
    else if (next_col >= 0 && next_col < m_ncols)
    {
        int move_x = _dcol * m_colWidth;
        int next_x = m_cursor.cx() + move_x;
        if (get_listing_offset_from_x_(next_x) < m_currentDirListing.size())
        {
            moveCursor(move_x, 0);
            m_currentCol += _dcol;
        }
        else
            moveCursor(0, 0);
    }
    else
        moveCursor(0, 0);

    
    // moveCursor handles refresh
    // refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void FileExplorerWindow::getCurrentDirContents()
{
    // m_currentDir = "/var/log";
    m_currentDir = "/bin";

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
                PrefixTree::insert_string(&ptree, fname);
                
                if (S_ISDIR(fstat.st_mode)) fname += '/';
                // FileEntry fe(dir->d_name, fstat.st_mode, fstat.st_size);
                FileEntry fe(fname, fstat.st_mode, fstat.st_size);
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

    // calculate listing column count based on longest filename
    int spacing = Config::FILE_DIALOG_LISTING_SPACING;
    m_ncols = (m_frame.ncols - spacing) / (max_fname_len + spacing);
    m_colWidth = m_frame.ncols / m_ncols;
    int total_width = spacing * m_ncols + m_colWidth * m_ncols;
    // LOG_RAW("n cols = %d (%f)", ncols, (float)(m_frame.ncols - spacing) / (max_fname_len + spacing));

    // calculate number of whole rows
    int remainder = m_currentDirListing.size() % m_ncols;
    m_nrows = m_currentDirListing.size() / m_ncols + (remainder ? 1 : 0);
    // LOG_RAW("n rows = (%zu) %d (%d)", m_currentDirListing.size(), nrows, remainder);
    
    char line[1024];
    memset(line, 0, 1024);
    size_t nfiles = m_currentDirListing.size();
    for (int row = 0; row < m_nrows; row++)
    {
        memset(line, ' ', total_width);
        for (int col = 0; col < m_ncols; col++)
        {
            int file_idx = col * m_nrows + row;
            if (file_idx >= nfiles)
                continue;
            
            auto &fname = m_currentDirListing[file_idx].name;

            // copy filename into correct x loc in this line
            memcpy(line + (col * m_colWidth ), (char *)fname.c_str(), fname.length());
        }
        m_lineBuffer.push_back(line, total_width);
    }

}

