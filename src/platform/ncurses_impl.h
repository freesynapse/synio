#ifndef __NCURSES_IMPL_H
#define __NCURSES_IMPL_H

#include <ncurses.h>

#include "../core.h"
#include "platform_impl.h"

//
class Ncurses_Impl : public Platform_Impl
{
public:
    Ncurses_Impl()  { initialize(); }
    ~Ncurses_Impl() { shutdown();   }
    //
    virtual int initialize() override;
    virtual int shutdown() override;
    virtual void getRenderSize(ivec2_t *_v) override;
    virtual API_WINDOW_PTR newWindow(irect_t *_frame) override;

    //
    virtual void clearScreen() override     { wclear((WINDOW *)m_screenPtr);   }
    virtual void refreshScreen() override   { wrefresh((WINDOW *)m_screenPtr); }
    virtual void clearWindow(API_WINDOW_PTR _w) override     { wclear((WINDOW *)_w);   }
    virtual void refreshWindow(API_WINDOW_PTR _w) override   { wrefresh((WINDOW *)_w); }

    //
    virtual int getKey() override;
    virtual void moveCursor(API_WINDOW_PTR _w, int _x, int _y) override;
    virtual void printBufferLine(API_WINDOW_PTR _w, int _cx, int _cy, char* _line) override;

};

#endif // __NCURSES_IMPL_H
