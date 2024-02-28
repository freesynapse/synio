#ifndef __WINDOW_H
#define __WINDOW_H

#include <string>
#include <set>

#include "../core.h"
#include "../types.h"
#include "../cursor.h"
#include "../utils/log.h"
#include "../utils/file_io.h"
#include "../buffer_formatter.h"
#include "../buffer/line_buffer.h"
#include "../platform/platform.h"
#include "../events.h"
#include "../config.h"

//
class Window
{
public:
    friend class Cursor;
    friend class Synio;

public:
    //Window() {}
    Window(const frame_t &_frame, const std::string &_id, bool _border=true);
    // Window(const ivec2_t &_v0, const ivec2_t &_v1, const std::string &_id, bool _border);
    virtual ~Window();
    
    // creates a border around the drawable area
    virtual void enableBorder();

    // Platform interactions
    virtual void resize(frame_t _new_frame);
    virtual void redraw() = 0;
    virtual void clear()
    { 
        if (!m_clearNextFrame)
            return;

        api->clearWindow(m_apiWindowPtr);
        if (m_apiBorderWindowPtr)
            api->clearWindow(m_apiBorderWindowPtr);
    }
    virtual void refresh()
    {
        if (!m_refreshNextFrame)
            return;

        if (m_apiBorderWindowPtr)
            api->refreshBorder(m_apiBorderWindowPtr);
        api->refreshWindow(m_apiWindowPtr);
    }

    // accessors
    const frame_t &frame() { return m_frame; }
    const std::string &ID() const { return m_ID; }
    void setVisibility(bool _b) { m_isWindowVisible = _b; }

    //
    #ifdef DEBUG
    void __debug_print(int _x, int _y, const char *_fmt, ...);
    #endif

protected:
    __always_inline void clear_next_frame_() { m_clearNextFrame = true; }
    __always_inline void refresh_next_frame_() { m_refreshNextFrame = true; }

protected:
    std::string m_ID = "";
    frame_t m_frame = frame_t(0);

    API_WINDOW_PTR m_apiWindowPtr = NULL;
    API_WINDOW_PTR m_apiBorderWindowPtr = NULL;

    bool m_isWindowVisible = true;
    bool m_clearNextFrame = true;
    bool m_refreshNextFrame = true;

    // vector of lines (in cursor corrdinates) in need of update (and thus clearing and 
    // re-rendering) to avoid using clear
    #ifdef NCURSES_IMPL
    std::set<int> m_linesUpdateList;
    #endif

};


#endif // __WINDOW_H
