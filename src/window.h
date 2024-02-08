#ifndef __WINDOW_H
#define __WINDOW_H

#include <string>

#include "types.h"
#include "cursor.h"
#include "utils/log.h"

//
class Window
{
public:
    friend class Cursor;

public:
    Window(const irect_t &_frame, const std::string &_id) : 
        m_frame(_frame), m_ID(_id)
    {
        m_cursor = new Cursor(this);
        LOG_INFO("%s: %s [%p] frame = (%d, %d) -- (%d, %d)\n",
                 __func__, m_ID.c_str(), this,
                 m_frame.v0.x, m_frame.v0.y,
                 m_frame.v1.x, m_frame.v1.y);
    }
    ~Window()
    {
        delete m_cursor;

    }

    // accessors
    const irect_t &frame() { return m_frame; }
    Cursor *cursor() { return m_cursor; }
    const std::string &ID() const { return m_ID; }

private:
    std::string m_ID;
    irect_t m_frame;
    Cursor *m_cursor;
};






#endif // __WINDOW_H
