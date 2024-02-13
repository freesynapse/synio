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
    friend class WCursor;
    friend class Cursor;
    friend class Synio;

public:
    Window() {}
    Window(irect_t *_frame, const std::string &_id, bool _border=true);
    ~Window();
    
    // creates a border around the drawable area
    virtual void enableBorder();

    // cursor functions
    virtual void moveCursor(int _dx, int _dy);
    virtual void moveCursorToLineBegin();
    virtual void moveCursorToLineEnd();
    virtual void insertCharAtCursor(char _c) {}
    virtual void insertStrAtCursor(char *_str, size_t _len) {}
    virtual void deleteCharAtCursor() {}        // <DEL>
    virtual void deleteCharBeforeCursor() {}    // <BACKSPACE>

    // update window cursor (called during rendering and after keypress)
    virtual void updateCursor();
    
    // virtual compulsory functions
    virtual void draw() = 0;
    virtual void clear() = 0;
    virtual void refresh() = 0;

    // accessors
    const irect_t &frame() { return m_frame; }
    const std::string &ID() const { return m_ID; }
    WCursor &cursor() { return m_cursor; }
    line_t *currentLine() { return m_currentLine; }

protected:
    std::string m_ID = "";
    irect_t m_frame = irect_t(0);

    WCursor m_cursor;
    ivec2_t m_bufferCursorPos;
    ivec2_t m_scrollPos;

    API_WINDOW_PTR m_apiWindowPtr = NULL;
    API_WINDOW_PTR m_apiBorderWindowPtr = NULL;

    LineBuffer m_lineBuffer;    // put into separate class and window -- for now only here
                                // also, could make it platform independent, i.e. provide 
                                // functions for windows, rendering etc for eg curses, GLFW
                                // SDL2 etc.

    line_t *m_currentLine   = NULL;
    line_t *m_pageFirstLine = NULL;
    line_t *m_pageLastLine  = NULL;

    BufferFormatter m_formatter;

    bool m_isVisible = true;

};

//
class Buffer : public Window
{
public:
    using Window::Window;

    // callback for ScrollEvent -- called from synio.cpp
    void onScroll(BufferScrollEvent *_e);

    // Cursor functions
    //
    // move cursor inside window
    virtual void moveCursor(int _dx, int _dy) override;
    virtual void moveCursorToLineBegin() override;
    virtual void moveCursorToLineEnd() override;
    virtual void insertCharAtCursor(char _c) override;
    virtual void insertStrAtCursor(char *_str, size_t _len) override;
    virtual void deleteCharAtCursor() override;     // <DEL>
    virtual void deleteCharBeforeCursor() override; // <BACKSPACE>

    // update window cursor (called during rendering and after keypress)
    virtual void updateCursor() override;
    // calculate the position of the cursor in the buffer
    void bufferCursorPos();
    // update the current line in the buffer based on cursor y movement
    // (called both my moveCursor() and onScroll())
    void updateCurrentLinePtr(int _dy);


    // read contents of a file into the buffer and set pointers
    void readFromFile(const char *_filename);

    // draw the buffer within window bounds using the BufferFormatter. Also draw Window
    // border (if any)
    virtual void draw() override;

    // Platform interactions
    virtual void clear() override { api->clearWindow(m_apiWindowPtr); }
    virtual void refresh() override
    {
        if (m_apiBorderWindowPtr)
            api->refreshBorder(m_apiBorderWindowPtr);
        api->refreshWindow(m_apiWindowPtr);
    }

    // accessors
    const char *loaded_filename() { return m_filename.c_str(); }


private:
    std::string m_filename = "";

};





#endif // __WINDOW_H
