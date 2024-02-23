
#include "cursor.h"
#include "window/window.h"
#include "event_handler.h"

//
void Cursor::set_cpos(int _x, int _y)
{
    m_cpos.x = _x;
    m_cpos.y = _y;
    clamp_to_frame_();

    api->moveCursor(m_parent->m_apiWindowPtr, _x, _y);

}

//---------------------------------------------------------------------------------------
void Cursor::set_cx(int _x)
{
    m_cpos.x = _x;
    clamp_to_frame_();

    api->moveCursor(m_parent->m_apiWindowPtr, m_cpos.x, m_cpos.y);

}

//---------------------------------------------------------------------------------------
void Cursor::set_cy(int _y)
{
    m_cpos.y = _y;
    clamp_to_frame_();

    api->moveCursor(m_parent->m_apiWindowPtr, m_cpos.x, m_cpos.y);
    
}

//---------------------------------------------------------------------------------------
void Cursor::update()
{
    // calculate the render position
    m_rpos.y = m_cpos.y;
    line_t *line = m_parent->m_currentLine;

    int x = 0;
    for (size_t i = 0; i < m_cpos.x; i++)
    {
        if ((line->content[i] & A_CHARTEXT) == '\t')
            x = (x + (Config::TAB_SIZE - (x % Config::TAB_SIZE)));
        else
            x++;
    }
    m_rpos.x = x;

    if (m_last_rx == -1)
        m_last_rx = x;

    LOG_INFO("m_rpos.x = %d", x);
    if (api->moveCursor(m_parent->m_apiWindowPtr, m_rpos.x, m_rpos.y) == ERR)
       LOG_WARNING("%s : m_pos (%d, %d), window lim (%d, %d)",
                   __func__,
                   m_rpos.x,
                   m_rpos.y,
                   m_parent->m_frame.v1.x,
                   m_parent->m_frame.v1.y);
    
    //if (api->moveCursor(m_parent->m_apiWindowPtr, m_cpos.x, m_cpos.y) == ERR)
    //    LOG_WARNING("%s : m_pos (%d, %d), window lim (%d, %d)",
    //                __func__,
    //                m_cpos.x,
    //                m_cpos.y,
    //                m_parent->m_frame.v1.x,
    //                m_parent->m_frame.v1.y);

}

//---------------------------------------------------------------------------------------
void Cursor::move(int _dx, int _dy)
{
    m_cpos.x += _dx;
    m_cpos.y += _dy;

    m_dx = _dx;
    m_dy = _dy;

    if (m_dx != 0)
        m_last_rx = -1;

    // x scrolling

    // y scrolling
    if (m_cpos.y < 0)
    {
        EventHandler::push_event(new BufferScrollEvent(
                                        Y_AXIS,
                                        m_dy,
                                        abs(m_cpos.y),
                                        m_parent));
        // keep cursor inside window
    }
    else if (m_cpos.y >= m_parent->m_frame.nrows - 1)
    {
        EventHandler::push_event(new BufferScrollEvent(
                                        Y_AXIS,
                                        m_dy,
                                        abs(m_cpos.y - (m_parent->m_frame.nrows - 1)),
                                        m_parent));
    }

    clamp_to_frame_();

}

//---------------------------------------------------------------------------------------
void Cursor::clamp_to_frame_()
{
    m_cpos.x = CLAMP(m_cpos.x, 0, m_parent->m_frame.ncols - 1);
    m_cpos.y = CLAMP(m_cpos.y, 0, m_parent->m_frame.nrows - 1);

    m_rpos.x = CLAMP(m_rpos.x, 0, m_parent->m_frame.ncols - 1);
    m_rpos.y = CLAMP(m_rpos.y, 0, m_parent->m_frame.nrows - 1);

}


