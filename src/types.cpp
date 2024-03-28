
#include "types.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef NCURSES_IMPL
#include <ncurses.h>
#endif

#include "platform/platform.h"
#include "buffer/replay_buffer.h"


//
line_t *create_line(char *_content, size_t _len)
{
    line_t *new_line = (line_t *)malloc(sizeof(line_t));
    
    new_line->next = NULL;
    new_line->prev = NULL;
    size_t bytes = CHTYPE_SIZE * (_len+1);
    new_line->content = (CHTYPE_PTR)malloc(bytes);
    new_line->len = _len;
    memset(new_line->content, 0, bytes);
    //memcpy(new_line->content, _content, _len);
    for (size_t i = 0; i < _len; i++)
        new_line->content[i] = _content[i];
    new_line->content[_len] = 0;

    #ifdef DEBUG
    new_line->__debug_content_to_str_();
    #endif

    new_line->replay_line_no = get_next_replay_line_no();

    return new_line;

}

//---------------------------------------------------------------------------------------
line_t *create_line(const char *_content)
{
    return create_line((char *)_content, strlen(_content));
    
}

//---------------------------------------------------------------------------------------
line_t *create_line(CHTYPE_PTR _content, size_t _len)
{
    line_t *new_line = (line_t *)malloc(sizeof(line_t));
    new_line->next = NULL;
    new_line->prev = NULL;
    size_t bytes = CHTYPE_SIZE * (_len+1);
    new_line->content = (CHTYPE_PTR)malloc(bytes);
    new_line->len = _len;
    memset(new_line->content, 0, bytes);
    memcpy(new_line->content, _content, bytes);
    for (size_t i = 0; i < _len; i++)
        new_line->content[i] = _content[i];
    new_line->content[_len] = 0;

    #ifdef DEBUG
    new_line->__debug_content_to_str_();
    #endif

    new_line->replay_line_no = get_next_replay_line_no();

    return new_line;

}

//---------------------------------------------------------------------------------------
line_t *copy_line(line_t *_line)
{
    line_t *line_copy = create_line(_line->content, _line->len);
    return line_copy;
}


//---------------------------------------------------------------------------------------
copy_line_t::copy_line_t(line_t *_line, bool _newline, bool _use_sel_offsets)
{
    // fow now, lets use __debug_str
    size_t offset0 = (_use_sel_offsets ? _line->sel_start : 0);
    size_t offset1 = (_use_sel_offsets ? _line->sel_end : _line->len);

    // len = _line->sel_end - _line->sel_start;
    len = offset1 - offset0;
    line_chars = (char *)malloc(len);
    // memcpy(line_chars, _line->__debug_str+_line->sel_start, len);
    memcpy(line_chars, _line->__debug_str+offset0, len);
    
    newline = _newline;

}

//---------------------------------------------------------------------------------------
void line_t::insert_char(char _c, size_t _pos)
{
    CHTYPE c = _c;
    insert_char(c, _pos);
    
}

//---------------------------------------------------------------------------------------
void line_t::insert_char(CHTYPE _c, size_t _pos)
{
    //
    if ((content = (CHTYPE_PTR)realloc(content, CHTYPE_SIZE * (len + 2))) == NULL) RAM_panic(this);
    memmove(content + _pos + 1, content + _pos, CHTYPE_SIZE * (len - _pos));
    content[_pos] = _c;
    content[++len] = '\0';

    #ifdef DEBUG
    __debug_content_to_str_();
    #endif
    
}

//---------------------------------------------------------------------------------------
void line_t::insert_str(char *_str, size_t _len, size_t _pos)
{
    CHTYPE_PTR str_ = (CHTYPE_PTR)malloc(CHTYPE_SIZE * _len);
    for (size_t i = 0; i < _len; i++)
       str_[i] = _str[i];
    
    insert_str(str_, _len, _pos);

    free(str_);

}

//---------------------------------------------------------------------------------------
void line_t::insert_str(CHTYPE_PTR _str, size_t _len, size_t _pos)
{

    if ((content = (CHTYPE_PTR)realloc(content, CHTYPE_SIZE * (len + _len + 1))) == NULL) RAM_panic(this);
    memmove(content + _pos + _len, content + _pos, CHTYPE_SIZE * (len - _pos));
    // TODO : add color here? -- no, surely on future onRowUpdate() callback?
    memcpy(content + _pos, _str, CHTYPE_SIZE * _len);
    len += _len;
    content[len] = 0;

    #ifdef DEBUG
    __debug_content_to_str_();
    #endif

}

//---------------------------------------------------------------------------------------
void line_t::append_line(line_t *_other)
{
    if ((content = (CHTYPE_PTR)realloc(content, CHTYPE_SIZE * (len + _other->len + 1))) == NULL) RAM_panic(this);
    memcpy(content + len, _other->content, CHTYPE_SIZE * _other->len);
    len = len + _other->len;
    content[len] = 0;

    #ifdef DEBUG
    __debug_content_to_str_();
    #endif

    // TODO : remove from map?
    free(_other);
    
}

//---------------------------------------------------------------------------------------
void line_t::delete_at(size_t _pos)
{
    if (!len || _pos < 0)
        return;

    memmove(content + _pos, content + _pos + 1, CHTYPE_SIZE * (len - _pos));
    len--;
    if ((content = (CHTYPE_PTR)realloc(content, CHTYPE_SIZE * (len + 1))) == NULL) RAM_panic(this);
    content[len] = 0;

    #ifdef DEBUG
    __debug_content_to_str_();
    #endif

}

//---------------------------------------------------------------------------------------
void line_t::delete_n_at(size_t _pos, size_t _n)
{
    if (!_n)
        return;
    
    assert(_pos + _n <= len);

    size_t bytes_to_move = (len - (_pos + _n)) * CHTYPE_SIZE;
    memmove(content + _pos, content + _pos + _n, bytes_to_move);
    len -= _n;
    if ((content = (CHTYPE_PTR)realloc(content, CHTYPE_SIZE * (len + 1))) == NULL) RAM_panic(this);
    content[len] = 0;

    #ifdef DEBUG
    __debug_content_to_str_();
    #endif

}

//---------------------------------------------------------------------------------------
line_t *line_t::split_at_pos(size_t _pos)
{
    size_t this_len = _pos;
    size_t next_len = len - _pos;

    line_t *new_line = create_line(content + _pos, next_len);

    if ((content = (CHTYPE_PTR)realloc(content, CHTYPE_SIZE * (this_len + 1))) == NULL) RAM_panic(this);
    len = this_len;
    content[len] = 0;

    #ifdef DEBUG
    __debug_content_to_str_();
    #endif

    return new_line;

}

//---------------------------------------------------------------------------------------
#ifdef DEBUG
void line_t::__debug_line()
{
    LOG_INFO("------------------------");
    LOG_INFO("next: %p", next);
    LOG_INFO("prev: %p", prev);
    LOG_INFO("content: %s", __debug_str);
    LOG_INFO("len = %zu", len);
    LOG_INFO("rlen = %zu", rlen);
    LOG_INFO("sel_start = %zu", sel_start);
    LOG_INFO("sel_end = %zu", sel_end);
}
#endif

//---------------------------------------------------------------------------------------
#ifdef DEBUG
void __debug_addchstr(API_WINDOW_PTR _w, const char *_fmt, ...)
{
    int x, y;
    getyx((WINDOW *)_w, y, x);
    char buffer0[256] = { 0 };
    va_list args;
    va_start(args, _fmt);
    int n = vsprintf(buffer0, _fmt, args);
    va_end(args);

    // convert to chtype
    CHTYPE_PTR print_buffer = (CHTYPE_PTR)malloc(CHTYPE_SIZE * (n + 1));
    for (size_t i = 0; i < n; i++)
    {
        print_buffer[i] = buffer0[i];
        addch((chtype)print_buffer[i]);
    }
    
    move(y+1, x);

    free(print_buffer);

}
#endif

//---------------------------------------------------------------------------------------
#ifdef DEBUG
void __debug_addchstr(API_WINDOW_PTR _w, CHTYPE_PTR _str)
{
    int x, y;
    getyx((WINDOW *)_w, y, x);

    CHTYPE_PTR p = _str;
    while(*p != '\0')
        addch((chtype)(*p++));
    // addchstr(_str);

    move(y+1, x);

}
#endif

//---------------------------------------------------------------------------------------
#ifdef DEBUG
void __debug_mvaddchstr(API_WINDOW_PTR _w, int _y, int _x, CHTYPE_PTR _str)
{
    int x, y;
    getyx((WINDOW *)_w, y, x);
    wmove((WINDOW *)_w, _y, _x);
    CHTYPE_PTR p = _str;
    while (*p != '\0')
        addch((chtype)(*p++));

    wmove((WINDOW *)_w, y, x);

}
#endif

//---------------------------------------------------------------------------------------
#ifdef DEBUG
const char *ctrlActionStr(CtrlKeyAction _action)
{
    switch (_action)
    {
        case CtrlKeyAction::CTRL_LEFT:          return "CTRL_LEFT";
        case CtrlKeyAction::CTRL_RIGHT:         return "CTRL_RIGHT";
        case CtrlKeyAction::CTRL_UP:            return "CTRL_UP";
        case CtrlKeyAction::CTRL_DOWN:          return "CTRL_DOWN";
        case CtrlKeyAction::CTRL_HOME:          return "CTRL_HOME";
        case CtrlKeyAction::CTRL_END:           return "CTRL_END";
        case CtrlKeyAction::CTRL_DEL:           return "CTRL_DELETE";
        case CtrlKeyAction::CTRL_SHIFT_DELETE:  return "CTRL_SHIFT_DELETE";
        case CtrlKeyAction::SHIFT_UP:           return "SHIFT_UP";
        case CtrlKeyAction::SHIFT_DOWN:         return "SHIFT_DOWN";
        case CtrlKeyAction::SHIFT_CTRL_LEFT:    return "SHIFT_CTRL_LEFT";
        case CtrlKeyAction::SHIFT_CTRL_RIGHT:   return "SHIFT_CTRL_RIGHT";
        case CtrlKeyAction::SHIFT_CTRL_UP:      return "SHIFT_CTRL_UP";
        case CtrlKeyAction::SHIFT_CTRL_DOWN:    return "SHIFT_CTRL_DOWN";
        case CtrlKeyAction::SHIFT_CTRL_HOME:    return "SHIFT_CTRL_HOME";
        case CtrlKeyAction::SHIFT_CTRL_END:     return "SHIFT_CTRL_END";
        case CtrlKeyAction::ALT_LEFT:           return "ALT_LEFT";
        case CtrlKeyAction::ALT_RIGHT:          return "ALT_RIGHT";
        case CtrlKeyAction::ALT_UP:             return "ALT_UP";
        case CtrlKeyAction::ALT_DOWN:           return "ALT_DOWN";
        case CtrlKeyAction::ALT_PAGEUP:         return "ALT_PAGEUP";
        case CtrlKeyAction::ALT_PAGEDOWN:       return "ALT_PAGEDOWN";
        case CtrlKeyAction::ALT_INSERT:         return "ALT_INSERT";
        case CtrlKeyAction::ALT_DELETE:         return "ALT_DELETE";
        case CtrlKeyAction::SHIFT_ALT_LEFT:     return "SHIFT_ALT_LEFT";
        case CtrlKeyAction::SHIFT_ALT_RIGHT:    return "SHIFT_ALT_RIGHT";
        case CtrlKeyAction::SHIFT_ALT_UP:       return "SHIFT_ALT_UP";
        case CtrlKeyAction::SHIFT_ALT_DOWN:     return "SHIFT_ALT_DOWN";
        default: return "(unknown ctrl_keycode_action)";

    }

}
#endif



