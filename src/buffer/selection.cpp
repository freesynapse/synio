
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
        select_deselect_(entry, DESELECT);
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
        select_deselect_(entry, SELECT);
    }
}

//---------------------------------------------------------------------------------------
void Selection::selectChars(line_t *_start_line, int _offset, int _n)
{
    // selects n characters from startline, even if _n > _start_line->len
    
    // only selecting in the first line?
    if (_n <= _start_line->len - _offset)
        select_deselect_(selection_entry_t(_start_line, _offset, _n), SELECT);
    
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
void Selection::select_deselect_(const selection_entry_t &_sel, int _selecting)
{
    m_entries.push_back(_sel);

    #ifdef NCURSES_IMPL
    ncurses_select_deselect_substr(_sel.line,
                                   _sel.offset,
                                   _sel.offset + _sel.nchars,
                                   _selecting);
    #endif

}
