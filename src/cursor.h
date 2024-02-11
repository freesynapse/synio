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
    ~Cursor() = default;

    //
    void update(Window *_window);
    void setPosition(Window *_window, int _x, int _y);

    //
    void move(Window *_window, int _dx, int _dy);

    // accessors
    const ivec2_t &pos() { return m_pos; }

private:
    void clamp_to_frame_(Window *_window);

private:
    ivec2_t m_pos;  // position in current window

};


class WCursor
{
public:
    WCursor() {}
    WCursor(Window *_parent) : m_parent(_parent) {}
    ~WCursor() = default;

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
    ivec2_t m_pos;  // position in current window
    Window *m_parent;

};

#endif // __CURSOR_H
