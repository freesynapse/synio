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
        m_windowRect(0) {}
    BufferFormatter(frame_t *_render_rect) :
        m_windowRect(_render_rect) {}
    ~BufferFormatter() {}

    void render(API_WINDOW_PTR _api_window, line_t *_first, line_t *_current);

private:
    #define WHITE_SPACE_BUFFER_LEN 384
    void init_white_space_buffer_()
    {
        memset(m_whiteSpaceBuffer, 0, CHTYPE_SIZE * WHITE_SPACE_BUFFER_LEN);
        CHTYPE c = ' ';
        c = ((c & ~CHTYPE_COLOR_MASK) | COLOR_PAIR(SYN_COLOR_HLROW));
        for (size_t i = 0; i < WHITE_SPACE_BUFFER_LEN; i++)
            m_whiteSpaceBuffer[i] = c;
    }

private:
    frame_t *m_windowRect;
    CHTYPE m_whiteSpaceBuffer[WHITE_SPACE_BUFFER_LEN];

};




#endif // __FORMATTER_H
