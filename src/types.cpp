
#include <stdlib.h>
#include <string.h>

#include "types.h"

//
void line_t::insert_char(char _c, int _pos)
{
    if ((content = (char *)realloc(content, len + 2)) == NULL) RAM_panic(this);

    memmove(content + _pos + 1, content + _pos, len - _pos);
    content[_pos] = _c;
    content[++len] = '\0';

}

//
void line_t::insert_str(char *_str, size_t _len, int _pos)
{
    if ((content = (char *)realloc(content, len + _len + 1)) == NULL) RAM_panic(this);

    memmove(content + _pos + _len, content + _pos, len - _pos);
    memcpy(content + _pos, _str, _len);
    len += _len;
    content[len] = '\0';

}

//
void line_t::delete_at(int _pos)
{
    if (!len)
        return;

    int offset[2];

    if (_pos > 0)   { offset[0] = -1; offset[1] = 0; }
    else            { offset[0] =  0; offset[1] = 1; }
    
    memmove(content + _pos + offset[0], content + _pos + offset[1], len - _pos);
    len--;
    if ((content = (char *)realloc(content, len + 1)) == NULL) RAM_panic(this);
    content[len] = '\0';

}

//
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


