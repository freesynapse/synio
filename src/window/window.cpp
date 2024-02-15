
#include "window.h"

//
Window::Window(irect_t *_frame, const std::string &_id, bool _border)
{
    m_frame = *_frame;
    m_ID = _id;

    m_apiWindowPtr = api->newWindow(&m_frame);
    if (_border)
        m_apiBorderWindowPtr = api->newBorderWindow(&m_frame);

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
    int y = m_cursor.y();
    m_cursor.setPosition(0, y);

}

//---------------------------------------------------------------------------------------
void Window::moveCursorToLineEnd()
{
    int y = m_cursor.y();
    m_cursor.setPosition(m_frame.ncols, y);

}

//---------------------------------------------------------------------------------------
void Window::updateCursor()
{
    m_cursor.update();

}

//---------------------------------------------------------------------------------------
#ifdef DEBUG
void Window::__debug_print(int _x, int _y, const char *_fmt, ...)
{
    memset(__debug_buffer, 0, __DEBUG_BUFFER_LEN);
    va_list arg_list;
    va_start(arg_list, _fmt);
    vsnprintf(__debug_buffer, __DEBUG_BUFFER_LEN, _fmt, arg_list);
    va_end(arg_list);
    mvwprintw((WINDOW*)m_apiWindowPtr, _y, _x, "%s", __debug_buffer);
}
#endif

