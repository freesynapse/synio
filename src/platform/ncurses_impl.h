#ifndef __NCURSES_IMPL_H
#define __NCURSES_IMPL_H

#include <ncurses.h>

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

    //
    virtual void clearScreen() override     { clear();   }
    virtual void refreshScreen() override   { refresh(); }

    //
    virtual int getKey() override;
    virtual void moveCursor(int _x, int _y) override;
    virtual void printBufferLine(int _cx, int _cy, char* _line) override;

private:
    WINDOW *m_mainWindow = NULL;
    

};

#endif // __NCURSES_IMPL_H
