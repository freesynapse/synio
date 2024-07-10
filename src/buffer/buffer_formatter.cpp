
#include "buffer_formatter.h"
#include "../core.h"
#include "../config.h"
#include "../utils/log.h"

//
void BufferFormatter::render(API_WINDOW_PTR _api_window, 
                             line_t *_first,
                             line_t *_current_line,
                             int _x_offset,
                             int _y_offset)
{
    // Coordinates is relative to window, so we need to start at (0, 0)
    // By owning a separate frame_t, different rendering coordinates from the actual 
    // window is encouraged.
    int x = _x_offset;
    int y = _y_offset;
    
    line_t *line = _first;

    // while (line != NULL && y < m_windowRect->nrows)
    while (line != NULL && y <= m_windowRect->nrows)
    {
        
        api->printBufferLine(_api_window, x, y, line->content, line->len);

        y++;
        line = line->next;
    }

}
