#ifndef __TYPES_H
#define __TYPES_H

#include <string.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <sys/stat.h>

#include "utils/log.h"

// scroll-related
#define X_AXIS    1
#define Y_AXIS    2

#define FORWARD   1
#define BACKWARD -1

#define NEXT      1
#define PREV     -1

// selections
#define SELECT    1
#define DESELECT  0

//
typedef void* API_WINDOW_PTR;

// the basic char type

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#define NCURSES_IMPL
#pragma clang diagnostic pop

#if defined NCURSES_IMPL
#include <ncurses.h>
// #define CHTYPE chtype
// #define CHTYPE_PTR chtype *
#define CHTYPE uint64_t
#define CHTYPE_PTR uint64_t *
#define CHTYPE_CHAR_MASK  0x00000000000000ff
#define CHTYPE_COLOR_MASK 0x000000000000ff00
#define CHTYPE_ATTR_MASK  0x00000000ffff0000
#define CHTYPE_SELECTION_BIT 32 // first bit after the first 4 bytes used by ncurses
// #elif defined (GLFW_IMPL)
#else
#define CHTYPE uint32_t
#define CHTYPE_PTR uint32_t *
#define CHTYPE_CHAR_MASK  0x000000ff
#define CHTYPE_COLOR_MASK 0x0000ff00
#define CHTYPE_ATTR_MASK  0xffff0000
#define CHTYPE_SELECTION_BIT 32
#endif
#define CHTYPE_SIZE sizeof(CHTYPE)
#define CHTYPE_PTR_SIZE sizeof(CHTYPE_PTR)

// bit manipulation used by CHTYPE
#define SET_BIT(x,nbit)     ((x) |=  ((CHTYPE)1<<(nbit)))
#define ZERO_BIT(x,nbit)    ((x) &= ~((CHTYPE)1<<(nbit)))
#define FLIP_BIT(x,nbit)    ((x) ^=  ((CHTYPE)1<<(nbit)))
#define CHECK_BIT(x,nbit)   ((x) &   ((CHTYPE)1<<(nbit)))

//
struct line_t
{
    line_t *next            = NULL;
    line_t *prev            = NULL;
    CHTYPE_PTR content      = NULL;
    size_t len              = 0;
    size_t rlen             = 0;    // rendered len, including tabs etc
    size_t sel_start        = 0;
    size_t sel_end          = 0;

    ~line_t() { free(content); content = NULL; }

    void insert_char(char _c, size_t _pos);
    void insert_char(CHTYPE _c, size_t _pos);
    void insert_str(char *_str, size_t _len, size_t _pos);
    void insert_str(const char *_str, size_t _pos);
    void insert_str(CHTYPE_PTR _str, size_t _len, size_t _pos);
    void append_line(line_t *_other);
    void delete_at(size_t _pos);
    void delete_n_at(size_t _pos, size_t _n);
    line_t *split_at_pos(size_t _pos);

    #ifdef DEBUG
    // TODO : make this permanent, and dynamic allocation of __debug_str
    char __debug_str[1024];
    void __debug_content_to_str_()
    {
        for (size_t i = 0; i < len; i++)
            __debug_str[i] = (char)(content[i] & CHTYPE_CHAR_MASK);
        __debug_str[len] = '\0';
    }
    void __debug_line();
    #endif

};

// helper function for line creation (incl malloc)
line_t *create_line(char *_content, size_t _len);
line_t *create_line(const char *_content);
line_t *create_line(const std::string &_content);
line_t *create_line(CHTYPE_PTR _content, size_t _len);
line_t *copy_line(line_t *_line);
void memcpyCharToCHTYPE(CHTYPE_PTR _dest, char *_src, size_t _len);
void memsetCHTYPE(CHTYPE_PTR _dest, CHTYPE _c, size_t _len);

// struct for copying lines
struct copy_line_t
{
    char *line_chars = NULL;
    size_t offset0 = 0;
    size_t offset1 = 0;
    size_t len = 0;
    bool newline = false;

    copy_line_t() {}
    copy_line_t(line_t *_line, bool _newline, bool _use_sel_offsets=true);
    copy_line_t(const copy_line_t &_rhs);           // copy ctor
    copy_line_t(copy_line_t &&_rhs);                // move ctor
    copy_line_t &operator=(const copy_line_t &_rhs) // copy assignment operator
    {
        offset0 = _rhs.offset0;
        offset1 = _rhs.offset1;
        len = _rhs.len;
        line_chars = (char *)malloc(len + 1);
        memcpy(line_chars, _rhs.line_chars, len);
        line_chars[len] = 0;
        newline = _rhs.newline;
        return *this;

    }
    //
    ~copy_line_t() { free(line_chars); }

};

// helper function for line_t realloc assertions
static void RAM_panic(line_t *_line)
{
    free(_line->content);
    _line->content = NULL;
    _line->len = 0;
    LOG_CRITICAL_ERROR("couldn't reallocate CHTYPE_PTR, considering buying more RAM.");
}

#ifdef DEBUG
// printing debugging function
void __debug_addchstr(API_WINDOW_PTR _w, const char *_fmt, ...);
void __debug_addchstr(API_WINDOW_PTR _w, CHTYPE_PTR _str);
void __debug_mvaddchstr(API_WINDOW_PTR _w, int _y, int _x, CHTYPE_PTR _str);
#endif

//
#ifdef DEBUG
#define DEBUG_BUFFER_SZ 1024
static char DEBUG_BUFFER[DEBUG_BUFFER_SZ];
#endif

struct ivec2_t
{
    int x = 0;
    int y = 0;
    
    //
    ivec2_t() {}
    ivec2_t(int _i) : x(_i), y(_i) {}
    ivec2_t(int _x, int _y) : x(_x), y(_y) {}

    // copy ctor
    ivec2_t(const ivec2_t &_v)              { x = _v.x; y = _v.y;                   }
    // copy assignment
    ivec2_t &operator=(const ivec2_t &_v)   { x = _v.x; y =_v.y; return *this;      }
    // move ctor
    ivec2_t(ivec2_t &&_v)                   { x = _v.x; y = _v.y;                   }

    // operators
    bool operator==(const ivec2_t &_v)          { return (x == _v.x && y == _v.y);      }
    bool operator!=(const ivec2_t &_v)          { return (x != _v.x || y != _v.y);      }
    ivec2_t operator+(const ivec2_t &_v) const  { return ivec2_t(x + _v.x, y + _v.y);   }
    void operator+=(const ivec2_t &_v)          { x += _v.x; y += _v.y;                 }
    ivec2_t operator-(const ivec2_t &_v)        { return ivec2_t(x - _v.x, y - _v.y);   }
    void operator-=(const ivec2_t &_v)          { x -= _v.x; y -= _v.y;                 }

};

// multi-line block (used for copy/cut/paste and undo buffers)
struct mline_block_t
{
    ivec2_t start_pos;
    ivec2_t end_pos;
    std::vector<copy_line_t> copy_lines;

    //
    mline_block_t() {}
    ~mline_block_t() = default;
    mline_block_t(const ivec2_t &_start_pos, const ivec2_t &_end_pos, const std::vector<copy_line_t> &_copy_lines) :
        start_pos(_start_pos), end_pos(_end_pos), copy_lines(_copy_lines)
    {}

    //
    void clear() { copy_lines.clear(); }
    const size_t size() const { return copy_lines.size(); }
    void setStart(const ivec2_t &_start) { start_pos = _start; }
    void setEnd(const ivec2_t &_end) { end_pos = _end; }

};

// single line exerpt, used by the undo buffer
struct line_chars_t
{
    ivec2_t start_pos = { 0 };
    size_t len = 0;
    char *chars = NULL;

    line_chars_t() {}
    line_chars_t(const ivec2_t _start_pos, char *_chars, size_t _len) :
        start_pos(_start_pos), len(_len)
    {
        malloc_and_copy_(_chars);
    }
    // copy ctor
    line_chars_t(const line_chars_t &_rhs) : 
        start_pos(_rhs.start_pos), len(_rhs.len)
    {
        malloc_and_copy_(_rhs.chars);
    }
    // move ctor
    line_chars_t(line_chars_t &&_rhs) :
        start_pos(_rhs.start_pos), len(_rhs.len)
    {
        malloc_and_copy_(_rhs.chars);
    }
    // copy assignment operator
    line_chars_t &operator=(const line_chars_t &_rhs)
    {
        start_pos = _rhs.start_pos;
        len = _rhs.len;
        malloc_and_copy_(_rhs.chars);
        return *this;
    }
    //
    ~line_chars_t() { free(chars); }

    //
    void malloc_and_copy_(char *_chars)
    {
        chars = (char *)malloc(len + 1);
        memcpy(chars, _chars, len);
        chars[len] = 0;
    }
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
        // ncols = (v1.x - 1) - v0.x;
        // nrows = (v1.y - 1) - v0.y;
        ncols = (v1.x) - v0.x;
        nrows = (v1.y) - v0.y;        
    }
    
    #ifdef DEBUG
    void __debug_print(const std::string &_prefix="")
    {
        
        memset(DEBUG_BUFFER, 0, DEBUG_BUFFER_SZ);
        size_t n = 0;
        if (_prefix != "")
            n = sprintf(DEBUG_BUFFER, "%s: ", _prefix.c_str());
        sprintf(DEBUG_BUFFER + n, "v0: [ %d  %d ], v1: [ %d  %d ], ncols=%d, nrows=%d", 
                v0.x, v0.y, v1.x, v1.y, ncols, nrows);
        LOG_INFO("%s", DEBUG_BUFFER);
            
    }
    #endif
    
};
typedef irect_t frame_t;

// related to ncruses strange capture of control keys (ctrl, shift, alt)
// NOTE: for some/all of these to work, some terminals with custom, non-conformant 
//       terminfo will not work, including kitty and terminator (unfortunately)..
// more keycodes at https://samuallb.github.io/ncurses/NCurses/Key.html
enum class CtrlKeyAction
{
    CTRL_LEFT, CTRL_RIGHT, CTRL_UP, CTRL_DOWN, CTRL_HOME, CTRL_END, CTRL_DEL, 
    CTRL_SHIFT_DELETE, SHIFT_UP, SHIFT_DOWN, /*SHIFT_HOME, SHIFT_END,*/ SHIFT_CTRL_LEFT, 
    SHIFT_CTRL_RIGHT, SHIFT_CTRL_UP, SHIFT_CTRL_DOWN, SHIFT_CTRL_HOME, SHIFT_CTRL_END, 
    ALT_LEFT, ALT_RIGHT, ALT_UP, ALT_DOWN, ALT_PAGEUP, ALT_PAGEDOWN, ALT_INSERT, 
    ALT_DELETE, SHIFT_ALT_LEFT, SHIFT_ALT_RIGHT, SHIFT_ALT_UP, SHIFT_ALT_DOWN, 
    NONE
};
//
#ifdef DEBUG
const char *ctrlActionStr(CtrlKeyAction _action);
#endif

//
struct keycode_t
{
    std::string id;         // e..g "kLFT5" for <ctrl> + <left arrow>

    keycode_t() {}
    keycode_t(const std::string &_id) :
        id(_id)
    {}

};
//
struct ctrl_keycode_t : keycode_t
{
    std::string ctrl_id;         // e..g "kLFT5" for <ctrl> + <left arrow>
    CtrlKeyAction action;   // corresponding enum
    
    ctrl_keycode_t() {}
    ctrl_keycode_t(const std::string &_id, CtrlKeyAction _action) :
        action(_action)
    {
        id = _id;
    }

};

// File entry, used by FileExplorerWindow
struct FileEntry
{
    std::string name = "";
    uint32_t mode = 0;      // file permissions and S_ISDIR()
    char mode_str[10] = { 0 };
    uint32_t sz = 0;

    bool is_dir() { return S_ISDIR(mode); }
    bool empty() { return sz == 0; }

    FileEntry() {}
    FileEntry(const std::string &_name, uint32_t _mode, uint32_t _sz) : 
        name(_name), mode(_mode), sz(_sz)
    {
        convert_mode_to_str_();
    }
    FileEntry(const char *_name, uint32_t _mode, uint32_t _sz) : 
        name(std::string(_name)), mode(_mode), sz(_sz)
    {
        convert_mode_to_str_();
    }

    void convert_mode_to_str_()
    {
        mode_str[0] = (S_ISDIR(mode))  ? 'd' : '-';
        mode_str[1] = (mode & S_IRUSR) ? 'r' : '-';
        mode_str[2] = (mode & S_IWUSR) ? 'w' : '-';
        mode_str[3] = (mode & S_IXUSR) ? 'x' : '-';
        mode_str[4] = (mode & S_IRGRP) ? 'r' : '-';
        mode_str[5] = (mode & S_IWGRP) ? 'w' : '-';
        mode_str[6] = (mode & S_IXGRP) ? 'x' : '-';
        mode_str[7] = (mode & S_IROTH) ? 'r' : '-';
        mode_str[8] = (mode & S_IWOTH) ? 'w' : '-';
        mode_str[9] = (mode & S_IXOTH) ? 'x' : '-';
    }

};


#endif // __TYPES_H
