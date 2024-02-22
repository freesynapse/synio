#ifndef __TYPES_H
#define __TYPES_H

#include <string.h>
#include <stdlib.h>
#include <string>

#include "utils/log.h"
//
typedef void* API_WINDOW_PTR;

#define CHTYPE_CHAR_MASK 0x000000ff

// the basic char type
#if defined NCURSES_IMPL
#include <ncurses.h>
#define CHTYPE chtype
#define CHTYPE_PTR chtype *
#define CHTYPE_COLOR_MASK 0x0000ff00
#define CHTYPE_ATTR_MASK  0xffff0000
// #elif defined (GLFW_IMPL)
#else
#define CHTYPE char
#define CHTYPE_PTR char *
#define CHTYPE_COLOR_MASK 0xffffffff
#define CHTYPE_ATTR_MASK  0xffffffff
#endif
#define CHTYPE_SIZE sizeof(CHTYPE)
#define CHTYPE_PTR_SIZE sizeof(CHTYPE_PTR)

// something to write in for conversions
#define buf0_sz 1024
static char buf0[buf0_sz];

//
struct line_t
{
    line_t *next        = NULL;
    line_t *prev        = NULL;
    CHTYPE_PTR content  = NULL;
    size_t len          = 0;

    ~line_t() { free(content); }

    void insert_char(char _c, size_t _pos);
    void insert_str(char *_str, size_t _len, size_t _pos);
    void delete_at(size_t _pos);
    line_t *split_at_pos(size_t _pos);

    // conversion to char * in case of debug printing
    char *content_to_str()
    {
        memset(buf0, 0, buf0_sz);
        for (size_t i = 0; i < len; i++)
            buf0[i] = (content[i] & 0x000000ff);
        buf0[len] = '\0';
        return buf0;
    }

};

// helper function for line creation (incl malloc)
line_t *create_line(char *_content, size_t _len);
line_t *create_line(const char *_content);
line_t *create_line(CHTYPE_PTR _content, size_t _len);

// helper function for line_t realloc assertions
static void RAM_panic(line_t *_line)
{
    free(_line->content);
    _line->content = NULL;
    _line->len = 0;
    LOG_CRITICAL_ERROR("couldn't reallocate char buffer, considering buying more RAM.");
}
#ifdef DEBUG
// printing debugging function
void __debug_addchstr(API_WINDOW_PTR _w, const char *_fmt, ...);
void __debug_addchstr(API_WINDOW_PTR _w, CHTYPE_PTR _str);
void __debug_mvaddchstr(API_WINDOW_PTR _w, int _y, int _x, CHTYPE_PTR _str);
#endif

//
struct ivec2_t
{
    int x = 0;
    int y = 0;
    
    //
    ivec2_t() {}
    ivec2_t(int _i) : x(_i), y(_i) {}
    ivec2_t(int _x, int _y) : x(_x), y(_y) {}

    // operators
    ivec2_t &operator=(const ivec2_t &_v)   { x = _v.x; y =_v.y; return *this;      }
    bool operator==(const ivec2_t &_v)      { return (x == _v.x && y == _v.y);      }
    ivec2_t operator+(const ivec2_t &_v)    { return ivec2_t(x + _v.x, y + _v.y);   }
    void operator+=(const ivec2_t &_v)      { x += _v.x; y += _v.y;                 }
    ivec2_t operator-(const ivec2_t &_v)    { return ivec2_t(x - _v.x, y - _v.y);   }
    void operator-=(const ivec2_t &_v)      { x -= _v.x; y -= _v.y;                 }

};

//
struct irect_t
{
    ivec2_t v0; // top-left
    ivec2_t v1; // bottom-right

    int ncols = 0;
    int nrows = 0;

    //
    irect_t() {}
    irect_t(int _i) : v0(ivec2_t(_i)), v1(ivec2_t(_i)) { update_dims(); }
    irect_t(const ivec2_t &_v0, const ivec2_t &_v1) : v0(_v0), v1(_v1) { update_dims(); }

    void update_dims()
    {
        ncols = (v1.x - 1) - v0.x;
        nrows = (v1.y - 1) - v0.y;
    }
    
};

// related to ncruses strange capture of control keys (ctrl, shift, alt)
enum class CtrlKeyAction
{
    CTRL_LEFT, CTRL_RIGHT, CTRL_UP, CTRL_DOWN, CTRL_HOME, CTRL_END, CTRL_DELETE, 
    CTRL_SHIFT_DELETE, SHIFT_UP, SHIFT_DOWN, SHIFT_CTRL_LEFT, SHIFT_CTRL_RIGHT, 
    SHIFT_CTRL_UP, SHIFT_CTRL_DOWN, SHIFT_CTRL_HOME, SHIFT_CTRL_END, ALT_LEFT, 
    ALT_RIGHT, ALT_UP, ALT_DOWN, ALT_PAGEUP, ALT_PAGEDOWN, ALT_INSERT, ALT_DELETE, 
    SHIFT_ALT_LEFT, SHIFT_ALT_RIGHT, SHIFT_ALT_UP, SHIFT_ALT_DOWN, 
    NONE
};
//
#ifdef DEBUG
const char *ctrlActionStr(CtrlKeyAction _action);
#endif
//
struct ctrl_keycode_t
{
    std::string id;             // e..g "kLFT5" for <ctrl> + <left arrow>
    CtrlKeyAction action;   // corresponding enum
    
    ctrl_keycode_t() {}
    ctrl_keycode_t(const std::string &_id, CtrlKeyAction _action) :
        id(_id), action(_action)
    {}

};


#endif // __TYPES_H
