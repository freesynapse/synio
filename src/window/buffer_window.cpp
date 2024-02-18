
#include <assert.h>

#include "window.h"

//
Buffer::Buffer(irect_t *_frame, const std::string &_id, bool _border) :
        Window(_frame, _id, _border)
{
    m_formatter = BufferFormatter(m_frame);

    // create a line numbers subwindow
    irect_t line_numbers_rect(ivec2_t(0, 0), ivec2_t(m_frame.v0.x - 2, m_frame.v1.y));
    m_lineNumbers = new LineNumbers(&line_numbers_rect, _id, _border);
    m_lineNumbers->setBuffer(this);
    if (!Config::SHOW_LINE_NUMBERS)
        m_lineNumbers->setVisibility(false);

}

//---------------------------------------------------------------------------------------
Buffer::~Buffer()
{
    delete m_lineNumbers;
    // delete m_delimiters;

}

//---------------------------------------------------------------------------------------
void Buffer::onScroll(BufferScrollEvent *_e)
{
    ivec2_t scroll = { 0, 0 };
    switch (_e->axis())
    {
        case X_AXIS:
            scroll.x = _e->dir() < 0 ? -_e->steps() : _e->steps();
            break;

        case Y_AXIS:
            int n = 0;
            int n_scrolled_y = 0;
            
            // scroll up
            if (_e->dir() < 0)
            {
                while (n < _e->steps() && m_pageFirstLine->prev != NULL)
                {
                    m_pageFirstLine = m_pageFirstLine->prev;
                    m_pageLastLine = m_pageLastLine->prev;
                    n++;
                    n_scrolled_y--;
                }

            }
            // scroll down
            else
            {
                while (n < _e->steps() && m_pageLastLine->next != NULL)
                {
                    m_pageFirstLine = m_pageFirstLine->next;
                    m_pageLastLine = m_pageLastLine->next;
                    n++;
                    n_scrolled_y++;
                }

            }

            scroll.y = n_scrolled_y;
            break;
    }

    updateCurrentLinePtr(scroll.y);

    // update scroll position and update cursor
    m_scrollPos += scroll;
    updateBufferCursorPos();
    
    if (scroll.x == 0)
        move_cursor_to_last_x_();   // hang on to previous x pos even through y scroll

}

//---------------------------------------------------------------------------------------
void Buffer::moveCursor(int _dx, int _dy)
{
    // save current position (to look for actual dy)
    ivec2_t prev_pos = m_cursor.pos();
    int dx = _dx;
    int dy = _dy;
    
    // moves in x at beginning or end of line
    if (prev_pos.x == 0 && dx < 0 && m_currentLine->prev != NULL)
    {
        m_cursor.setX(m_currentLine->prev->len);
        dx = 0;
        dy = -1;
    }
    else if (prev_pos.x == m_currentLine->len && dx > 0 && m_currentLine->next != NULL) 
    {
        m_cursor.setX(0);
        dx = 0;
        dy = 1;
    }

    // move the cursor (if possible)
    m_cursor.move(dx, dy);

    // get updated position
    ivec2_t new_pos = m_cursor.pos();

    // store x position so that we can go to the same column on up/down moves
    if (dx != 0)
        m_lastCursorX = new_pos.x;

    // handle tabs
    // TODO : handle tabs here?! Yes?

    // update pointer into line buffer
    updateCurrentLinePtr(new_pos.y - prev_pos.y);

    // try to move in x to the same column as previous line visited
    if (dy != 0)
        move_cursor_to_last_x_();

    // snap to end of line
    if (new_pos.x > m_currentLine->len)
        m_cursor.setPosition((int)m_currentLine->len, new_pos.y);

}

//---------------------------------------------------------------------------------------
void Buffer::moveCursorToLineBegin()
{
    // find first character (not tabs/spaces)
    int first_ch = 0;
    char *c = m_currentLine->content;
    while ((*c == ' ' || *c == '\t') && *c != 0)
    {
        first_ch++;
        c++;
    }

    moveCursor(-(m_cursor.x() - first_ch), 0);

}

//---------------------------------------------------------------------------------------
void Buffer::moveCursorToLineEnd()
{
    moveCursor(m_currentLine->len - m_cursor.x(), 0);

}

//---------------------------------------------------------------------------------------
void Buffer::moveCursorToColDelim(int _dir)
{
    assert(_dir == -1 || _dir == 1);

    char *p = m_currentLine->content + m_cursor.x();
    int x = (_dir < 0 ? m_cursor.x() : 0);
    bool on_delimiter = is_delimiter_(m_colDelimiters, *p);

    bool do_continue = (_dir < 0 ? x > 0 : *p != '\0');
    while (do_continue)
    {
        x = x + _dir;
        p = p + _dir;

        if (is_delimiter_(m_colDelimiters, *p))
        {
            // skip continuous delimiters
            if (on_delimiter && is_delimiter_(m_colDelimiters, *(p + _dir)))
            {
                while (is_delimiter_(m_colDelimiters, *p))
                {
                    x = x + _dir;
                    p = p + _dir;
                } 
            }

            break;
        }
        
        do_continue = (_dir < 0 ? x > 0 : *p != '\0');
    }

    int dx = (_dir < 0 ? -(m_cursor.x() - x) : x);
    moveCursor(dx, 0);

}

//---------------------------------------------------------------------------------------
void Buffer::moveCursorToRowDelim(int _dir)
{
    assert(_dir == -1 || _dir == 1);
    line_t *curr_line = m_currentLine;
    
    int y = (_dir < 0 ? m_cursor.y() : 0);
    bool do_continue = (_dir < 0 ? curr_line->prev != NULL : curr_line->next != NULL);
    while (do_continue)
    {
        curr_line = (_dir < 0 ? curr_line->prev : curr_line->next);
        y += _dir;

        if ((curr_line->len == 1 && is_delimiter_(m_rowDelimiters, curr_line->content[0])) ||
            (curr_line->len == 0))
            break;

        do_continue = (_dir < 0 ? curr_line->prev != NULL : curr_line->next != NULL);

    }

    int dy = (_dir < 0 ? -(m_cursor.y() - y) : y);
    moveCursor(0, dy);

}

//---------------------------------------------------------------------------------------
void Buffer::insertCharAtCursor(char _c)
{
    m_currentLine->insert_char(_c, m_cursor.x());
    moveCursor(1, 0);
    // m_cursor.move(1, 0);
    
}

//---------------------------------------------------------------------------------------
void Buffer::insertStrAtCursor(char *_str, size_t _len)
{
    m_currentLine->insert_str(_str, _len, m_cursor.pos().x);
    moveCursor(_len, 0);
    // m_cursor.move(_len, 0);

}

//---------------------------------------------------------------------------------------
void Buffer::insertNewLine()
{
    // split line at cursor x pos
    line_t *new_line = m_currentLine->split_at_pos(m_cursor.pos().x);
    m_lineBuffer.insertAtPtr(m_currentLine, INSERT_AFTER, new_line);
    
    int cpos_x = m_cursor.x();

    // m_cursor.setX(0);
    moveCursor(-cpos_x, 1);
    // moveCursor(0, 1);
    
    // scroll up one line since new line is inserted, keeping the number of viewable
    // lines constant
    if (m_pageLastLine->prev != NULL)
        m_pageLastLine = m_pageLastLine->prev;

}

//---------------------------------------------------------------------------------------
void Buffer::deleteCharAtCursor()
{
    // <DEL>

    if (m_cursor.pos().x == m_currentLine->len && m_currentLine->next == NULL)
        return;

    if (m_cursor.pos().x < m_currentLine->len)
        m_currentLine->delete_at(m_cursor.pos().x);
    else
    {
        m_lineBuffer.appendNextToThis(m_currentLine);
        // update page pointers
        if (m_pageLastLine->next != NULL)
            m_pageLastLine = m_pageLastLine->next;
    }
}

//---------------------------------------------------------------------------------------
void Buffer::deleteCharBeforeCursor()
{
    // <BACKSPACE>
    
    if (m_cursor.x() == 0 && m_currentLine->prev == NULL)
        return;

    if (m_cursor.x() > 0)
    {
        ivec2_t cpos = m_cursor.pos();
        m_cursor.setPosition(cpos.x - 1, cpos.y);
        m_currentLine->delete_at(cpos.x);

    }
    else // at x 0
    {
        size_t prev_len = m_currentLine->prev->len;
        m_currentLine = m_lineBuffer.appendThisToPrev(m_currentLine);
        m_cursor.setPosition(prev_len, m_cursor.y() - 1);
        
        // update page pointers
        if (m_pageLastLine->next != NULL)
            m_pageLastLine = m_pageLastLine->next;

    }

}

//---------------------------------------------------------------------------------------
void Buffer::updateCursor()
{
    m_cursor.update();
    updateBufferCursorPos();

}

//---------------------------------------------------------------------------------------
void Buffer::updateBufferCursorPos()
{
    m_bufferCursorPos = m_scrollPos + m_cursor.pos();

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
    // line pointers
    m_currentLine = m_lineBuffer.m_head;
    m_pageFirstLine = m_lineBuffer.m_head;
    m_pageLastLine = m_lineBuffer.ptrFromIdx(m_frame.nrows);

    m_filename = std::string(_filename);

    // initialize line numbers window
    if (Config::SHOW_LINE_NUMBERS)
    {
        // TODO : resize the width of LineNumbers window based on number of lines in 
        // file. 
        // int width = round(std::log10((float)m_lineBuffer.size()));
        //
    }

}

//---------------------------------------------------------------------------------------
void Buffer::draw()
{
    // order matters here
    m_lineNumbers->draw();

    #ifdef DEBUG
    int x = 110;
    int y = 0;
    __debug_print(x, y++, "cpos = (%d, %d)", m_bufferCursorPos.x, m_bufferCursorPos.y);
    __debug_print(x, y++, "spos = (%d, %d)", m_scrollPos.x , m_scrollPos.y);

    if (m_currentLine != NULL)
    {
        __debug_print(x, y++, "this line: '%s'", m_currentLine->content);
        __debug_print(x, y++, "this len:  %zu", m_currentLine->len);

        if (m_currentLine->prev != NULL)
        {
            __debug_print(x, y++, "prev line: '%s'", m_currentLine->prev->content);
            __debug_print(x, y++, "prev len:  %zu", m_currentLine->prev->len);
            
        }

    }
    #endif
    
    if (m_isWindowVisible)
        m_formatter.render(m_apiWindowPtr, m_pageFirstLine, m_pageLastLine);

}

//---------------------------------------------------------------------------------------
void Buffer::clear()
{
    m_lineNumbers->clear();

    api->clearWindow(m_apiWindowPtr);

}
//---------------------------------------------------------------------------------------
void Buffer::refresh()
{
    m_lineNumbers->refresh();   // called first to move the cursor to the buffer window
                                // on updateCursor()

    if (m_apiBorderWindowPtr)
        api->refreshBorder(m_apiBorderWindowPtr);

    api->refreshWindow(m_apiWindowPtr);

}

//---------------------------------------------------------------------------------------
void Buffer::move_cursor_to_last_x_()
{
    if (m_lastCursorX <= m_currentLine->len)
        m_cursor.setPosition(m_lastCursorX, m_cursor.y());
    else if (m_lastCursorX > m_currentLine->len)
        m_cursor.setPosition((int)m_currentLine->len, m_cursor.y());    
}

//---------------------------------------------------------------------------------------
bool Buffer::is_delimiter_(const char *_delim, char _c)
{
    for (size_t i = 0; i < strlen(_delim); i++)
        if (_c == _delim[i])
            return true;
    return false;
    
}

//---------------------------------------------------------------------------------------
void LineNumbers::draw()
{
    if (!m_isWindowVisible)
        return;

    int width = m_frame.v1.x - 1;

    int y = 0;
    int line_no = m_associatedBuffer->m_scrollPos.y + 1;

    while (y < m_frame.nrows)
        api->wprint(m_apiWindowPtr, 0, y++, "%*d", width, line_no++);

}

