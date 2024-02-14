#ifndef __TYPES_H
#define __TYPES_H

#include <string.h>
#include <stdlib.h>

#include "utils/log.h"

//
struct line_t
{
    line_t *next  = NULL;
    line_t *prev  = NULL;
    char *content = NULL;
    size_t len    = 0;

    ~line_t() { free(content); }
    
    void insert_char(char _c, size_t _pos);
    void insert_str(char *_str, size_t _len, size_t _pos);
    void delete_at(size_t _pos);
    line_t *split_at_pos(size_t _pos);

    #ifdef DEBUG
    void __debug_print(bool _show_ptrs, const char *_str="");
    #endif

};

// helper function for line creation (incl malloc)
line_t *create_line(char *_content, size_t _len);

// helper function for line_t realloc assertions
static void RAM_panic(line_t *_line)
{
    free(_line->content);
    _line->content = NULL;
    _line->len = 0;
    LOG_CRITICAL_ERROR("couldn't reallocate char buffer, considering buying more RAM.");
}


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
    
    //int ncols() { return (v1.x - 1) - v0.x; }
    //int nrows() { return (v1.y - 1) - v0.y; }

};


#endif // __TYPES_H