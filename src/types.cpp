
#include <stdlib.h>
#include <string.h>

#include "types.h"

//
line_t *create_line(char *_content, size_t _len)
{
    line_t *new_line = (line_t *)malloc(sizeof(line_t));
    new_line->next = NULL;
    new_line->prev = NULL;
    new_line->content = (char *)malloc(_len+1);
    new_line->len = _len;
    memset(new_line->content, 0, _len+1);
    memcpy(new_line->content, _content, _len);

    return new_line;

}

//---------------------------------------------------------------------------------------
line_t *create_line(const char *_content)
{
    return create_line((char *)_content, strlen(_content));
}

//---------------------------------------------------------------------------------------
void line_t::insert_char(char _c, size_t _pos)
{
    if ((content = (char *)realloc(content, len + 2)) == NULL) RAM_panic(this);

    memmove(content + _pos + 1, content + _pos, len - _pos);
    content[_pos] = _c;
    content[++len] = '\0';

}

//---------------------------------------------------------------------------------------
void line_t::insert_str(char *_str, size_t _len, size_t _pos)
{
    if ((content = (char *)realloc(content, len + _len + 1)) == NULL) RAM_panic(this);

    memmove(content + _pos + _len, content + _pos, len - _pos);
    memcpy(content + _pos, _str, _len);
    len += _len;
    content[len] = '\0';

}

//---------------------------------------------------------------------------------------
void line_t::delete_at(size_t _pos)
{
    if (!len)
        return;

    int offset[2];

    if (_pos > 0) { offset[0] = -1; offset[1] = 0; }
    else          { offset[0] =  0; offset[1] = 1; }
    
    memmove(content + _pos + offset[0], content + _pos + offset[1], len - _pos);
    len--;
    if ((content = (char *)realloc(content, len + 1)) == NULL) RAM_panic(this);
    content[len] = '\0';

}

//---------------------------------------------------------------------------------------
line_t *line_t::split_at_pos(size_t _pos)
{
    size_t this_len = _pos;
    size_t next_len = len - _pos;

    line_t *new_line = create_line(content + _pos, next_len);

    if ((content = (char *)realloc(content, this_len + 1)) == NULL) RAM_panic(this);
    len = this_len;
    content[len] = '\0';

    return new_line;

}

//---------------------------------------------------------------------------------------
#ifdef DEBUG
void line_t::__debug_print(bool _show_ptrs, const char *_str)
{
    if (strcmp(_str, "") != 0)
        LOG_INFO("%s", _str);
    LOG_INFO("%p: [%s]", this, content);
    if (_show_ptrs)
    {
        LOG_INFO("    next: %s", next == NULL ? "NULL" : next->content);
        LOG_INFO("    prev: %s", prev == NULL ? "NULL" : prev->content);
    }

}
#endif

//---------------------------------------------------------------------------------------
#ifdef DEBUG
const char *ctrlActionStr(CtrlKeycodeAction _action)
{
    switch (_action)
    {
        case CtrlKeycodeAction::CTRL_LEFT:          return "CTRL_LEFT";
        case CtrlKeycodeAction::CTRL_RIGHT:         return "CTRL_RIGHT";
        case CtrlKeycodeAction::CTRL_UP:            return "CTRL_UP";
        case CtrlKeycodeAction::CTRL_DOWN:          return "CTRL_DOWN";
        case CtrlKeycodeAction::CTRL_HOME:          return "CTRL_HOME";
        case CtrlKeycodeAction::CTRL_END:           return "CTRL_END";
        case CtrlKeycodeAction::CTRL_DELETE:        return "CTRL_DELETE";
        case CtrlKeycodeAction::CTRL_SHIFT_DELETE:  return "CTRL_SHIFT_DELETE";
        case CtrlKeycodeAction::SHIFT_UP:           return "SHIFT_UP";
        case CtrlKeycodeAction::SHIFT_DOWN:         return "SHIFT_DOWN";
        case CtrlKeycodeAction::SHIFT_CTRL_LEFT:    return "SHIFT_CTRL_LEFT";
        case CtrlKeycodeAction::SHIFT_CTRL_RIGHT:   return "SHIFT_CTRL_RIGHT";
        case CtrlKeycodeAction::SHIFT_CTRL_UP:      return "SHIFT_CTRL_UP";
        case CtrlKeycodeAction::SHIFT_CTRL_DOWN:    return "SHIFT_CTRL_DOWN";
        case CtrlKeycodeAction::SHIFT_CTRL_HOME:    return "SHIFT_CTRL_HOME";
        case CtrlKeycodeAction::SHIFT_CTRL_END:     return "SHIFT_CTRL_END";
        case CtrlKeycodeAction::ALT_LEFT:           return "ALT_LEFT";
        case CtrlKeycodeAction::ALT_RIGHT:          return "ALT_RIGHT";
        case CtrlKeycodeAction::ALT_UP:             return "ALT_UP";
        case CtrlKeycodeAction::ALT_DOWN:           return "ALT_DOWN";
        case CtrlKeycodeAction::ALT_PAGEUP:         return "ALT_PAGEUP";
        case CtrlKeycodeAction::ALT_PAGEDOWN:       return "ALT_PAGEDOWN";
        case CtrlKeycodeAction::ALT_INSERT:         return "ALT_INSERT";
        case CtrlKeycodeAction::ALT_DELETE:         return "ALT_DELETE";
        case CtrlKeycodeAction::SHIFT_ALT_LEFT:     return "SHIFT_ALT_LEFT";
        case CtrlKeycodeAction::SHIFT_ALT_RIGHT:    return "SHIFT_ALT_RIGHT";
        case CtrlKeycodeAction::SHIFT_ALT_UP:       return "SHIFT_ALT_UP";
        case CtrlKeycodeAction::SHIFT_ALT_DOWN:     return "SHIFT_ALT_DOWN";
        default: return "(unknown ctrl_keycode_action)";

    }

}
#endif


