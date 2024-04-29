#ifndef __GLFW_IMPL_H
#define __GLFW_IMPL_H

#include "platform_impl.h"


//
#ifdef GLFW_IMPL
class GLFW_Impl : public Platform_Impl
{
public:
    GLFW_Impl() {}
    ~GLFW_Impl() {}

    //
    virtual int initialize() override {}
    virtual int shutdown() override {}
    virtual void getRenderSize(ivec2_t *_v) override {}
    virtual API_WINDOW_PTR newWindow(frame_t *_frame) override {}
    virtual API_WINDOW_PTR newBorderWindow(frame_t *_frame) override {}
    virtual void deleteWindow(API_WINDOW_PTR _w) override {}

    virtual API_WINDOW_PTR newVerticalBarWindow(int _x, int _y0, int _y1) override { return NULL; }

    //
    virtual void clearScreen() override {}
    virtual void refreshScreen() override {}
    virtual void clearWindow(API_WINDOW_PTR _w) override {}
    virtual void refreshWindow(API_WINDOW_PTR _w) override {}

    // interaction
    virtual int getKey() override {}
    virtual CtrlKeyAction getCtrlKeyAction(int _key) override;
    virtual int moveCursor(API_WINDOW_PTR _w, int _x, int _y) override {}

    // output
    virtual int clearBufferLine(API_WINDOW_PTR _w, int _cy, int _win_maxx) override {};
    virtual int clearSpace(API_WINDOW_PTR _w, int _cx, int _cy, int _n) override {};
    virtual int printBufferLine(API_WINDOW_PTR _w, int _cx, int _cy, CHTYPE_PTR _line, size_t _len) override {}
    virtual int printString(API_WINDOW_PTR _w, int _cx, int _cy, CHTYPE_PTR _str, size_t _len) override {};
    virtual int wprint(API_WINDOW_PTR _w, int _cx, int _cy, const char *_fmt, ...) override {}

};
#endif


#endif // __GLFW_IMPL_H
