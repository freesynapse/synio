
#include "buffer_formatter.h"
#include "../core.h"
#include "../config.h"
#include "../utils/log.h"

//
void BufferFormatter::render(API_WINDOW_PTR _api_window, 
                             line_t *_first,
                             line_t *_current_line,
                             int _x_offset)
{
    // coordinates is relative to window, so we need to start at (0, 0)
    int x = _x_offset;
    int y = 0;

    line_t *line = _first;

    while (line != NULL && y < m_windowRect->nrows)
    {
        
        api->printBufferLine(_api_window, x, y, line->content, line->len);

        y++;
        line = line->next;
    }

}
