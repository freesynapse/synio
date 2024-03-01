
#include "selection.h"

#include <assert.h>

#include "../platform/ncurses_colors.h"


//
void Selection::clear_all()
{

}

//---------------------------------------------------------------------------------------
void Selection::select_sequence_()
{
    line_t *line = m_startLine;
    while (line->next != NULL)
    {
        // last line of selection
        if (line == m_endLine)
        {
            select_substr_(line, 0, m_endOffset);
            break;
        }
        // first line of selection, account for offset into line
        else if (line == m_startLine)
        {
            select_substr_(line, m_startOffset, line->len);
        }
        // in the middle of multi-line selection, select the whole line
        else if (line != m_startLine && line != m_endLine)
        {
            select_substr_(line, 0, line->len);
        }
        // ?
        else
        {
            LOG_INFO("how did we get here?!");
        }

        line = line->next;
    }
}

//---------------------------------------------------------------------------------------
void Selection::select_substr_(line_t *_line, size_t _start, size_t _end)
{
    assert(_start > 0 && _end <= _line->len);
    for (size_t i = _start; i < _end; i++)
    {
        // A_COLOR = 0x0000ff00 (masking bit 9..16)
        int16_t cp_idx = (_line->content[i] & A_COLOR) >> 8;
        // printw("char %c : cp_idx = %d --> %d\n", _s->content[i], cp_idx, cp_idx + SELECTION_OFFSET);
        _line->content[i] = (_line->content[i] | COLOR_PAIR(cp_idx + SELECTION_OFFSET));
        
    }

}

