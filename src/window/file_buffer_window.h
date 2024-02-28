#ifndef __BUFFER_WINDOW_H
#define __BUFFER_WINDOW_H

#include "buffer_window_base.h"
#include "line_numbers_window.h"
#include "../cursor.h"


//
class FileBufferWindow : public BufferWindowBase
{
public:
    friend class LineNumbers;
    friend class Cursor;

public:

    FileBufferWindow(const frame_t &_frame, const std::string &_id, bool _border=false);
    ~FileBufferWindow();

    //
    void scroll_(int _axis, int _dir, int _steps, bool _update_current_line) override;

    // Cursor functions
    //
    // move cursor inside window
    virtual void moveCursor(int _dx, int _dy) override;
    virtual void moveCursorToLineBegin() override;
    virtual void moveCursorToLineEnd() override;
    virtual void moveCursorToColDelim(int _dir) override;
    virtual void moveCursorToRowDelim(int _dir) override;
    virtual void movePageUp() override;
    virtual void movePageDown() override;
    virtual void moveHome() override;
    virtual void moveEnd() override;
    virtual void insertCharAtCursor(char _c) override;
    virtual void insertStrAtCursor(char *_str, size_t _len) override;
    virtual void insertNewLine() override;
    virtual void deleteCharAtCursor() override;
    virtual void deleteCharBeforeCursor() override;

    // update window cursor (called during rendering and after keypress)
    virtual void updateCursor() override;
    // calculate the position of the cursor in the buffer
    void updateBufferCursorPos();
    // update the current line in the buffer based on cursor y movement
    // (called both my moveCursor() and onScroll())
    void updateCurrentLinePtr(int _dy);

    // read contents of a file into the buffer and set pointers
    void readFromFile(const std::string &_filename);

    //
    virtual void resize(frame_t _new_frame, int _left_reserved=-1);// <-- override?
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

    //
    MultiLineBuffer m_lineBuffer;
    //m_currentLine in BufferWindowBase
    line_t *m_pageFirstLine = NULL;
    
    //
    Cursor m_cursor;
    ivec2_t m_bufferCursorPos;
    ivec2_t m_scrollPos;

    //
    BufferFormatter m_formatter;

    //
    LineNumbers *m_lineNumbers = NULL;

};





#endif // __BUFFER_WINDOW_H
