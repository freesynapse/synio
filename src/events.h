#ifndef __EVENTS_H
#define __EVENTS_H

#include <string>

//
#define EVENT_TYPE(_type) static EventType staticType() { return EventType::_type; } \
    virtual EventType eventType() const override { return staticType(); } \
    virtual const char* name() const override { return #_type; }

//
enum class EventType
{
    BUFFER_SCROLL,

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
#define X_AXIS 1
#define Y_AXIS 2
class BufferScrollEvent : public Event
{
public:
    BufferScrollEvent(int _axis, int _dir, int _steps, Window *_window_ptr) :
        m_axis(_axis), m_dir(_dir), m_steps(_steps), m_windowPtr(_window_ptr)
    {}

    inline const int axis() const { return m_axis; }
    inline const int dir() const { return m_dir; }
    inline const int steps() const { return m_steps; }
    inline const Window *windowPtr() const { return m_windowPtr; }

    EVENT_TYPE(BUFFER_SCROLL)

private:
    int m_axis = 0;
    int m_dir = 0;
    int m_steps = 0;
    Window *m_windowPtr = NULL;
    
};



#endif // __EVENTS_H