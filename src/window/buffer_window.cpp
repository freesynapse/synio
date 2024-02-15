
#include "window.h"

//
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
    bufferCursorPos();

}

//---------------------------------------------------------------------------------------
void Buffer::moveCursor(int _dx, int _dy)
{
    // save current position (to look for actual dy)
    ivec2_t prev_pos = m_cursor.pos();
    int dx = _dx;
    int dy = _dy;
    
    // moves in x at beginning or end of line
    if (prev_pos.x == 0 && _dx == -1 && m_currentLine->prev != NULL)
    {
        m_cursor.setX(m_currentLine->prev->len);
        dx = 0;
        dy = -1;
    }
    else if (prev_pos.x == m_currentLine->len && _dx == 1 && m_currentLine->next != NULL) 
    {
        m_cursor.setX(0);
        dx = 0;
        dy = 1;
    }


    // move the cursor (if possible)
    m_cursor.move(dx, dy);

    // get updated position
    ivec2_t new_pos = m_cursor.pos();

    // handle tabs
    // TODO : handle tabs here?! Yes?

    // update pointer into line buffer
    updateCurrentLinePtr(new_pos.y - prev_pos.y);

    // snap to line end if x > len(line)
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

    m_cursor.setPosition(first_ch, m_cursor.y());

}

//---------------------------------------------------------------------------------------
void Buffer::moveCursorToLineEnd()
{
    m_cursor.setPosition((int)m_currentLine->len, m_cursor.y());

}

//---------------------------------------------------------------------------------------
void Buffer::insertCharAtCursor(char _c)
{
    m_currentLine->insert_char(_c, m_cursor.x());
    m_cursor.move(1, 0);
    
}

//---------------------------------------------------------------------------------------
void Buffer::insertStrAtCursor(char *_str, size_t _len)
{
    m_currentLine->insert_str(_str, _len, m_cursor.pos().x);
    m_cursor.move(_len, 0);

}

//---------------------------------------------------------------------------------------
void Buffer::insertNewLine()
{
    // split line at cursor x pos
    line_t *new_line = m_currentLine->split_at_pos(m_cursor.pos().x);
    m_lineBuffer.insertAtPtr(m_currentLine, INSERT_AFTER, new_line);
    
    m_cursor.setX(0);
    moveCursor(0, 1);
    
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
    bufferCursorPos();

}

//---------------------------------------------------------------------------------------
void Buffer::bufferCursorPos()
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

}

//---------------------------------------------------------------------------------------
void Buffer::draw()
{
    // order matters
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

