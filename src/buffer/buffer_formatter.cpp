
#include "buffer_formatter.h"
#include "../core.h"
#include "../config.h"
#include "../utils/log.h"

//
void BufferFormatter::render(API_WINDOW_PTR _api_window, 
                             line_t *_first,
                             line_t *_last)
{
    // coordinates is relative to window, so we need to start at (0, 0)
    int x = 0;
    int y = 0;

    line_t *line = _first;

    line = _first;
    int len;

    while (line != NULL && y < m_windowRect->nrows)// && line != _last)
    {
        //LOG_INFO("len of line=%d", line->len);
        //if ((line->content[0] & CHTYPE_CHAR_MASK) == '\t')
        //    LOG_INFO("found a tab!");
        api->printBufferLine(_api_window, x, y++, line->content, line->len);
        line = line->next;
    }

}
    