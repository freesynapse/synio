
#include "buffer_window_base.h"

//---------------------------------------------------------------------------------------
bool BufferWindowBase::is_delimiter_(const char *_delim, CHTYPE _c)
{
    for (size_t i = 0; i < strlen(_delim); i++)
        if ((_c & CHTYPE_CHAR_MASK) == _delim[i])
            return true;
    return false;
    
}

//---------------------------------------------------------------------------------------
bool BufferWindowBase::is_row_empty_(line_t *_line)
{
    CHTYPE_PTR p = _line->content;
    while ((*p & CHTYPE_CHAR_MASK) != '\0')
    {
        if ((*p & CHTYPE_CHAR_MASK) != ' ' && 
            (*p & CHTYPE_CHAR_MASK) != '\t')
            return false;
        p++;
    }

    return true;

}

//---------------------------------------------------------------------------------------
int BufferWindowBase::find_indentation_level_(line_t *_line)
{
    int first_char_idx = find_first_non_empty_char_(_line);
    
    if ((_line->content[first_char_idx] & CHTYPE_CHAR_MASK) == '{')
        return first_char_idx + Config::TAB_SIZE;
    
    return first_char_idx;
        
}

//---------------------------------------------------------------------------------------
int BufferWindowBase::find_first_non_empty_char_(line_t *_line)
{
    CHTYPE_PTR p = _line->content;
    int x = 0;
    while (((*p & CHTYPE_CHAR_MASK) != '\0'))
    {
        if ((*p & CHTYPE_CHAR_MASK) == ' ')
            x++;
        else if ((*p & CHTYPE_CHAR_MASK) == '\t')
            x += Config::TAB_SIZE;
        else if ((*p & CHTYPE_CHAR_MASK) != ' ' && 
                 (*p & CHTYPE_CHAR_MASK) != '\t')
            break;

        p++;
    }

    return x;

}
