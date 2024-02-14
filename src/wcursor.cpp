
#include "cursor.h"
#include "window.h"
#include "event_handler.h"

//
void WCursor::setPosition(int _x, int _y)
{
    m_pos.x = _x;
    m_pos.y = _y;
    clamp_to_frame_();

    api->moveCursor(m_parent->m_apiWindowPtr, _x, _y);

}

//---------------------------------------------------------------------------------------
void WCursor::setX(int _x)
{
    m_pos.x = _x;
    clamp_to_frame_();

    api->moveCursor(m_parent->m_apiWindowPtr, m_pos.x, m_pos.y);

}

//---------------------------------------------------------------------------------------
void WCursor::setY(int _y)
{
    m_pos.y = _y;
    clamp_to_frame_();

    api->moveCursor(m_parent->m_apiWindowPtr, m_pos.x, m_pos.y);
    
}

//---------------------------------------------------------------------------------------
void WCursor::update()
{
    clamp_to_frame_();
    if (api->moveCursor(m_parent->m_apiWindowPtr, m_pos.x, m_pos.y) == ERR)
        LOG_WARNING("%s : m_pos (%d, %d), window lim (%d, %d)",
                    __func__,
                    m_pos.x,
                    m_pos.y,
                    m_parent->m_frame.v1.x,
                    m_parent->m_frame.v1.y);

}

//---------------------------------------------------------------------------------------
void WCursor::move(int _dx, int _dy)
{
    m_pos.x += _dx;
    m_pos.y += _dy;

    // scrolling
    //

    // x

    // y
    if (m_pos.y < 0)
    {
        EventHandler::push_event(new BufferScrollEvent(
                                        Y_AXIS,
                                        _dy,
                                        abs(m_pos.y),
                                        m_parent));
        // keep cursor inside window
        clamp_to_frame_();
    }
    else if (m_pos.y >= m_parent->m_frame.nrows - 1)
    {
        EventHandler::push_event(new BufferScrollEvent(
                                        Y_AXIS,
                                        _dy,
                                        abs(m_pos.y - (m_parent->m_frame.nrows - 1)),
                                        m_parent));
        clamp_to_frame_();
    }

}

//---------------------------------------------------------------------------------------
void WCursor::clamp_to_frame_()
{
    m_pos.x = CLAMP(m_pos.x, 0, m_parent->m_frame.ncols - 1);
    m_pos.y = CLAMP(m_pos.y, 0, m_parent->m_frame.nrows - 1);

}


