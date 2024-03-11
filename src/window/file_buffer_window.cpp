
#include <assert.h>
#include <cmath>

#include "file_buffer_window.h"
#include "../platform/ncurses_colors.h"
#include "../utils/timer.h"

//
FileBufferWindow::FileBufferWindow(const frame_t &_frame,
                                   const std::string &_id,
                                   bool _border) :
    BufferWindowBase(_frame, _id, _border)
{
    m_formatter = BufferFormatter(&m_frame);
    m_cursor = Cursor(this);
    
    // Create a line numbers subwindow. In case of reading buffer from file, the width 
    // of the LineNumbers are deduced at runtime
    frame_t line_numbers_rect(ivec2_t(0, m_frame.v0.y), ivec2_t(m_frame.v0.x, m_frame.v1.y));
    m_lineNumbers = new LineNumbers(line_numbers_rect, _id+"_line_numbers", _border);
    m_lineNumbers->setBuffer(this);

    int width = 5;
    m_lineNumbers->setWidth(width);
    resize(m_frame, width);

    if (!Config::SHOW_LINE_NUMBERS)
        m_lineNumbers->setVisibility(false);

    // selection instance
    m_selection = new Selection();

}

//---------------------------------------------------------------------------------------
FileBufferWindow::~FileBufferWindow()
{
    delete m_lineNumbers;

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{
    if (_ctrl_action != CtrlKeyAction::NONE)
    {
        switch (_ctrl_action)
        {
            case CtrlKeyAction::CTRL_LEFT:  deselect_(1); moveCursorToColDelim(-1); break;
            case CtrlKeyAction::CTRL_RIGHT: deselect_(0); moveCursorToColDelim( 1); break;
            case CtrlKeyAction::CTRL_UP:    deselect_(1); moveCursorToRowDelim(-1); break;
            case CtrlKeyAction::CTRL_DOWN:  deselect_(0); moveCursorToRowDelim( 1); break;
            case CtrlKeyAction::CTRL_HOME:  deselect_(1); moveFileBegin();          break;
            case CtrlKeyAction::CTRL_END:   deselect_(0); moveFileEnd();            break;
            
            // selections
            case CtrlKeyAction::SHIFT_UP:           select_(); moveCursor(0, -1);           break;
            case CtrlKeyAction::SHIFT_DOWN:         select_(); moveCursor(0,  1);           break;
            case CtrlKeyAction::SHIFT_CTRL_LEFT:    select_(); moveCursorToColDelim(-1);    break;
            case CtrlKeyAction::SHIFT_CTRL_RIGHT:   select_(); moveCursorToColDelim( 1);    break;
            case CtrlKeyAction::SHIFT_CTRL_UP:      select_(); moveCursorToRowDelim(-1);    break;
            case CtrlKeyAction::SHIFT_CTRL_DOWN:    select_(); moveCursorToRowDelim( 1);    break;
            case CtrlKeyAction::SHIFT_CTRL_HOME:    select_(); moveFileBegin();             break;
            case CtrlKeyAction::SHIFT_CTRL_END:     select_(); moveFileEnd();               break;

            default: break;
            // default: LOG_INFO("ctrl keycode %d : %s", _c, ctrlActionStr(_ctrl_action)); break;

        }
    }
    else
    {
        bool esc_pressed = false; int c;
        switch (_c)
        {
            // cursor movement
            case KEY_DOWN:      deselect_(0); moveCursor( 0,  1);       break;
            case KEY_UP:        deselect_(1); moveCursor( 0, -1);       break;
            case KEY_LEFT:      deselect_(1); moveCursor(-1,  0);       break;
            case KEY_RIGHT:     deselect_(0); moveCursor( 1,  0);       break;
            case KEY_PPAGE:     deselect_(1); movePageUp();             break;
            case KEY_NPAGE:     deselect_(0); movePageDown();           break;
            case KEY_HOME:      deselect_(1); moveCursorToLineBegin();  break;
            case KEY_END:       deselect_(0); moveCursorToLineEnd();    break;
            case KEY_DC:        deselect_(1); deleteCharAtCursor();     break;
            case KEY_BACKSPACE: deselect_(1); deleteCharBeforeCursor(); break;
            case KEY_ENTER:
            case 10:            deselect_(); insertNewLine();           break;

            // selections
            case KEY_SLEFT:     select_(); moveCursor(-1, 0);           break;
            case KEY_SRIGHT:    select_(); moveCursor( 1, 0);           break;
            case KEY_SHOME:     select_(); moveCursorToLineBegin();     break;
            case KEY_SEND:      select_(); moveCursorToLineEnd();       break;
            case KEY_SPREVIOUS: select_(); movePageUp();                break;
            case KEY_SNEXT:     select_(); movePageDown();              break;

            //
            default:
                insertCharAtCursor((char)_c);
                break;

        }
    }

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::scroll_(int _axis, int _dir, int _steps, bool _update_current_line)
{
    ivec2_t scroll = { 0, 0 };
    switch (_axis)
    {
        case X_AXIS:
            scroll.x = _dir < 0 ? -_steps : _steps;
            m_cursor.set_scrolled_x();
            break;

        case Y_AXIS:
            int n = 0;
            int n_scrolled_y = 0;
            
            // scroll up
            if (_dir < 0)
            {
                while (n < _steps && m_pageFirstLine->prev != NULL)
                {
                    m_pageFirstLine = m_pageFirstLine->prev;
                    n++;
                    n_scrolled_y--;
                }

            }
            // scroll down
            else
            {
                while (n < _steps)
                {
                    m_pageFirstLine = m_pageFirstLine->next;
                    n++;
                    n_scrolled_y++;
                }

            }

            scroll.y = n_scrolled_y;
            m_cursor.set_scrolled_y();
            break;
    }
    
    if (_update_current_line)
        updateCurrentLinePtr(scroll.y);

    // update scroll position (cursor updated in )
    m_scrollPos += scroll;
    
    clear_next_frame_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveCursor(int _dx, int _dy)
{
    int dx = _dx;
    int dy = _dy;
    m_prevLine = m_currentLine;
    
    // cursor position
    int cx = m_cursor.cx();
    int cy = m_cursor.cy();
    
    // beggining of line, move to last char of prev
    if (cx == 0 && dx < 0 && m_currentLine->prev != NULL)
    {
        dx = m_currentLine->prev->len - cx;
        dy = -1;
    }
    // last char in line, move to first char of next
    else if (cx == m_currentLine->len && dx > 0 && m_currentLine->next != NULL)
    {
        dx = -cx;
        dy = 1;
    }
    // last char of last line, do nothing
    else if (cx == m_currentLine->len && dx > 0 && m_currentLine->next == NULL)
    {
        dx = 0;
        dy = 0;
    }

    // prevent out of bounds
    if (dy > 0 && m_currentLine->next == NULL)
        dy = 0;
        
    // move the cursor (if possible)
    m_cursor.move(dx, dy);
    ivec2_t new_pos = m_cursor.cpos();

    // update pointer into line buffer
    updateCurrentLinePtr(new_pos.y - cy);

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveCursorToLineBegin()
{
    // find first character (not tabs/spaces)
    int first_ch = 0;
    CHTYPE_PTR c = m_currentLine->content;
    while ((*c == ' ' || *c == '\t') && *c != 0)
    {
        first_ch++;
        c++;
    }

    moveCursor(-(m_cursor.cx() - first_ch), 0);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveCursorToLineEnd()
{
    moveCursor(m_currentLine->len - m_cursor.cx(), 0);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveCursorToColDelim(int _dir)
{
    if (_dir > 0 && m_cursor.cx() == m_currentLine->len - 1)    { moveCursor(1, 0); return; }
    else if (_dir > 0 && m_cursor.cx() == m_currentLine->len)   { moveCursor(1, 0); return; }
    else if (_dir < 0 && m_cursor.cx() == 1)                    { moveCursor(-1, 0); return; }
    else if (_dir < 0 && m_cursor.cx() == 0)                    { moveCursor(-1, 0); return; }

    CHTYPE_PTR p = m_currentLine->content + m_cursor.cx();
    size_t line_len = m_currentLine->len;
    int x = (_dir < 0 ? m_cursor.cx() : 0);
    bool on_delimiter = is_delimiter_(Config::COL_DELIMITERS, *p);
    bool do_continue = (_dir < 0 ? x > 0 : (*p & CHTYPE_CHAR_MASK) != 0);

    //
    while (do_continue)
    {
        x += _dir;
        p += _dir;

        // -- DEBUG
        char c = (*p & CHTYPE_CHAR_MASK);
        char c2 = (*(p + _dir) & CHTYPE_CHAR_MASK);

        // if starting on a delimiter,<>=!,,, we search for non-delimiters
        if (on_delimiter && is_delimiter_(Config::COL_DELIMITERS, *p))
        {
            while (is_delimiter_(Config::COL_DELIMITERS, *p) && do_continue)
            {
                x += _dir;
                p += _dir;

                do_continue = (_dir < 0 ? x > 0 : (*p & CHTYPE_CHAR_MASK) != 0);
            }
            x -= 1;
            break;
        }
        else if (on_delimiter && !is_delimiter_(Config::COL_DELIMITERS, *p))
        {
            // ex: 'char *p = ...'
            // find next delimiter
            while (!is_delimiter_(Config::COL_DELIMITERS, *p) && do_continue)
            {
                x += _dir;
                p += _dir;

                do_continue = (_dir < 0 ? x > 0 : (*p & CHTYPE_CHAR_MASK) != 0);
            }
            break;
        }
        else // not starting on delimiter
        {
            if (is_delimiter_(Config::COL_DELIMITERS, *p))
                break;
        }

        do_continue = (_dir < 0 ? x > 0 : (*p & CHTYPE_CHAR_MASK) != 0);
    }

    int dx = (_dir < 0 ? -(m_cursor.cx() - x) : x);
    moveCursor(dx, 0);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveCursorToRowDelim(int _dir)
{
    assert(_dir == -1 || _dir == 1);
    line_t *line = m_currentLine;
    
    int y = (_dir < 0 ? m_cursor.cy() : 0);
    bool do_continue = (_dir < 0 ? line->prev != NULL : line->next != NULL);
    while (do_continue)
    {
        line = (_dir < 0 ? line->prev : line->next);
        y += _dir;

        if (line->len == 0 || is_row_empty_(line))
            break;

        do_continue = (_dir < 0 ? line->prev != NULL : line->next != NULL);

    }

    int dy = (_dir < 0 ? -(m_cursor.cy() - y) : y);
    moveCursor(0, dy);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::movePageUp()
{
    moveCursor(0, -Config::PAGE_SIZE);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::movePageDown()
{
    int steps = Config::PAGE_SIZE;
    if (m_bufferCursorPos.y + Config::PAGE_SIZE > m_lineBuffer.lineCount() - 1)
        steps = m_lineBuffer.lineCount() - 1 - m_bufferCursorPos.y;
    moveCursor(0, steps);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveFileBegin()
{
    moveCursor(-m_cursor.cx(), -m_bufferCursorPos.y);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveFileEnd()
{
    int steps = m_lineBuffer.lineCount() - 1 - m_bufferCursorPos.y;
    // two moves are needed since the x move causes the cursor to use 'last char in line'
    // behaviour and set dy to 1... well, well..
    // another hack is needed to prevent the x move to overwrite the prev line ptr
    line_t *p = m_currentLine;
    moveCursor(0, steps);
    moveCursor(m_lineBuffer.m_tail->len - m_cursor.cx(), 0);
    m_prevLine = p;
}

//---------------------------------------------------------------------------------------
void FileBufferWindow::insertCharAtCursor(char _c)
{
    m_currentLine->insert_char(_c, m_cursor.cx());
    moveCursor(1, 0);

    if (_c == '\t' || _c == ' ')
        m_linesUpdateList.insert(m_cursor.cy());
    
    refresh_next_frame_();
    buffer_changed_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::insertStrAtCursor(char *_str, size_t _len)
{
    m_currentLine->insert_str(_str, _len, m_cursor.cpos().x);
    moveCursor(_len, 0);

    // if tabs or spaces needs update
    for (size_t i = 0; i < _len; i++)
    {
        if (_str[i] == '\t' || _str[i] == ' ')
        {
            m_linesUpdateList.insert(m_cursor.cy());
            break;
        }
    }

    refresh_next_frame_();
    buffer_changed_();
    
}

//---------------------------------------------------------------------------------------
void FileBufferWindow::insertNewLine()
{
    // split line at cursor pos.x
    line_t *new_line = m_currentLine->split_at_pos(m_cursor.cpos().x);
    m_lineBuffer.insertAtPtr(m_currentLine, INSERT_AFTER, new_line);

    // find correct indentation -- use spaces, not tabs
    // TODO :   1.  Use TabsOrSpaces Config::USE_TABS_OR_SPACES to determine tabs or
    //              spaces, for now, use spaces.
    //          2.  Also fix for backspace, so that when pressed and all whitespace,
    //              a whole TABs worth of spaces are removed per keypress.
    //
    int white_spaces = find_indentation_level_(m_currentLine);
    for (int i = 0; i < white_spaces; i++)
        new_line->insert_char(' ', 0);

    moveCursor(-m_cursor.cx() + white_spaces, 1);
    
    // redraw lines
    update_lines_after_y_(m_cursor.cy() - 1);
    refresh_next_frame_();
    buffer_changed_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::deleteCharAtCursor()
{
    // <DEL>

    if (m_cursor.cpos().x == m_currentLine->len && m_currentLine->next == NULL)
        return;

    if (m_cursor.cpos().x < m_currentLine->len)
    {
        m_currentLine->delete_at(m_cursor.cpos().x);
        m_linesUpdateList.insert(m_cursor.cy());
    }
    else
    {
        m_lineBuffer.appendNextToThis(m_currentLine);
        // redraw lines
        update_lines_after_y_(m_cursor.cy());
    }

    refresh_next_frame_();
    buffer_changed_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::deleteCharBeforeCursor()
{
    // <BACKSPACE>
    
    if (m_cursor.cx() == 0 && m_currentLine->prev == NULL) // beggining of first line
    {
        m_cursor.move(0, 0);
        refresh_next_frame_();
        return;
    }
    else if (m_cursor.cx() == 0 && m_currentLine->prev != NULL)// at x 0 and not first line
    {
        if (m_cursor.cy() == 0)
            scroll_(Y_AXIS, -1, 1, false);

        size_t prev_len = m_currentLine->prev->len;
        m_currentLine = m_lineBuffer.appendThisToPrev(m_currentLine);
        m_cursor.set_cpos(prev_len, m_cursor.cy() - 1);

        // redraw lines
        update_lines_after_y_(m_cursor.cy());

    }
    else if (m_cursor.cx() > 0) // inside line
    {
        m_cursor.move(-1, 0);
        m_currentLine->delete_at(m_cursor.cx() + 1);
        m_linesUpdateList.insert(m_cursor.cy());

    }

    refresh_next_frame_();
    buffer_changed_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::updateCursor()
{
    static ivec2_t prev_pos = m_cursor.cpos();

    // actually move the cursor
    m_cursor.update();

    //
    updateBufferCursorPos();

    //
    if (prev_pos != m_cursor.cpos())
    {
        refresh_next_frame_();
        prev_pos = m_cursor.cpos();
    }

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::gotoBufferCursorPos(const ivec2_t &_pos)
{
//    int steps = m_lineBuffer.lineCount() - 1 - m_bufferCursorPos.y;
//    // two moves are needed since the x move causes the cursor to use 'last char in line'
//    // behaviour and set dy to 1... well, well..
//    // another hack is needed to prevent the x move to overwrite the prev line ptr
//    line_t *p = m_currentLine;
//    moveCursor(0, steps);
//    moveCursor(m_lineBuffer.m_tail->len - m_cursor.cx(), 0);
//    m_prevLine = p;
//
    int dx = _pos.x - m_bufferCursorPos.x;
    int dy = _pos.y - m_bufferCursorPos.y;
    moveCursor(dx, dy);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::updateBufferCursorPos()
{
    m_prevBufferCursorPos = m_bufferCursorPos;
    m_bufferCursorPos = m_scrollPos + m_cursor.cpos();
    

    // TODO : shift+down followed by shift+up selects the whole file


    // cursor moved in buffer, and in selection mode, add selection
    if (m_prevBufferCursorPos != m_bufferCursorPos && m_isSelecting)
    {
        Timer t(__func__, false);

        assert(m_selection != NULL);

        ivec2_t prev = m_prevBufferCursorPos;
        ivec2_t curr = m_bufferCursorPos;

        // find number of chars between current and previous position
        //
        int n = 0;
        line_t *prev_ptr = m_prevLine;
        line_t *curr_ptr = m_currentLine;

        // same line
        if (prev.y == curr.y)
        {
            if (curr.x < prev.x)
                std::swap(prev, curr);
            n = curr.x - prev.x;
            if (n > 0)
                m_selection->selectLineChars(prev_ptr, prev.x, n);
        }

        // different lines -- here we need the offset into the first and last selected
        // line and the starting and ending line pointers
        else
        {
            if (prev.y > curr.y)
            {
                std::swap(prev, curr);
                std::swap(prev_ptr, curr_ptr);
            }

            m_selection->selectLines(prev_ptr, prev_ptr->len - prev.x, curr_ptr, curr.x);
        }

        //
        // if (n > 0)
            // m_selection->selectChars(prev_ptr, prev.x, n);

        //
        refresh_next_frame_();

        // LOG_INFO("selection in %f ms.", t.getDeltaTimeMs());
    }
// 
}

//---------------------------------------------------------------------------------------
void FileBufferWindow::updateCurrentLinePtr(int _dy)
{
    // m_prevLine = m_currentLine;

    // if the cursor moved, update current line in buffer
    if (_dy > 0)
    {
        for (int i = 0; i < abs(_dy); i++)
            m_currentLine = m_currentLine->next; 
    }
    else if (_dy < 0)
    {
        for (int i = 0; i < abs(_dy); i++)
            m_currentLine = m_currentLine->prev; 
    }

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::readFromFile(const std::string &_filename)
{
    FileIO::readFileIntoBuffer(_filename, &m_lineBuffer);
    
    // DEBUG
    #if defined DEBUG && defined NCURSES_IMPL
    line_t *line = m_lineBuffer.m_head;
    while (line != NULL)
    {
        #ifdef NCURSES_IMPL
        ncurses_color_substr(line, 0, line->len, SYNIO_COLOR_KEYWORD);
        #endif
        line = line->next;
    }
    #endif

    // line pointers
    m_currentLine = m_lineBuffer.m_head;
    m_pageFirstLine = m_lineBuffer.m_head;

    m_filename = std::string(_filename);

    // initialize line numbers window
    if (m_filename != "" && Config::SHOW_LINE_NUMBERS)
    {
        int width = (int)std::round(std::log10((float)m_lineBuffer.m_lineCount) + 1) + 1;
        // also leave 2 blank spaces left and 1 before the divider
        width += 4;
        m_lineNumbers->setWidth(width);
        resize(m_frame, width);

    }

    m_isDirty = false;

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::resize(frame_t _new_frame, int _left_reserved)
{
    // the _left_reserved argument is used to reserve space of the LineNumbers window
    // (eg when called from FileBufferWindow::readFromFile when we know the max number of lines in 
    // the file).
    //

    m_frame = _new_frame;
    if (_left_reserved != -1 && m_frame.v0.x != _left_reserved)
    {
        m_frame.v0.x = _left_reserved;
        m_frame.update_dims();
    }

    api->clearWindow(m_apiWindowPtr);
    api->deleteWindow(m_apiWindowPtr);
    m_apiWindowPtr = api->newWindow(&m_frame);  // resfresh called by api->newWindow()
    
    if (m_apiBorderWindowPtr)
    {
        api->clearWindow(m_apiBorderWindowPtr);
        api->deleteWindow(m_apiBorderWindowPtr);
        m_apiBorderWindowPtr = api->newBorderWindow(&m_frame);
    }

    m_lineNumbers->resize(m_frame);

    m_cursor.set_frame(m_frame);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::redraw()
{
    // order matters here
    m_lineNumbers->redraw();

    #if (defined DEBUG) & (defined NCURSES_IMPL)
    updateCursor();
    int x = 110;
    int y = 0;
    __debug_print(x, y++, "bpos  = (%d, %d)", m_bufferCursorPos.x, m_bufferCursorPos.y);
    __debug_print(x, y++, "pbpos = (%d, %d)", m_prevBufferCursorPos.x, m_prevBufferCursorPos.y);
    __debug_print(x, y++, "cpos  = (%d, %d)", m_cursor.cx(), m_cursor.cy());
    __debug_print(x, y++, "rpos  = (%d, %d)", m_cursor.rx(), m_cursor.ry());
    y++;
    __debug_print(x, y++, "last_rx = %d", m_cursor.last_rx());
    __debug_print(x, y++, "spos = (%d, %d)", m_scrollPos.x , m_scrollPos.y);
    y++;
    if (m_currentLine != NULL)
    {
        __debug_print(x, y++, "this line: '%s'", m_currentLine->__debug_str());
        __debug_print(x, y++, "this len:  %zu", m_currentLine->len);
    }
    if (m_prevLine != NULL)
    {
        __debug_print(x, y++, "prev line: '%s'", m_prevLine->__debug_str());
        __debug_print(x, y++, "prev len:  %zu", m_prevLine->len);
    }
    y++;
    if (m_pageFirstLine != NULL)    __debug_print(x, y++, "page[ 0]: '%s'", m_pageFirstLine->__debug_str());
    else                            __debug_print(x, y++, "page[ 0]: NULL");

    y++;
    __debug_print(x, y++, "selecting: %s", m_isSelecting ? "true" : "false");
    if (m_selection != NULL)
       __debug_print(x, y++, "selection count = %zu", m_selection->selectionCount());

    #endif
    
    if (m_isWindowVisible)
    {
        // check for lines that have been changed
        for (auto &line_y : m_linesUpdateList)
            api->clearBufferLine(m_apiWindowPtr, line_y, m_frame.ncols);

        m_linesUpdateList.clear();

        m_formatter.render(m_apiWindowPtr, m_pageFirstLine, NULL);
    }

    updateCursor();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::clear()
{
    m_lineNumbers->clear();

    if (m_clearNextFrame)
    {
        api->clearWindow(m_apiWindowPtr);
        m_clearNextFrame = false;
    }

}
//---------------------------------------------------------------------------------------
void FileBufferWindow::refresh()
{
    m_lineNumbers->refresh();   // called first to move the cursor to the buffer window
                                // on updateCursor()

    if (m_refreshNextFrame)
    {
        if (m_apiBorderWindowPtr)
            api->refreshBorder(m_apiBorderWindowPtr);

        api->refreshWindow(m_apiWindowPtr);

        m_refreshNextFrame = false;
    }

}

