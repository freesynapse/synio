
#include "window.h"


//
Window::Window(const frame_t &_frame, const std::string &_id, bool _border)
{
   m_frame = _frame;
   m_ID = _id;

   m_apiWindowPtr = api->newWindow(&m_frame);
   if (_border)
       m_apiBorderWindowPtr = api->newBorderWindow(&m_frame);

   LOG_INFO("Window '%s' [%p] created.", m_ID.c_str(), this);


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
void Window::resize(frame_t _new_frame)
{
    m_frame = _new_frame;

    api->clearWindow(m_apiWindowPtr);
    api->deleteWindow(m_apiWindowPtr);
    m_apiWindowPtr = api->newWindow(&m_frame);
    // resfresh called by api->newWindow()

    if (m_apiBorderWindowPtr)
    {
        api->clearWindow(m_apiBorderWindowPtr);
        api->deleteWindow(m_apiBorderWindowPtr);
        m_apiBorderWindowPtr = api->newBorderWindow(&m_frame);
    }
    
}

//---------------------------------------------------------------------------------------
#if (defined DEBUG) & (defined NCURSES_IMPL)
void Window::__debug_print(int _x, int _y, const char *_fmt, ...)
{
    memset(DEBUG_BUFFER, 0, DEBUG_BUFFER_SZ);
    va_list arg_list;
    va_start(arg_list, _fmt);
    size_t n = vsnprintf(DEBUG_BUFFER, DEBUG_BUFFER_SZ, _fmt, arg_list);
    va_end(arg_list);

    int i = 0;
    while (i < m_frame.v1.x - _x)
    {
        mvwaddch((WINDOW *)m_apiWindowPtr, _y, _x+i, ' ');
        i++;
    }
    mvwprintw((WINDOW*)m_apiWindowPtr, _y, _x, "%s", DEBUG_BUFFER);
    
}
#endif
