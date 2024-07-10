#ifndef __FORMATTER_H
#define __FORMATTER_H

#include "line_buffer.h"
#include "../types.h"
#include "../platform/ncurses_colors.h"

// could also be platform agnostic, requires more work though. For now ncurses ftw.
class BufferFormatter
{
public:
    BufferFormatter() :
        m_windowRect(0)
    {}
    BufferFormatter(frame_t *_render_rect) :
        m_windowRect(_render_rect)
    {}
    ~BufferFormatter() {}

    void render(API_WINDOW_PTR _api_window, line_t *_first, line_t *_current=NULL,
                int _x_offset=0, int _y_offset=0);

private:
    frame_t *m_windowRect;

};




#endif // __FORMATTER_H
