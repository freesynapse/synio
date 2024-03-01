
#include "selection.h"

#include <assert.h>

#include "../platform/ncurses_colors.h"


//
void Selection::clear_all()
{
    select_deselect_sequence_(false);

}

//---------------------------------------------------------------------------------------
void Selection::select_deselect_sequence_(bool _selecting)
{
    line_t *line = m_startLine;
    while (line->next != NULL)
    {
        // last line of selection
        if (line == m_endLine)
        {
            #ifdef NCURSES_IMPL
            ncurses_select_deselect_substr(line, 0, m_endOffset, _selecting);
            #endif

            break;
        }
        // first line of selection, account for offset into line
        else if (line == m_startLine)
        {
            #ifdef NCURSES_IMPL
            ncurses_select_deselect_substr(line, m_startOffset, line->len, _selecting);
            #endif
        }
        // in the middle of multi-line selection, select the whole line
        else if (line != m_startLine && line != m_endLine)
        {
            #ifdef NCURSES_IMPL
            ncurses_select_deselect_substr(line, 0, line->len, _selecting);
            #endif
        }
        // ?
        else
        {
            LOG_INFO("how did we get here?!");
        }

        line = line->next;
    }
}

