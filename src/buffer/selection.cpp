
#include "selection.h"

#include <assert.h>

#include "../platform/ncurses_colors.h"


//
void Selection::clear()
{
    for (line_t *entry : m_entries)
        ncurses_deselect_substr(entry, 0, entry->len);
    
    m_entries.clear();
    m_startingBufferPos = ivec2_t(-1);

}

//---------------------------------------------------------------------------------------
void Selection::selectChars(line_t *_line, size_t _start, size_t _end)
{
    select_(_line, _start, _end);

}

//---------------------------------------------------------------------------------------
void Selection::selectLines(line_t *_start_line,
                            size_t _start_offset,
                            line_t *_end_line,
                            size_t _end_offset)
{
    line_t *p = _start_line;
    if (p->len - _start_offset >= 0)
        select_(p, _start_offset, p->len);
    p = p->next;
    while (p != NULL)
    {
        if (p == _end_line)
        {
            // if (_end_offset > 0)
                select_(p, 0, _end_offset);
            // else
            // {
                // p->sel_start = 0;   // can't understand why this is needed...
                // p->sel_end = 0;
            // }
            break;
        }
        else
            select_(p, 0, p->len);

        p = p->next;

    }

}

//---------------------------------------------------------------------------------------
void Selection::select_(line_t *_line, size_t _start, size_t _end)
{
    // select / deselect (depending)
    ncurses_toggle_selection_substr(_line, _start, _end);

    // update correct line selection start and end
    ncurses_find_selected_offsets(_line, &_line->sel_start, &_line->sel_end);

    //
    m_entries.insert(_line);

}

//---------------------------------------------------------------------------------------
void Selection::expandSelection(line_t *_line, size_t _start_expand, size_t _end_expand)
{
    ncurses_toggle_selection_substr(_line, _start_expand, _end_expand);
    ncurses_find_selected_offsets(_line, &_line->sel_start, &_line->sel_end);
    m_entries.insert(_line);

}