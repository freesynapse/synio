#ifndef __BUFFER_WINDOW_H
#define __BUFFER_WINDOW_H

#include "window.h"
#include "line_numbers_window.h"
#include "../cursor.h"


//
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
    void scroll_(int _axis, int _dir, int _steps, bool _update_current_line);

    // Cursor functions
    //
    // move cursor inside window
    virtual void moveCursor(int _dx, int _dy);
    virtual void moveCursorToLineBegin();
    virtual void moveCursorToLineEnd();
    virtual void moveCursorToColDelim(int _dir);
    virtual void moveCursorToRowDelim(int _dir);
    virtual void pageUp();
    virtual void pageDown();
    virtual void moveHome();
    virtual void moveEnd();
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
    virtual void resize(frame_t _new_frame, int _left_reserved=-1);
    virtual void redraw() override;
    virtual void clear() override;
    virtual void refresh() override;

    // accessors
    const char *loadedFile() { return m_filename.c_str(); }
    line_t *currentLine() { return m_currentLine; }

private:
    bool is_delimiter_(const char *_delim, CHTYPE _c);
    bool is_row_empty_(line_t *_line);
    __always_inline void update_lines_after_y_(int _y)
    {
        for (int i = _y; i < m_frame.nrows; i++)
            m_linesUpdateList.insert(i);
    }
    int find_indentation_level_(line_t *_line);
    int find_first_non_empty_char_(line_t *_line);
    

protected:
    std::string m_filename = "";

    LineBuffer m_lineBuffer;    // put into separate class and window -- for now only here
                                // also, could make it platform independent, i.e. provide 
                                // functions for windows, rendering etc for eg curses, GLFW
                                // SDL2 etc.

    line_t *m_currentLine   = NULL;
    line_t *m_pageFirstLine = NULL;

    Cursor m_cursor;

    ivec2_t m_bufferCursorPos;
    ivec2_t m_scrollPos;

    // int m_idxIntoContent = 0;

    BufferFormatter m_formatter;

    LineNumbers *m_lineNumbers = NULL;

};





#endif // __BUFFER_WINDOW_H
