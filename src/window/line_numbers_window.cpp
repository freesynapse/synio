
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

    int y = 0;
    int line_no = m_associatedBuffer->m_scrollPos.y + 1;
    line_t *curr_line = m_associatedBuffer->m_pageFirstLine;

    while (y < m_frame.nrows && curr_line != NULL)
    {
        api->wprint(m_apiWindowPtr, 0, y++, "%*d", m_width-3, line_no++);
        curr_line = curr_line->next;
    }

    // TODO : move this into platform_impl and sub-classes thereof
    mvwvline((WINDOW *)m_apiWindowPtr, 0, m_frame.ncols - 1, ACS_VLINE, m_frame.nrows);

}

