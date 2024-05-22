
#include "line_numbers_window.h"
#include "file_buffer_window.h"


//
void LineNumbers::resize(frame_t _buffer_frame)
{
    // use parent buffer and width to deduce frame
    frame_t new_frame = frame_t(ivec2_t(0, 0), 
                                ivec2_t(m_width, _buffer_frame.v1.y));

    Window::resize(new_frame);

}

//---------------------------------------------------------------------------------------
void LineNumbers::redraw()
{
    if (!m_isWindowVisible)
        return;

    // int width = m_frame.v1.x - 2;
    api->clearWindow(m_apiWindowPtr);

    int y = 0;
    int line_no = m_associatedBuffer->m_scrollPos.y + 1;
    line_t *curr_line = m_associatedBuffer->m_pageFirstLine;

    api->enableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_LINENO));
    while (y < m_frame.nrows && curr_line != NULL)
    {
        api->wprint(m_apiWindowPtr, 0, y++, "%*d", m_width-3, line_no++);
        curr_line = curr_line->next;
    }
    api->disableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_LINENO));

    // draw blank line numbers
    if (y < m_frame.nrows)
    {
        api->enableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_LINENO_INACTIVE));
        while (y < m_frame.nrows)
            api->wprint(m_apiWindowPtr, 0, y++, "%*d", m_width-3, line_no++);
        api->disableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_LINENO_INACTIVE));
    }

    api->vertical_divider(m_apiWindowPtr, m_frame.ncols - 1, 0, m_frame.nrows);
}

//---------------------------------------------------------------------------------------
void LineNumbers::refresh()
{
    // TODO : always draw linenumbers? For now, yes.
    // if (!m_refreshNextFrame)
    //     return;
    // m_refreshNextFrame = false;
    
    // for LineNumbers, always refresh (!?)
    if (m_apiBorderWindowPtr)
        api->refreshBorder(m_apiBorderWindowPtr);
    
    api->refreshWindow(m_apiWindowPtr);
}
