
#include "window.h"

//
Window::Window(irect_t *_frame, const std::string &_id, bool _border)
{
    m_frame = *_frame;
    m_ID = _id;

    m_apiWindowPtr = api->newWindow(&m_frame);
    if (_border)
        m_apiBorderWindowPtr = api->newBorderWindow(&m_frame);

    m_formatter = BufferFormatter(m_frame);
    m_cursor = WCursor(this);

    LOG_INFO("'%s' [%p] created.", m_ID.c_str(), this);


}

//---------------------------------------------------------------------------------------
Window::~Window() 
{ 
    if (m_apiWindowPtr != NULL) api->deleteWindow(m_apiWindowPtr);
    if (m_apiBorderWindowPtr != NULL) api->deleteWindow(m_apiBorderWindowPtr);
}

//---------------------------------------------------------------------------------------
void Window::enableBorder()
{
    m_apiBorderWindowPtr = api->newBorderWindow(&m_frame);

}

//---------------------------------------------------------------------------------------
void Window::moveCursor(int _dx, int _dy)
{
    m_cursor.move(_dx, _dy);

}

//---------------------------------------------------------------------------------------
void Window::moveCursorToLineBegin()
{
    int y = m_cursor.pos().y;
    m_cursor.setPosition(0, y);

}

//---------------------------------------------------------------------------------------
void Window::moveCursorToLineEnd()
{
    int y = m_cursor.pos().y;
    m_cursor.setPosition(m_frame.ncols, y);

}

//---------------------------------------------------------------------------------------
void Window::updateCursor()
{
    m_cursor.update();

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
    bufferCursorPos();

}

//---------------------------------------------------------------------------------------
void Buffer::moveCursor(int _dx, int _dy)
{
    // save current position (to look for actual dy)
    ivec2_t prev_pos = m_cursor.pos();
    // move the cursor (if possible)
    m_cursor.move(_dx, _dy);
    // get updated position
    ivec2_t new_pos = m_cursor.pos();
    
    updateCurrentLinePtr(new_pos.y - prev_pos.y);

    // snap to line if x > len(line)
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

    int y = m_cursor.pos().y;
    m_cursor.setPosition(first_ch, y);

}

//---------------------------------------------------------------------------------------
void Buffer::moveCursorToLineEnd()
{
    int y = m_cursor.pos().y;
    m_cursor.setPosition((int)m_currentLine->len, y);

}

//---------------------------------------------------------------------------------------
void Buffer::insertCharAtCursor(char _c)
{
    m_currentLine->insert_char(_c, m_cursor.pos().x);
    m_cursor.move(1, 0);
    
}

//---------------------------------------------------------------------------------------
void Buffer::insertStrAtCursor(char *_str, size_t _len)
{
    m_currentLine->insert_str(_str, _len, m_cursor.pos().x);
    m_cursor.move(_len, 0);

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
    
    if (m_cursor.pos().x == 0 && m_currentLine->prev == NULL)
        return;

    if (m_cursor.pos().x > 0)
    {
        ivec2_t cpos = m_cursor.pos();
        m_cursor.setPosition(cpos.x - 1, cpos.y);
        m_currentLine->delete_at(cpos.x);

    }
    else // at x 0
    {
        size_t prev_len = m_currentLine->prev->len;
        m_currentLine = m_lineBuffer.appendThisToPrev(m_currentLine);
        m_cursor.setPosition(prev_len, m_cursor.pos().y - 1);
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
void Buffer::readFromFile(const char *_filename)
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
    #define n_ 256
    char b[n_];
    memset(b, 0, n_);
    snprintf(b, n_, "cpos = (%d, %d)", m_bufferCursorPos.x, m_bufferCursorPos.y);
    mvwprintw((WINDOW *)m_apiWindowPtr, 10, 120, "%s", b);
    memset(b, 0, n_);
    snprintf(b, n_, "spos = (%d, %d)", m_scrollPos.x , m_scrollPos.y);
    mvwprintw((WINDOW *)m_apiWindowPtr, 11, 120, "%s", b);
    
    if (m_currentLine != NULL)
    {
        memset(b, 0, n_);
        snprintf(b, n_, "line: %s", m_currentLine->content);
        mvwprintw((WINDOW *)m_apiWindowPtr, 12, 120, "%s", b);
        memset(b, 0, n_);
        snprintf(b, n_, "len: %zu", m_currentLine->len);
        mvwprintw((WINDOW *)m_apiWindowPtr, 13, 120, "%s", b);
        
    }
    
    if (m_isVisible)
        m_formatter.render(m_apiWindowPtr, m_pageFirstLine, m_pageLastLine);

}




