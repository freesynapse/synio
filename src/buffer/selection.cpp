
#include "selection.h"

#include <assert.h>

#include "../platform/ncurses_colors.h"


//
void Selection::clear()
{
    for (auto &entry : m_entries)
    {
        // auto entry = sel.second;
        // select_deselect_(entry.line, entry.offset, entry.nchars, DESELECT);
        select_deselect_(&entry, DESELECT);
    }
    
    m_entries.clear();

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
        selection_entry_t entry(_start_line, _offset, _n);
        m_entries.push_back(entry);
        select_deselect_(&entry, SELECT);
    }
}

//---------------------------------------------------------------------------------------
void Selection::addRegion(line_t *_start_line,
                          int _start_offset,
                          line_t *_end_line,
                          int _end_offset)
{
    line_t *p = _start_line;
    // add first line
    add(p, _start_offset, p->len - _start_offset);
    p = p->next;
    // loop
    while (p != _end_line)
    {
        add(p, 0, p->len);
        p = p->next;
    }
    // add end line
    add(p, 0, _end_offset);
    
}

//---------------------------------------------------------------------------------------
void Selection::select_deselect_(selection_entry_t *_entry, int _selecting)
{
    #ifdef NCURSES_IMPL
    ncurses_select_deselect_substr(_entry->line, 
                                   _entry->offset,
                                   _entry->offset + _entry->nchars, 
                                   _selecting);
    #endif
}
