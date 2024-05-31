#ifndef __EVENTS_H
#define __EVENTS_H

#include <string>
#include "types.h"

//
#define EVENT_TYPE(_type) static EventType staticType() { return EventType::_type; } \
    virtual EventType eventType() const override { return staticType(); } \
    virtual const char* name() const override { return #_type; }

//
enum class EventType
{
    BUFFER_SCROLL, BUFFER_SEARCH_QUERY, SAVE, SAVE_AS, ADJUST_BUFFER_WINDOW, 
    NEW_COMMAND_WINDOW, DELETE_COMMAND_WINDOW,
    EXIT

};

//
class Event
{
public:
    virtual ~Event() {};
    inline virtual EventType eventType() const = 0;
    inline virtual const char* name() const = 0;
    inline virtual std::string toString() const { return name(); }
    bool m_handled = false;
    inline virtual const bool& isHandled() const { return m_handled; }
};

//
class Window;
//class BufferScrollEvent : public Event
//{
//public:
//    BufferScrollEvent(int _axis, int _dir, int _steps, Window *_window_ptr) :
//        m_axis(_axis), m_dir(_dir), m_steps(_steps), m_windowPtr(_window_ptr)
//    {}
//
//    __always_inline const int axis() const { return m_axis; }
//    __always_inline const int dir() const { return m_dir; }
//    __always_inline const int steps() const { return m_steps; }
//    __always_inline const Window *windowPtr() const { return m_windowPtr; }
//
//    EVENT_TYPE(BUFFER_SCROLL)
//
//private:
//    int m_axis = 0;
//    int m_dir = 0;
//    int m_steps = 0;
//    Window *m_windowPtr = NULL;
//    
//};

//
class AdjustBufferWindowEvent : public Event
{
public:
    AdjustBufferWindowEvent(int _dy) : dy(_dy) {}
    EVENT_TYPE(ADJUST_BUFFER_WINDOW);
    int dy;
};

//
class NewCommandWindowEvent : public Event
{
public:
    NewCommandWindowEvent() {}
    EVENT_TYPE(NEW_COMMAND_WINDOW)
};

//
class DeleteCommandWindowEvent : public Event
{
public:
    DeleteCommandWindowEvent() {}
    EVENT_TYPE(DELETE_COMMAND_WINDOW)
};

//
class ExitEvent : public Event
{
public:
    ExitEvent() {}
    EVENT_TYPE(EXIT)
};


#endif // __EVENTS_H