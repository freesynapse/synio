#ifndef __WINDOW_H
#define __WINDOW_H

#include <string>
#include <set>

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
    //Window() {}
    Window(const frame_t &_frame, const std::string &_id, bool _border=true);
    // Window(const ivec2_t &_v0, const ivec2_t &_v1, const std::string &_id, bool _border);
    virtual ~Window();
    
    // creates a border around the drawable area
    virtual void enableBorder();

    // Platform interactions
    virtual void resize(frame_t _new_frame);
    virtual void redraw() = 0;
    virtual void clear()
    { 
        if (!m_clearNextFrame)
            return;

        api->clearWindow(m_apiWindowPtr);
        if (m_apiBorderWindowPtr)
            api->clearWindow(m_apiBorderWindowPtr);
    }
    virtual void refresh()
    {
        if (!m_refreshNextFrame)
            return;

        if (m_apiBorderWindowPtr)
            api->refreshBorder(m_apiBorderWindowPtr);
        api->refreshWindow(m_apiWindowPtr);
    }

    // accessors
    const frame_t &frame() { return m_frame; }
    const std::string &ID() const { return m_ID; }
    void setVisibility(bool _b) { m_isWindowVisible = _b; }

    //
    #ifdef DEBUG
    void __debug_print(int _x, int _y, const char *_fmt, ...);
    #endif

protected:
    __always_inline void clear_next_frame_() { m_clearNextFrame = true; }
    __always_inline void refresh_next_frame_() { m_refreshNextFrame = true; }

protected:
    std::string m_ID = "";
    frame_t m_frame = frame_t(0);

    API_WINDOW_PTR m_apiWindowPtr = NULL;
    API_WINDOW_PTR m_apiBorderWindowPtr = NULL;

    bool m_isWindowVisible = true;
    bool m_clearNextFrame = true;
    bool m_refreshNextFrame = true;

    // vector of lines (in cursor corrdinates) in need of update (and thus clearing and 
    // re-rendering) to avoid using clear
    #ifdef NCURSES_IMPL
    std::set<int> m_linesUpdateList;
    #endif

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
    virtual void redraw() override;

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
    friend class Cursor;

public:

    Buffer(const frame_t &_frame, const std::string &_id, bool _border=false);
    ~Buffer();

    // callback for ScrollEvent -- called from synio.cpp
    void onScroll(BufferScrollEvent *_e);
    void scroll_(int _axis, int _dir, int _steps, bool _update_current_line=true);

    // Cursor functions
    //
    // move cursor inside window
    virtual void moveCursor(int _dx, int _dy);
    virtual void moveCursorToLineBegin();
    virtual void moveCursorToLineEnd();
    virtual void moveCursorToColDelim(int _dir);
    virtual void moveCursorToRowDelim(int _dir);
    virtual void insertCharAtCursor(char _c);
    virtual void insertStrAtCursor(char *_str, size_t _len);
    virtual void insertNewLine();
    virtual void deleteCharAtCursor();     // <DEL>
    virtual void deleteCharBeforeCursor(); // <BACKSPACE>

    // update window cursor (called during rendering and after keypress)
    virtual void updateCursor();
    // calculate the position of the cursor in the buffer
    void updateBufferCursorPos();
    // update the current line in the buffer based on cursor y movement
    // (called both my moveCursor() and onScroll())
    void updateCurrentLinePtr(int _dy);

    // read contents of a file into the buffer and set pointers
    void readFromFile(const std::string &_filename);

    // draw the buffer within window bounds using the BufferFormatter. Also draw Window
    // border (if any)
    virtual void resize(frame_t _new_frame) override;
    virtual void redraw() override;
    virtual void clear() override;
    virtual void refresh() override;

    // accessors
    const char *loadedFile() { return m_filename.c_str(); }
    line_t *currentLine() { return m_currentLine; }

private:
    void move_cursor_to_last_x_();
    bool is_delimiter_(const char *_delim, CHTYPE _c);
    bool is_row_empty_(line_t *_line);
    __always_inline void update_lines_after_y_(int _y)
    {
        for (int i = _y; i < m_frame.nrows; i++)
            m_linesUpdateList.insert(i);
    }
    

protected:
    std::string m_filename = "";

    LineBuffer m_lineBuffer;    // put into separate class and window -- for now only here
                                // also, could make it platform independent, i.e. provide 
                                // functions for windows, rendering etc for eg curses, GLFW
                                // SDL2 etc.

    line_t *m_currentLine   = NULL;
    line_t *m_pageFirstLine = NULL;
    line_t *m_pageLastLine  = NULL;

    Cursor m_cursor;

    ivec2_t m_bufferCursorPos;
    ivec2_t m_scrollPos;

    // int m_idxIntoContent = 0;

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
        m_frame = frame_t(ivec2_t(_x, _y0), ivec2_t(_x + 1, _y1));
        m_parent = _parent;
        m_apiBorderWindowPtr = api->newVerticalBarWindow(_x, _y0, _y1);
    }

    // virtual compulsory functions
    virtual void redraw() override {};

protected:
    Window *m_parent;

};
*/

#endif // __WINDOW_H
