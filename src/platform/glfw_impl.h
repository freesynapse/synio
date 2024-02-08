#ifndef __GLFW_IMPL_H
#define __GLFW_IMPL_H

#include "platform_impl.h"


//
class GLFW_Impl : public Platform_Impl
{
public:
    GLFW_Impl() {}
    ~GLFW_Impl() {}

    //
    virtual int initialize() {};
    virtual int shutdown() {};
    virtual void getRenderSize(ivec2_t *_v) {};

    //
    virtual void clearScreen() {};
    virtual void refreshScreen() {};

    // interaction
    virtual int getKey() {};
    virtual void moveCursor(int _x, int _y) {};

    // output
    virtual void printBufferLine(int _cx, int _cy, char* _line) {};

};



#endif // __GLFW_IMPL_H
