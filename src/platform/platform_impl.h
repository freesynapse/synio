#ifndef __PLATFORM_IMPL_H
#define __PLATFORM_IMPL_H

#include "../types.h"

//
class Platform_Impl
{
public:
    Platform_Impl() {}
    ~Platform_Impl() {}

    //
    virtual int initialize() = 0;
    virtual int shutdown() = 0;
    virtual void getRenderSize(ivec2_t *_v) = 0;

    //
    virtual void clearScreen() = 0;
    virtual void refreshScreen() = 0;

    // interaction
    virtual int getKey() = 0;
    virtual void moveCursor(int _x, int _y) = 0;

    // output
    virtual void printBufferLine(int _cx, int _cy, char* _line) = 0;

};

#endif // __PLATFORM_IMPL_H
