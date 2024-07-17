#ifndef __EVENTS_H
#define __EVENTS_H

#include <string>
#include <filesystem>

#include "types.h"

//
#define EVENT_TYPE(_type) static EventType staticType() { return EventType::_type; } \
    virtual EventType eventType() const override { return staticType(); } \
    virtual const char* name() const override { return #_type; }

//
enum class EventType
{
    BUFFER_SCROLL, BUFFER_SEARCH_QUERY, SAVE, SAVE_AS, ADJUST_BUFFER_WINDOW, 
    NEW_COMMAND_WINDOW, CLOSE_COMMAND_WINDOW, CLOSE_BUFFER_WINDOW,
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
class CloseCommandWindowEvent : public Event
{
public:
    CloseCommandWindowEvent() {}
    EVENT_TYPE(CLOSE_COMMAND_WINDOW)
};

//
class CloseFileBufferEvent : public Event
{
public:
    CloseFileBufferEvent(const std::filesystem::path _filepath) :
        filepath(_filepath)
    {}
    CloseFileBufferEvent(bool _close_this) :
        close_this(_close_this)
    {}
    EVENT_TYPE(CLOSE_BUFFER_WINDOW)

    std::filesystem::path filepath = "";
    int close_this = false;
};

//
class ExitEvent : public Event
{
public:
    ExitEvent() {}
    EVENT_TYPE(EXIT)
};


#endif // __EVENTS_H