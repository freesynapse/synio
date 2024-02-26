
#include <assert.h>
#include <cmath>

#include "buffer_window.h"
#include "../platform/ncurses_colors.h"

//
Buffer::Buffer(const frame_t &_frame, const std::string &_id, bool _border) :
    Window(_frame, _id, _border)
{
    m_formatter = BufferFormatter(&m_frame);
    m_cursor = Cursor(this);
    
    // create a line numbers subwindow
    // TODO : auto-deduct width
    frame_t line_numbers_rect(ivec2_t(0, m_frame.v0.y), ivec2_t(m_frame.v0.x, m_frame.v1.y));
    m_lineNumbers = new LineNumbers(line_numbers_rect, _id, _border);
    m_lineNumbers->setBuffer(this);
    if (!Config::SHOW_LINE_NUMBERS)
        m_lineNumbers->setVisibility(false);
}

//---------------------------------------------------------------------------------------
Buffer::~Buffer()
{
    delete m_lineNumbers;

}

//---------------------------------------------------------------------------------------
void Buffer::scroll_(int _axis, int _dir, int _steps, bool _update_current_line)
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

    // update scroll position and update cursor
    m_scrollPos += scroll;
    updateBufferCursorPos();
    
    clear_next_frame_();

}

//---------------------------------------------------------------------------------------
void Buffer::moveCursor(int _dx, int _dy)
{
    int dx = _dx;
    int dy = _dy;
    
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
void Buffer::moveCursorToLineBegin()
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
void Buffer::moveCursorToLineEnd()
{
    moveCursor(m_currentLine->len - m_cursor.cx(), 0);

}

//---------------------------------------------------------------------------------------
void Buffer::moveCursorToColDelim(int _dir)
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
void Buffer::moveCursorToRowDelim(int _dir)
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
void Buffer::pageUp()
{
    moveCursor(0, -Config::PAGE_SIZE);

}

//---------------------------------------------------------------------------------------
void Buffer::pageDown()
{
    int steps = Config::PAGE_SIZE;
    if (m_bufferCursorPos.y + Config::PAGE_SIZE > m_lineBuffer.lineCount() - 1)
        steps = m_lineBuffer.lineCount() - 1 - m_bufferCursorPos.y;
    moveCursor(0, steps);

}

//---------------------------------------------------------------------------------------
void Buffer::moveHome()
{
    moveCursor(-m_cursor.cx(), -m_bufferCursorPos.y);

}

//---------------------------------------------------------------------------------------
void Buffer::moveEnd()
{
    int steps = m_lineBuffer.lineCount() - 1 - m_bufferCursorPos.y;
    moveCursor(m_lineBuffer.m_tail->len - m_cursor.cx(), steps);

}

//---------------------------------------------------------------------------------------
void Buffer::insertCharAtCursor(char _c)
{
    m_currentLine->insert_char(_c, m_cursor.cx());
    moveCursor(1, 0);

    if (_c == '\t' || _c == ' ')
        m_linesUpdateList.insert(m_cursor.cy());
    
    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void Buffer::insertStrAtCursor(char *_str, size_t _len)
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

}

//---------------------------------------------------------------------------------------
void Buffer::insertNewLine()
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

}

//---------------------------------------------------------------------------------------
void Buffer::deleteCharAtCursor()
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

}

//---------------------------------------------------------------------------------------
void Buffer::deleteCharBeforeCursor()
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
        // m_cursor.move(prev_len, -1);

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

}

//---------------------------------------------------------------------------------------
void Buffer::updateCursor()
{
    static ivec2_t prev_pos = m_cursor.cpos();

    // actually move the cursor
    m_cursor.update();

    // // retain last x position
    // if (m_cursor.dy() != 0)
    //     move_cursor_to_last_x_();

    updateBufferCursorPos();


    if (prev_pos != m_cursor.cpos())
    {
        refresh_next_frame_();
        prev_pos = m_cursor.cpos();
    }

}

//---------------------------------------------------------------------------------------
void Buffer::updateBufferCursorPos()
{
    m_bufferCursorPos = m_scrollPos + m_cursor.cpos();

}

//---------------------------------------------------------------------------------------
void Buffer::updateCurrentLinePtr(int _dy)
{
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
void Buffer::readFromFile(const std::string &_filename)
{
    FileIO::readFileIntoBuffer(_filename, &m_lineBuffer);
    
    // DEBUG
    #if defined DEBUG && defined NCURSES_IMPL
    line_t *line = m_lineBuffer.m_head;
    while (line != NULL)
    {
        color_substr(line, 0, line->len, SYNIO_COLOR_KEYWORD);
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

}

//---------------------------------------------------------------------------------------
void Buffer::resize(frame_t _new_frame, int _left_reserved)
{
    // the _left_reserved argument is used to reserve space of the LineNumbers window
    // (eg when called from Buffer::readFromFile when we know the max number of lines in 
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

}

//---------------------------------------------------------------------------------------
void Buffer::redraw()
{
    // order matters here
    m_lineNumbers->redraw();

    #if (defined DEBUG) & (defined NCURSES_IMPL)
    updateCursor();
    int x = 110;
    int y = 0;
    __debug_print(x, y++, "bpos = (%d, %d)", m_bufferCursorPos.x, m_bufferCursorPos.y);
    __debug_print(x, y++, "cpos = (%d, %d)", m_cursor.cx(), m_cursor.cy());
    __debug_print(x, y++, "rpos = (%d, %d)", m_cursor.rx(), m_cursor.ry());
    y++;
    __debug_print(x, y++, "last_rx = %d", m_cursor.last_rx());
    __debug_print(x, y++, "spos = (%d, %d)", m_scrollPos.x , m_scrollPos.y);
    y++;
    if (m_currentLine != NULL)
    {
        __debug_print(x, y++, "this line: '%s'", m_currentLine->__debug_str());
        __debug_print(x, y++, "this len:  %zu", m_currentLine->len);

    }
    y++;
    if (m_pageFirstLine != NULL)    __debug_print(x, y++, "page[ 0]: '%s'", m_pageFirstLine->__debug_str());
    else                            __debug_print(x, y++, "page[ 0]: NULL");
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
void Buffer::clear()
{
    m_lineNumbers->clear();

    if (m_clearNextFrame)
    {
        api->clearWindow(m_apiWindowPtr);
        m_clearNextFrame = false;
    }

}
//---------------------------------------------------------------------------------------
void Buffer::refresh()
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

//---------------------------------------------------------------------------------------
bool Buffer::is_delimiter_(const char *_delim, CHTYPE _c)
{
    for (size_t i = 0; i < strlen(_delim); i++)
        if ((_c & CHTYPE_CHAR_MASK) == _delim[i])
            return true;
    return false;
    
}

//---------------------------------------------------------------------------------------
bool Buffer::is_row_empty_(line_t *_line)
{
    CHTYPE_PTR p = _line->content;
    while ((*p & CHTYPE_CHAR_MASK) != '\0')
    {
        if ((*p & CHTYPE_CHAR_MASK) != ' ' && 
            (*p & CHTYPE_CHAR_MASK) != '\t')
            return false;
        p++;
    }

    return true;

}

//---------------------------------------------------------------------------------------
int Buffer::find_indentation_level_(line_t *_line)
{
    int first_char_idx = find_first_non_empty_char_(_line);
    
    if ((_line->content[first_char_idx] & CHTYPE_CHAR_MASK) == '{')
        return first_char_idx + Config::TAB_SIZE;
    
    return first_char_idx;
        
}

//---------------------------------------------------------------------------------------
int Buffer::find_first_non_empty_char_(line_t *_line)
{
    CHTYPE_PTR p = _line->content;
    int x = 0;
    while (((*p & CHTYPE_CHAR_MASK) != '\0'))
    {
        if ((*p & CHTYPE_CHAR_MASK) == ' ')
            x++;
        else if ((*p & CHTYPE_CHAR_MASK) == '\t')
            x += Config::TAB_SIZE;
        else if ((*p & CHTYPE_CHAR_MASK) != ' ' && 
                 (*p & CHTYPE_CHAR_MASK) != '\t')
            break;

        p++;
    }

    return x;

}
