
#include "cursor.h"
#include "window.h"
#include "event_handler.h"

//
Cursor::Cursor(Window *_parent_window)
{
    m_parentWindow = _parent_window;
    m_pos = m_parentWindow->m_frame.v0;

}

//---------------------------------------------------------------------------------------
void Cursor::setPosition(int _x, int _y)
{
    m_pos.x = _x;
    m_pos.y = _y;
    clamp_to_frame_();

    api->moveCursor(_x, _y);

}

//---------------------------------------------------------------------------------------
void Cursor::update()
{
    api->moveCursor(m_pos.x, m_pos.y);

}

//---------------------------------------------------------------------------------------
void Cursor::move(int _dx, int _dy)
{
    m_pos.x += _dx;
    m_pos.y += _dy;

    // scrolling?
    if (m_pos.y < m_parentWindow->m_frame.v0.y)
        EventHandler::push_event(new BufferScrollEvent(
                                        Y_AXIS,
                                        _dy,
                                        abs(m_pos.y - m_parentWindow->m_frame.v0.y),
                                        m_parentWindow));

    else if (m_pos.y >= m_parentWindow->m_frame.v1.y)
        EventHandler::push_event(new BufferScrollEvent(
                                        Y_AXIS,
                                        _dy,
                                        abs(m_pos.y - m_parentWindow->m_frame.v1.y),
                                        m_parentWindow));

    clamp_to_frame_();
    
}

//---------------------------------------------------------------------------------------
void Cursor::clamp_to_frame_()
{
    m_pos.x = CLAMP(m_pos.x, 
                    m_parentWindow->m_frame.v0.x,
                    m_parentWindow->m_frame.v1.x);

    m_pos.y = CLAMP(m_pos.y, 
                    m_parentWindow->m_frame.v0.y,
                    m_parentWindow->m_frame.v1.y);

}


