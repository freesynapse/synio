#ifndef __WINDOW_H
#define __WINDOW_H

#include <string>

#include "../core.h"
#include "../types.h"
#include "../cursor.h"
#include "../utils/log.h"
#include "../utils/file_io.h"
#include "../buffer_formatter.h"
#include "../line_buffer.h"
#include "../platform/platform.h"
#include "../events.h"
#include "../config.h"

//
class Window
{
public:
    friend class WCursor;
    friend class Cursor;
    friend class Synio;

public:
    Window() {}
    // Window(irect_t *_frame, const std::string &_id, bool _border=true);
    Window(const irect_t &_frame, const std::string &_id, bool _border=true);
    // Window(const ivec2_t &_v0, const ivec2_t &_v1, const std::string &_id, bool _border);
    ~Window();
    
    // creates a border around the drawable area
    virtual void enableBorder();

    // cursor functions
    virtual void moveCursor(int _dx, int _dy);
    virtual void moveCursorToLineBegin();
    virtual void moveCursorToLineEnd();
    virtual void moveCursorToColDelim(int _dir) {}
    virtual void moveCursorToRowDelim(int _dir) {}
    virtual void insertCharAtCursor(char _c) {}
    virtual void insertStrAtCursor(char *_str, size_t _len) {}
    virtual void insertNewLine() {}
    virtual void deleteCharAtCursor() {}        // <DEL>
    virtual void deleteCharBeforeCursor() {}    // <BACKSPACE>

    // update window cursor (called during rendering and after keypress)
    virtual void updateCursor();
    
    // virtual compulsory functions
    virtual void draw() = 0;

    // Platform interactions
    virtual void clear() { api->clearWindow(m_apiWindowPtr); }
    virtual void refresh()
    {
        if (m_apiBorderWindowPtr)
            api->refreshBorder(m_apiBorderWindowPtr);
        api->refreshWindow(m_apiWindowPtr);
    }

    // accessors
    const irect_t &frame() { return m_frame; }
    const std::string &ID() const { return m_ID; }
    WCursor &cursor() { return m_cursor; }
    void setVisibility(bool _b) { m_isWindowVisible = _b; }

    //
    #ifdef DEBUG
    #define __DEBUG_BUFFER_LEN 256
    char __debug_buffer[__DEBUG_BUFFER_LEN];
    void __debug_print(int _x, int _y, const char *_fmt, ...);
    #endif

protected:
    std::string m_ID = "";
    irect_t m_frame = irect_t(0);

    WCursor m_cursor;
    ivec2_t m_bufferCursorPos;
    ivec2_t m_scrollPos;

    API_WINDOW_PTR m_apiWindowPtr = NULL;
    API_WINDOW_PTR m_apiBorderWindowPtr = NULL;

    bool m_isWindowVisible = true;

};

/*
 * Acts as a sub-window in the Buffer class, only draws line numbers
 */
class Buffer;
class LineNumbers : public Window
{
public:
    using Window::Window;

    // virtual compulsory functions
    virtual void draw() override;

    // set associated buffer
    void setBuffer(Buffer *_buffer) { m_associatedBuffer = _buffer; }

private:
    Buffer *m_associatedBuffer = NULL;

};

/*
 * The Buffer class, showing the actual text.
 */
class Buffer : public Window
{
public:
    friend class LineNumbers;

public:

    Buffer(const irect_t &_frame, const std::string &_id, bool _border=true);
    ~Buffer();

    // callback for ScrollEvent -- called from synio.cpp
    void onScroll(BufferScrollEvent *_e);

    // Cursor functions
    //
    // move cursor inside window
    virtual void moveCursor(int _dx, int _dy) override;
    virtual void moveCursorToLineBegin() override;
    virtual void moveCursorToLineEnd() override;
    virtual void moveCursorToColDelim(int _dir) override;
    virtual void moveCursorToRowDelim(int _dir) override;
    virtual void insertCharAtCursor(char _c) override;
    virtual void insertStrAtCursor(char *_str, size_t _len) override;
    virtual void insertNewLine() override;
    virtual void deleteCharAtCursor() override;     // <DEL>
    virtual void deleteCharBeforeCursor() override; // <BACKSPACE>

    // update window cursor (called during rendering and after keypress)
    virtual void updateCursor() override;
    // calculate the position of the cursor in the buffer
    void updateBufferCursorPos();
    // update the current line in the buffer based on cursor y movement
    // (called both my moveCursor() and onScroll())
    void updateCurrentLinePtr(int _dy);

    // read contents of a file into the buffer and set pointers
    void readFromFile(const std::string &_filename);

    // draw the buffer within window bounds using the BufferFormatter. Also draw Window
    // border (if any)
    virtual void draw() override;
    virtual void clear() override;
    virtual void refresh() override;

    // accessors
    const char *loadedFile() { return m_filename.c_str(); }
    line_t *currentLine() { return m_currentLine; }

private:
    void move_cursor_to_last_x_();
    bool is_delimiter_(const char *_delim, char _c);
    bool is_row_empty_(line_t *_line);

protected:
    std::string m_filename = "";

    LineBuffer m_lineBuffer;    // put into separate class and window -- for now only here
                                // also, could make it platform independent, i.e. provide 
                                // functions for windows, rendering etc for eg curses, GLFW
                                // SDL2 etc.

    line_t *m_currentLine   = NULL;
    line_t *m_pageFirstLine = NULL;
    line_t *m_pageLastLine  = NULL;
    int m_lastCursorX = 0;

    BufferFormatter m_formatter;

    LineNumbers *m_lineNumbers = NULL;

};

// TODO : vertical bar implementation
// Bars -- treated as one (1) column or row Window with custom border
/*
class VerticalBar : public Window
{
public:
    VerticalBar(int _x, int _y0, int _y1, Window *_parent=NULL)
    {
        m_frame = irect_t(ivec2_t(_x, _y0), ivec2_t(_x + 1, _y1));
        m_parent = _parent;
        m_apiBorderWindowPtr = api->newVerticalBarWindow(_x, _y0, _y1);
    }

    // virtual compulsory functions
    virtual void draw() override {};

protected:
    Window *m_parent;

};
*/

#endif // __WINDOW_H