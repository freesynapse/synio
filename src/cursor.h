#ifndef __CURSOR_H
#define __CURSOR_H

#include "core.h"
#include "utils/utils.h"

// forward decl
class Window;

//
class Cursor
{
public:
    Cursor() {}
    Cursor(Window *_parent_window);
    ~Cursor() = default;

    //
    void update();
    void setPosition(int _x, int _y);

    //
    void move(int _dx, int _dy);

    // accessors
    const ivec2_t &pos() { return m_pos; }

private:
    void clamp_to_frame_();

private:
    ivec2_t m_pos = { 0 };
    Window *m_parentWindow = NULL;

};








#endif // __CURSOR_H
