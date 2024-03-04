#ifndef __CURSOR_H
#define __CURSOR_H

#include "core.h"
#include "utils/utils.h"

// forward decl
class BufferWindowBase;

//
class Cursor
{
public:
    Cursor() {}
    Cursor(BufferWindowBase *_parent);
    ~Cursor() = default;

    //
    void update();
    
    //void set_cpos(const ivec2_t &_p) { set_cpos(_p.x, _p.y); };
    void set_cpos(int _x, int _y);
    void set_cx(int _x);
    void set_cy(int _y);
    //void set_rpos(const ivec2_t &_p) { set_rpos(_p.x, _p.y); };
    //void set_rpos(int _x, int _y);
    //void set_rx(int _x);
    //void set_ry(int _y);
    
    //
    void move(int _dx, int _dy);

    // accessors
    const ivec2_t &cpos() { return m_cpos; }
    const int &cx() { return m_cpos.x; }
    const int &cy() { return m_cpos.y; }
    const ivec2_t &rpos() { return m_rpos; }
    const int &rx() { return m_rpos.x; }
    const int &ry() { return m_rpos.y; }
    const int &last_rx() { return m_last_rx; }
    const int &dx() { return m_dx; }
    const int &dy() { return m_dy; }
    int offset_x() { return m_offset_pos.x; }
    int offset_y() { return m_offset_pos.y; }

    void set_scrolled_x() { m_scrolled_x = true; }
    void set_scrolled_y() { m_scrolled_y = true; }
    bool was_scrolled_x() { return m_scrolled_x; }
    bool was_scrolled_y() { return m_scrolled_y; }
    void set_frame(const irect_t &_frame) { m_frame = _frame; }
    void set_offset(const ivec2_t &_v) { m_offset_pos = _v; }
    void set_offset_x(int _x) { m_offset_pos.x = _x; }
    void set_offset_y(int _y) { m_offset_pos.y = _y; }

private:
    void clamp_to_frame_();
    int calc_rposx_from_cposx_();
    int calc_cposx_from_rposx_();

private:
    int m_dx = 0;
    int m_dy = 0;
    ivec2_t m_cpos = ivec2_t(0);
    ivec2_t m_rpos = ivec2_t(0);
    ivec2_t m_offset_pos = ivec2_t(0);    // added to render position
    
    int m_last_rx = 0;

    bool m_scrolled_x = false;
    bool m_scrolled_y = false;

    BufferWindowBase *m_parent = NULL;
    frame_t m_frame;

};

#endif // __CURSOR_H
