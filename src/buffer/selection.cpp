
#include "selection.h"

#include <assert.h>

#include "../platform/ncurses_colors.h"


//
void Selection::clear()
{
    // TODO : thread this for large files?
    for (line_t *entry : m_entries)
    {
        // auto entry = sel.second;
        select_deselect_(entry, 0, entry->len, DESELECT);
        // select_deselect_(entry, DESELECT);
    }
    
    m_entries.clear();
    m_startingBufferPos = ivec2_t(-1);

}

//---------------------------------------------------------------------------------------
void Selection::add(line_t *_start_line, int _offset, int _n)
{
    if (_n > 0)
    {
        if (_n > _start_line->len - _offset)
        {
            int n = _n;
            int offset = _offset;
            line_t *p = _start_line;
            while (n > 0)
            {

                n -= p->len - offset;
                p = p->next;

            }
        }
        select_deselect_(_start_line, _offset, _n, SELECT);

    }
}

//---------------------------------------------------------------------------------------
void Selection::selectChars(line_t *_start_line, int _offset, int _n)
{
    // selects n characters from startline, even if _n > _start_line->len
    
    // only selecting in the first line?
    if (_n <= _start_line->len - _offset)
        select_deselect_(_start_line, _offset, _n, SELECT);
    
    // text from more than one line is selected
    else
    {
        line_t *p = _start_line;
        int n = _n;
        // first select the first line (get rid of the offset)
        select_deselect_(p, _offset, p->len - _offset, SELECT);
        n -= (p->len - _offset);
        p = p->next;

        // now do the rest of the characters
        while (n > 0 && p != NULL)
        {
            if (n > p->len)
            {
                select_deselect_(p, 0, p->len, SELECT);
                n -= p->len;
            }
            // last line of selection
            else
            {
                select_deselect_(p, 0, n, SELECT);
                n = 0;
            }

            p = p->next;

        }
    }

}

//---------------------------------------------------------------------------------------
void Selection::selectLineChars(line_t *_line, int _offset, int _n)
{
    assert(_offset + _n <= _line->len);
    select_deselect_(_line, _offset, _n, SELECT);
    
}

//---------------------------------------------------------------------------------------
void Selection::selectLines(line_t *_start_line,
                            int _start_offset,
                            line_t *_end_line,
                            int _end_offset)
{
    line_t *p = _start_line;
    select_deselect_(p, _start_offset, p->len - _start_offset, SELECT);
    while (p != NULL)
    {
        if (p == _end_line)
        {
            select_deselect_(p, 0, _end_offset, SELECT);
            break;
        }
        else
            select_deselect_(p, 0, p->len, SELECT);

        p = p->next;

    }

}

//---------------------------------------------------------------------------------------
void Selection::select_deselect_(line_t *_line, int _offset, int _n, int _selecting)
{
    // save_selection_entry_(_entry);
    m_entries.insert(_line);

    #ifdef NCURSES_IMPL
    ncurses_select_deselect_substr(_line,
                                   _offset,
                                   _offset + _n,
                                   _selecting);
    #else
    assert(0 && "implement me!");
    #endif

}
