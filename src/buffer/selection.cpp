
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
    selection_entry_t entry(_start_line, _offset, _n);
    m_entries.push_back(entry);
    select_deselect_(&entry, SELECT);

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
