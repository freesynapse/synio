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
    virtual int initialize() override {};
    virtual int shutdown() override {};
    virtual void getRenderSize(ivec2_t *_v) override {};
    virtual API_WINDOW_PTR newWindow(irect_t *_frame) override {};
    virtual API_WINDOW_PTR newBorderWindow(irect_t *_frame) override {};
    virtual void deleteWindow(API_WINDOW_PTR _w) override {}

    //
    virtual void clearScreen() override {}
    virtual void refreshScreen() override {}
    virtual void clearWindow(API_WINDOW_PTR _w) override {};
    virtual void refreshWindow(API_WINDOW_PTR _w) override {};

    // interaction
    virtual int getKey() override {};
    virtual int moveCursor(API_WINDOW_PTR _w, int _x, int _y) override {};

    // output
    virtual int printBufferLine(API_WINDOW_PTR _w, int _cx, int _cy, char* _line) override {};

};



#endif // __GLFW_IMPL_H
