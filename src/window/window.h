#ifndef __WINDOW_H
#define __WINDOW_H

#include <string>
#include <unordered_set>

#include "../core.h"
#include "../types.h"
#include "../cursor.h"
#include "../utils/log.h"
#include "../utils/file_io.h"
#include "../buffer/buffer_formatter.h"
#include "../buffer/line_buffer.h"
#include "../platform/platform.h"
#include "../events.h"
#include "../config.h"

// window parameters
#define WPARAMS_NONE        0x00000000
#define WPARAMS_BORDER      0x00000001

//
class Window
{
public:
    friend class Cursor;
    friend class Synio;

public:
    Window(const frame_t &_frame, const std::string &_id, int _wnd_params);
    virtual ~Window();
    
    // creates a border around the drawable area
    virtual void enableBorder();

    // input for windows where this is relevant
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) = 0;

    // Platform interactions
    virtual void resize(frame_t _new_frame);
    virtual void redraw() = 0;
    virtual void clear()
    { 
        // clear() clears the borders...
        // ---> https://stackoverflow.com/questions/33986047/ncurses-is-it-possible-to-refresh-a-window-without-removing-its-borders
        // -- Changed in the Window class so that all extra 'border' windows can be drawn.        
        if (!m_clearNextFrame)
            return;

        m_clearNextFrame = false;
        api->clearWindow(m_apiWindowPtr);
        
        if (m_apiBorderWindowPtr)
            api->clearWindow(m_apiBorderWindowPtr);
    }
    virtual void refresh()
    {
        if (!m_refreshNextFrame)
            return;

        m_refreshNextFrame = false;

        // This causes screen flickering on repeated input!
        //if (m_apiBorderWindowPtr)
        //    api->refreshBorder(m_apiBorderWindowPtr);
        
        api->refreshWindow(m_apiWindowPtr);
    }

    // accessors
    const frame_t &frame() { return m_frame; }
    const std::string &ID() const { return m_ID; }
    void setVisibility(bool _b) { m_isWindowVisible = _b; }
    void setID(const std::string &_id) { m_ID = _id; }

    //
    #ifdef DEBUG
    void __debug_print(int _x, int _y, const char *_fmt, ...);
    #endif

protected:
    __always_inline void clear_next_frame_() { m_clearNextFrame = true; }
    __always_inline void refresh_next_frame_() { m_refreshNextFrame = true; }

protected:
    frame_t m_frame = frame_t(0);
    std::string m_ID = "";
    int m_wndParams = WPARAMS_NONE;

    API_WINDOW_PTR m_apiWindowPtr = NULL;
    API_WINDOW_PTR m_apiBorderWindowPtr = NULL;

    bool m_isWindowVisible = true;
    bool m_clearNextFrame = true;
    bool m_refreshNextFrame = true;

    // set of lines (in cursor corrdinates) in need of update (and thus clearing and 
    // re-rendering) to avoid using clear
    #ifdef NCURSES_IMPL
    std::unordered_set<int> m_windowLinesUpdateList;
    #endif

};


#endif // __WINDOW_H
