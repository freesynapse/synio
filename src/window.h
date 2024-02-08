#ifndef __WINDOW_H
#define __WINDOW_H

#include <string>

#include "core.h"
#include "types.h"
#include "cursor.h"
#include "utils/log.h"
#include "buffer_formatter.h"
#include "line_buffer.h"
#include "platform/platform_impl.h"
#include "file_io.h"
#include "events.h"

//
class Window
{
public:
    friend class Cursor;
    friend class Synio;

public:
    Window() {}
    ~Window() = default;

    //
    virtual void draw() = 0;
    virtual void clear() = 0;
    virtual void refresh() = 0;
    __always_inline virtual void scrollBuffer(const ivec2_t &_delta) = 0;

    // accessors
    const irect_t &frame() { return m_frame; }
    const std::string &ID() const { return m_ID; }

protected:
    std::string m_ID = "";
    irect_t m_frame = irect_t(0);

    ivec2_t m_bufferCursorPos;
    ivec2_t m_scrollPos;

    API_WINDOW_PTR m_apiWindowPtr = NULL;

    LineBuffer m_lineBuffer;    // put into separate class and window -- for now only here
                                // also, could make it platform independent, i.e. provide 
                                // functions for windows, rendering etc for eg curses, GLFW
                                // SDL2 etc.

    line_t *m_currentLine   = NULL;
    line_t *m_pageFirstLine = NULL;
    line_t *m_pageLastLine  = NULL;

    BufferFormatter m_formatter;

};

//
class BufferWindow : public Window
{
public:
    BufferWindow(irect_t *_frame, const std::string &_id)
    {
        m_frame = *_frame;
        m_ID = _id;

        m_apiWindowPtr = api->newWindow(_frame);
        m_formatter = BufferFormatter(_frame);

        #ifdef DEBUG
        LOG_INFO("%s: %s [%p] frame = (%d, %d) -- (%d, %d)\n",
                 __func__, m_ID.c_str(), this,
                 m_frame.v0.x, m_frame.v0.y,
                 m_frame.v1.x, m_frame.v1.y);
        #endif
    }
    ~BufferWindow() = default;

    //
    void onScroll(BufferScrollEvent *_e)
    {
        ivec2_t scroll = { 0, 0 };
        switch (_e->axis())
        {
            case X_AXIS:
                // m_scrollPos.x += e->steps();
                scroll.x = _e->dir() < 0 ? -_e->steps() : _e->steps();
                break;

            case Y_AXIS:
                // adjust pointers
                int n = 0;
                // scroll up
                if (_e->dir() < 0)
                {
                    while (n < _e->steps() && m_pageFirstLine->prev != NULL)
                    {
                        m_pageFirstLine = m_pageFirstLine->prev;
                        m_pageLastLine = m_pageLastLine->prev;
                        n++;
                    }

                }
                // scroll down
                else
                {
                    while (n < _e->steps() && m_pageLastLine->next != NULL)
                    {
                        m_pageFirstLine = m_pageFirstLine->next;
                        m_pageLastLine = m_pageLastLine->next;
                        n++;
                    }

                }

                scroll.y = _e->dir() < 0 ? -_e->steps() : _e->steps();
                break;
        }

        scrollBuffer(scroll);

    }

    // read contents of a file into the buffer and set pointers
    void readFromFile(const char *_filename)
    {
        FileIO::readFileIntoBuffer(_filename, &m_lineBuffer);
        // line pointers
        m_currentLine = m_lineBuffer.m_head;
        m_pageFirstLine = m_lineBuffer.m_head;
        m_pageLastLine = m_lineBuffer.ptrFromIdx(DEBUG_N_LINES+1);

        line_t *p = m_currentLine;
        int y = 0;
        while (p != NULL)
        {
            LOG_INFO("%3d : %s\n", y++, p->content);
            p = p->next;
        }

    }

    // draw the buffer within window bounds using the BufferFormatter. Also draw Window
    // border (if any)
    virtual void draw() override
    {
        m_formatter.render(m_apiWindowPtr, &m_lineBuffer, m_pageFirstLine, m_pageLastLine);

    }

    virtual void clear() override { api->clearWindow(m_apiWindowPtr); }
    virtual void refresh() override { api->refreshWindow(m_apiWindowPtr); }


    // update scroll position -- the callback is in Synio.h|.cpp
    __always_inline virtual void scrollBuffer(const ivec2_t &_delta) override { m_scrollPos += _delta; }

};





#endif // __WINDOW_H
