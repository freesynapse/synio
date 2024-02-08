
#include "buffer_formatter.h"
#include "core.h"
#include "utils/log.h"

//
void BufferFormatter::render(API_WINDOW_PTR _api_window, 
                            //  LineBuffer *_buffer,
                             line_t *_first,
                             line_t *_last)
{
    int x = m_windowRect.v0.x+1;   // account for borders
    int y = m_windowRect.v0.y+1;
    
    line_t *line = _first;

    while (line != NULL && y <= m_windowRect.v1.y && line != _last)
    {
        api->printBufferLine(_api_window, x, y++, line->content);
        // LOG_INFO("%3d : %s\n", y-1, line->content);        
        line = line->next;

    }

}
    