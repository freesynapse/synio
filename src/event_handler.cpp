
#include "event_handler.h"

#include "utils/utils.h"
#include "utils/log.h"


// declaration of static member variables
//
unsigned char EventHandler::m_queueHead;
unsigned char EventHandler::m_queueTail;
Event *EventHandler::m_eventQueue[EventHandler::MAX_EVENTS];
unsigned short EventHandler::m_numCallbacks;
std::multimap<EventType, std::function<void(Event *)>> g_mapHandlerFnc;

//---------------------------------------------------------------------------------------
void EventHandler::init()
{
    LOG_INFO("listening.");

    m_queueHead = 0;
    m_queueTail = 0;
    m_numCallbacks = 0;
}

//---------------------------------------------------------------------------------------
void EventHandler::shutdown()
{
    LOG_INFO("clearing event queue.");

    int numCleared = 0;
    for (size_t i = 0; i < EventHandler::MAX_EVENTS; i++)
    {
        if (m_eventQueue[i])
        {
            delete m_eventQueue[i];
            numCleared++;

        }
    }

    // LOG_INFO("%d event(s) deleted.", numCleared);

}

//---------------------------------------------------------------------------------------
int EventHandler::push_event(Event *_event)
{
    // release stored event (if any)
    if (m_eventQueue[m_queueTail])
    {
        delete m_eventQueue[m_queueTail];
        m_eventQueue[m_queueTail] = nullptr;

    }

    // add event to the end of the list
    m_eventQueue[m_queueTail] = _event;
    m_queueTail = (m_queueTail + 1) % MAX_EVENTS;

    return RETURN_SUCCESS;
}

//---------------------------------------------------------------------------------------
int EventHandler::queue_length()
{
    return (m_queueTail % MAX_EVENTS) - (m_queueHead % MAX_EVENTS);
}

//---------------------------------------------------------------------------------------
/* TODO: Eventually, different systems may register with the EventHandler to process events
    * of the EventType (Event.h) corresponding to task of the registered class. Each class that
    * registers for an event type have to provide a function pointer to the function used to
    * process events of a certain EventType.
    */
//unsigned int ProcessEvent() {}
Event *EventHandler::next_event()
{
    if (m_queueHead == m_queueTail)
        return nullptr;

    Event *ptr;
    if (m_eventQueue[m_queueHead])
    {
        ptr = m_eventQueue[m_queueHead];
        m_queueHead = (m_queueHead + 1) % MAX_EVENTS;

        if (ptr->isHandled())
            return nullptr;
    }
    else
        ptr = nullptr;

    return ptr;

}

//---------------------------------------------------------------------------------------
/*
    Process events pushed by subsystems of SynapseCore.
*/
void EventHandler::process_events()
{
    Event *e;
    while ((e = EventHandler::next_event()))
    {
        // find all values corresponding to key EventType of the current Event.
        std::pair<mapIterator, mapIterator> res = g_mapHandlerFnc.equal_range(e->eventType());

        //std::cout << "EventType: " << (int)e->eventType() << ": funcs:" << std::endl;
        // step through result and dispatch to handler functions
        for (mapIterator it = res.first; it != res.second; it++)
        {
            // call the function with the current event
            it->second(e);
        }

        // mark as handled
        e->m_handled = true;

    }

}

//---------------------------------------------------------------------------------------
void EventHandler::register_callback(EventType _event_type, std::function<void(Event *)> _handler_fnc)
{
    #ifdef DEBUG_EVENTS
        SYN_CORE_TRACE("new callback registered: ", strEventType(_event_type), ", 0x", *(long*)(char*)& _handler_fnc, ".");
    #endif

    if (m_numCallbacks < MAX_CALLBACKS)
    {
        // add the new function to the multimap under the correct key
        g_mapHandlerFnc.insert(std::pair<EventType, std::function<void(Event *)> >(_event_type, _handler_fnc));

        m_numCallbacks++;
    }

}
