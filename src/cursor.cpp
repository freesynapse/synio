
#include "cursor.h"
#include "window.h"
#include "event_handler.h"

//
void Cursor::setPosition(Window *_window, int _x, int _y)
{
    m_pos.x = _x;
    m_pos.y = _y;
    clamp_to_frame_(_window);

    api->moveCursor(_window->m_apiWindowPtr, _x, _y);

}

//---------------------------------------------------------------------------------------
void Cursor::update(Window *_window)
{
    clamp_to_frame_(_window);
    if (api->moveCursor(_window->m_apiWindowPtr, m_pos.x, m_pos.y) == ERR)
        LOG_WARNING("%s : m_pos (%d, %d), window lim (%d, %d)", __func__, m_pos.x, m_pos.y, _window->frame().v1.x, _window->frame().v1.y);

}

//---------------------------------------------------------------------------------------
void Cursor::move(Window *_window, int _dx, int _dy)
{
    m_pos.x += _dx;
    m_pos.y += _dy;

    // scrolling?
    if (m_pos.y < _window->m_frame.v0.y)
    {
        EventHandler::push_event(new BufferScrollEvent(
                                        Y_AXIS,
                                        _dy,
                                        abs(m_pos.y - _window->m_frame.v0.y),
                                        _window));
        // keep cursor inside window
        clamp_to_frame_(_window);
    }
    else if (m_pos.y >= _window->m_frame.v1.y - 1)
    {
        EventHandler::push_event(new BufferScrollEvent(
                                        Y_AXIS,
                                        _dy,
                                        abs(m_pos.y - (_window->m_frame.v1.y - 1)),
                                        _window));
        clamp_to_frame_(_window);
    }

}

//---------------------------------------------------------------------------------------
void Cursor::clamp_to_frame_(Window *_window)
{
    m_pos.x = CLAMP(m_pos.x, 
                    _window->m_frame.v0.x,
                    _window->m_frame.v1.x);

    m_pos.y = CLAMP(m_pos.y, 
                    _window->m_frame.v0.y,
                    _window->m_frame.v1.y - 1);

}


