
#include "buffer_formatter.h"
#include "core.h"

//
void BufferFormatter::render(LineBuffer *_buffer, line_t *_first, line_t *_last)
{
    int x = m_windowRect.v0.x;
    int y = m_windowRect.v0.y;
    
    line_t *line = _first;

    while (line != NULL && y <= m_windowRect.v1.y && line != _last)
    {
        api->printBufferLine(x, y++, line->content);
        line = line->next;

    }



}
    