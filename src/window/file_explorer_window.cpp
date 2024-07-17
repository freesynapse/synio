
#include "file_explorer_window.h"

#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

#ifdef NCURSES_IMPL
#include "../platform/ncurses_colors.h"
#endif


//
FileExplorerWindow::FileExplorerWindow(const frame_t &_frame, 
                                       const std::string &_id, 
                                       int _wnd_params,
                                       WindowCallback _callback,
                                       const std::string &_input_prompt) :
    MLineInputWindow(_frame, _id, _wnd_params)
{
    //
    m_callback = _callback;
    m_currentPath = std::filesystem::current_path();
    m_inputPrompt = _input_prompt;
    m_inputFieldOffset = ivec2_t(m_inputPrompt.length(), m_frame.nrows - 1);

    // set cursor offset (under current dir header)
    m_cursor.set_offset(ivec2_t(0, 1));
    m_cursor.set_frame(m_renderFrame);
    m_listingOffset.x = Config::FILE_DIALOG_LISTING_SPACING;

    //
    getCurrentDirContents();

    moveCursor(Config::FILE_DIALOG_LISTING_SPACING, 0);
    updateCursor();
    
}

//---------------------------------------------------------------------------------------
void FileExplorerWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{

    if (_ctrl_action != CtrlKeyAction::NONE)
    {
        moveCursor(0, 0);
        //switch (_ctrl_action)
        //{
        //    default: moveCursor(0, 0); break;
        //}
    }
    else
    {
        std::filesystem::path selected = m_currentPath;
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
            case 8:         clear_input_(); moveCursor(0, 0);           break;
            
            case KEY_BACKSPACE:
                popCharFromInput();
                moveCursor(0, 0);
                break;

            case 9:     // <TAB>
                autocompleteInput();
                moveCursor(0, 0);
                break;
            
            // TODO : move to separate function for clenliness?
            case 10:    // <ENTER>
                if (m_inputLine->len > 0)
                    m_selectedFilename = std::string(m_inputLine->__debug_str);
                else
                {
                    int fname_offset = get_listing_offset_from_x_(m_cursor.cx());
                    m_selectedFilename = m_currentDirListing[fname_offset].name;
                }

                //
                selected /= m_selectedFilename;

                //
                if (FileIO::is_file_dir(selected))
                {
                    if (m_selectedFilename == "..")
                        m_currentPath = m_currentPath.parent_path();
                    else if (m_selectedFilename == ".")
                    {
                        m_selectedFilename = "";
                        moveCursor(0, 0);
                        break;
                    }
                    else
                        m_currentPath = selected;

                    m_selectedFilename = "";
                    getCurrentDirContents();
                    if (m_errorMsg == "")
                    {
                        // reset cursor in new dir listing
                        moveCursor(-m_cursor.cx() + Config::FILE_DIALOG_LISTING_SPACING, 
                                    -m_cursor.cy());
                    }
                    // something was wrong, and parent of selected is set to current, 
                    // so read again
                    else
                        getCurrentDirContents(false);

                    clear_input_();
                    clear_next_frame_();
                    refresh_next_frame_();
                }
                // else m_selectedFilename is set, signaling to caller
                else
                {
                    m_selectedFilename = std::string((m_currentPath / m_selectedFilename).c_str());
                    m_callback(m_selectedFilename);
                }
                    
                break;

            default:
                pushCharToInput(_c);
                break;

        }
    }

}

//---------------------------------------------------------------------------------------
void FileExplorerWindow::redraw()
{
    if (!m_isWindowVisible)
        return;

    // print current directory
    api->clearSpace(m_apiWindowPtr, 0, 0, m_frame.ncols);
    api->enableAttr(m_apiWindowPtr, A_BOLD);
    int n = api->wprint(m_apiWindowPtr, 0, 0, "Directory: %s", m_currentPath.c_str());
    if (m_errorMsg != "")
    {
        api->enableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_ERROR));
        api->wprint(m_apiWindowPtr, m_frame.ncols - m_errorMsg.length() - 2, 0, "%s", m_errorMsg.c_str());
        api->disableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_ERROR));
    }
    api->disableAttr(m_apiWindowPtr, A_BOLD);

    // print listing
    m_formatter.render(m_apiWindowPtr, m_pageFirstLine, NULL, m_listingOffset.x, 1);
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
        
        if (m_autocompleteLine.length() > 0)
        {
            api->enableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_INACTIVE));
            int x_offset = m_inputFieldOffset.x + m_inputLine->len + 1;
            api->wprint(m_apiWindowPtr, x_offset, m_inputFieldOffset.y, "%s", m_autocompleteLine.c_str());
            api->disableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_INACTIVE));
        }

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
void FileExplorerWindow::moveToColRow(int _col, int _row)
{
    if (_col == m_currentCol)
    {
        moveCursor(0, 0);
    }

    int x = m_bufferCursorPos.x;
    int next_x = _col * m_colWidth + Config::FILE_DIALOG_LISTING_SPACING;
    
    int y = m_bufferCursorPos.y;
    int next_y = _row;

    // handles both when next x is larger and smaller than current buffer pos
    moveCursor(next_x - x, next_y - y);

    m_currentCol = _col;

}

//---------------------------------------------------------------------------------------
void FileExplorerWindow::findCompletions()
{
    std::string input = std::string(m_inputLine->__debug_str);
    prefix_node_t *stree = PrefixTree::find_subtree(m_ptree, input);

    m_autocompletions.clear();
    PrefixTree::find_completions(stree, &m_autocompletions, input);

    //
    m_autocompleteLine = "";
    int space_left = m_frame.ncols - m_inputLine->len - m_inputPrompt.length() - 1;

    // Only one match, let's jump to that file in the listing
    if (m_autocompletions.size() == 1 && m_autocompletions[0] == input)
    {
        auto it = std::find_if(m_currentDirListing.begin(), m_currentDirListing.end(), 
                                [input](const FileEntry &_fe) {
                                    return _fe.name == input;
                                });
        if (it != m_currentDirListing.end())
        {
            size_t idx = it - m_currentDirListing.begin();
            int col_no = idx / m_nrows;
            int line_no = idx - col_no * m_nrows;
            moveToColRow(col_no, line_no);

        }
        refresh_next_frame_();
        return;
    }

    // Multiple matches; fill line with as many as possible
    int n_entered = 0;
    bool done = false;
    for (size_t i = 0; i < m_autocompletions.size() && !done; i++)
    {
        std::string ac = m_autocompletions[i];

        for (char c : ac)
        {
            m_autocompleteLine += c;
            n_entered++;
            if (space_left - n_entered <= 3)
            {
                m_autocompleteLine += "...";
                done = true;
                break;
            }
        }

        m_autocompleteLine += ' ';
        n_entered++;

    }

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void FileExplorerWindow::getCurrentDirContents(bool _reset_error)
{
    DIR *dir_stream;
    struct dirent *dir;
    struct stat fstat;

    // clear existing variables (if called again on new dir)
    m_currentDirListing.clear();
    m_lineBuffer.clear();
    delete m_ptree;    
    m_ptree = new prefix_node_t;

    size_t max_fname_len = 0;
    std::string longfname = "";

    // return success is accessed through m_errorMsg, not return code
    if (_reset_error)
        m_errorMsg = "";

    dir_stream = opendir(m_currentPath.c_str());

    if (dir_stream == NULL)
    {
        LOG_ERROR("could not open dir '%s' (%s)", m_currentPath.c_str(), strerror(errno));
        m_errorMsg = "ERROR: reading '" + std::string(m_currentPath) + "'<" + std::string(strerror(errno)) + ">";
        m_currentPath = m_currentPath.parent_path();
        return;
    }
    else
    {
        int n_error = 0;
        while ((dir = readdir(dir_stream)) != NULL)
        {
            std::string fname = std::string(dir->d_name);
            std::string file_path = m_currentPath / fname;

            if (stat(file_path.c_str(), &fstat) != 0)
            {
                LOG_ERROR("couldn't retrieve file stats: %s", strerror(errno));
            }
            else
            {
                // insert in prefix tree (for autocomplete) and vector (for rendering)
                // TODO : ptree insert in separate thread? (for later)
                PrefixTree::insert_string(m_ptree, fname);
                FileEntry fe(fname, fstat.st_mode, fstat.st_size);
                m_currentDirListing.push_back(fe);

                // record longest filename for splitting into columns
                if (fname.length() > max_fname_len)
                {
                    max_fname_len = fname.length();
                    longfname = fname;
                }

            }
        }
    }

    closedir(dir_stream);

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

    // calculate number of whole rows
    int remainder = m_currentDirListing.size() % m_ncols;
    m_nrows = m_currentDirListing.size() / m_ncols + (remainder ? 1 : 0);

    // TODO : I will refactor this later...
    if (m_currentDirListing.size() < m_renderFrame.nrows)
    {
        m_ncols = 1;
        m_nrows = m_currentDirListing.size();
    }

    //
    CHTYPE line_CHTYPE[512] = { 0 };
    line_t *formatted_line;
    size_t nfiles = m_currentDirListing.size();
    
    //
    for (int row = 0; row < m_nrows; row++)
    {        
        memsetCHTYPE(line_CHTYPE, ' ', total_width);

        for (int col = 0; col < m_ncols; col++)
        {
            int file_idx = col * m_nrows + row;
            if (file_idx >= nfiles)
                continue;

            FileEntry entry = m_currentDirListing[file_idx];
            std::string &fname = entry.name;

            // copy filename into correct x loc in this line (using CHTYPE at this point 
            // to be able to color and bold files)
            int offset_x = col * m_colWidth;
            memcpyCharToCHTYPE(line_CHTYPE + offset_x, (char *)fname.c_str(), fname.length());
            // coloring
            if (entry.is_dir())
            {
                size_t offset_x2 = offset_x + fname.length();
                ncurses_color_substr_raw(line_CHTYPE, offset_x, offset_x2, SYN_COLOR_DIR);
                ncurses_enable_attribute(line_CHTYPE, offset_x, offset_x2, A_BOLD);
            }
            // executable?
            else if (entry.mode & S_IXUSR)
            {
                size_t offset_x2 = offset_x + fname.length();
                ncurses_color_substr_raw(line_CHTYPE, offset_x, offset_x2, SYN_COLOR_STRING);
                ncurses_enable_attribute(line_CHTYPE, offset_x, offset_x2, A_BOLD);
            }

        }
        // formatted_line = create_line(line_CHTYPE, total_width);
        m_lineBuffer.push_back(line_CHTYPE, total_width);

    }

    // set line ptrs
    m_currentLine = m_lineBuffer.m_head;
    m_pageFirstLine = m_lineBuffer.m_head;

    // reset current col
    m_currentCol = 0;

}

