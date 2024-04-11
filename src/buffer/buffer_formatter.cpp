
#include "buffer_formatter.h"
#include "../core.h"
#include "../config.h"
#include "../utils/log.h"

//
void BufferFormatter::render(API_WINDOW_PTR _api_window, 
                             line_t *_first,
                             line_t *_current_line) // for row highlight
{
    // coordinates is relative to window, so we need to start at (0, 0)
    int x = 0;
    int y = 0;

    line_t *line = _first;

    line = _first;
    int len;

    while (line != NULL && y < m_windowRect->nrows)// && line != _last)
    {
        api->printBufferLine(_api_window, x, y, line->content, line->len);
        
        if (line == _current_line)
        {
            api->printString(_api_window, line->len, y, m_whiteSpaceBuffer, m_windowRect->v1.x - line->len);
            // ny funktion behövs här, typ printStr(_api_window, x=line->len, y, space_buffer -- kan vara del av klassen och vara jättestor)
        }

        y++;
        line = line->next;
    }

}
    