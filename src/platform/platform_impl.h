#ifndef __PLATFORM_IMPL_H
#define __PLATFORM_IMPL_H

#include <ncurses.h>
#include "../types.h"

typedef void* API_WINDOW_PTR;


//
class Platform_Impl
{
public:
    Platform_Impl() {}
    ~Platform_Impl() {}

    //
    virtual int initialize() = 0;
    virtual int shutdown() = 0;
    virtual void getRenderSize(ivec2_t *) = 0;
    virtual API_WINDOW_PTR newWindow(irect_t *_frame) = 0;
    virtual API_WINDOW_PTR newBorderWindow(irect_t *_frame) = 0;
    virtual void deleteWindow(API_WINDOW_PTR _w) = 0;

    //
    virtual void clearScreen() = 0;
    virtual void refreshScreen() = 0;

    virtual void clearWindow(API_WINDOW_PTR _w) = 0;
    virtual void refreshWindow(API_WINDOW_PTR _w) = 0;
    virtual void refreshBorder(API_WINDOW_PTR _w) {}

    // interaction
    virtual int getKey() = 0;
    virtual CtrlKeyAction getCtrlKeyAction(int _key) = 0;
    virtual int moveCursor(API_WINDOW_PTR _w, int _x, int _y) = 0;

    // output
    virtual int printBufferLine(API_WINDOW_PTR _w, int _cx, int _cy, char* _line) = 0;
    virtual int wprint(API_WINDOW_PTR _w, int _cx, int _cy, const char *_fmt, ...) = 0;

    // accessor
    API_WINDOW_PTR screenPtr() { return m_screenPtr; }
    
protected:
    API_WINDOW_PTR m_screenPtr = NULL;
    
};

#endif // __PLATFORM_IMPL_H
