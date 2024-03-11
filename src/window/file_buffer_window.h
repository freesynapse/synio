#ifndef __BUFFER_WINDOW_H
#define __BUFFER_WINDOW_H

#include "buffer_window_base.h"
#include "line_numbers_window.h"
#include "../cursor.h"
#include "../buffer/selection.h"


//
class FileBufferWindow : public BufferWindowBase
{
public:
    friend class LineNumbers;
    friend class Selection; 

public:

    FileBufferWindow(const frame_t &_frame, const std::string &_id, bool _border=false);
    ~FileBufferWindow();

    // (BufferWindowBase overrides)
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;
    virtual void scroll_(int _axis, int _dir, int _steps, bool _update_current_line) override;

    // Cursor functions (BufferWindowBase overrides)
    virtual void moveCursor(int _dx, int _dy) override;
    virtual void moveCursorToLineBegin() override;
    virtual void moveCursorToLineEnd() override;
    virtual void moveCursorToColDelim(int _dir) override;
    virtual void moveCursorToRowDelim(int _dir) override;
    virtual void movePageUp() override;
    virtual void movePageDown() override;
    virtual void moveFileBegin() override;
    virtual void moveFileEnd() override;
    virtual void insertCharAtCursor(char _c) override;
    virtual void insertStrAtCursor(char *_str, size_t _len) override;
    virtual void insertNewLine() override;
    virtual void deleteCharAtCursor() override;
    virtual void deleteCharBeforeCursor() override;
    //
    virtual void updateCursor() override;
    //
    void gotoBufferCursorPos(const ivec2_t &_pos);


    // calculate the position of the cursor in the buffer
    void updateBufferCursorPos();
    // update the current line in the buffer based on cursor y movement
    // (called both my moveCursor() and onScroll())
    void updateCurrentLinePtr(int _dy);

    // read contents of a file into the buffer and set pointers
    void readFromFile(const std::string &_filename);

    // Window class overrides
    virtual void resize(frame_t _new_frame, int _left_reserved=-1);
    virtual void redraw() override;
    virtual void clear() override;
    virtual void refresh() override;

    // accessors
    const char *loadedFile() { return m_filename.c_str(); }
    line_t *currentLine() { return m_currentLine; }

private:
    //
    __always_inline void update_lines_after_y_(int _y)
    {
        for (int i = _y; i < m_frame.nrows; i++)
            m_linesUpdateList.insert(i);
    }
    //
    __always_inline void buffer_changed_() { m_isDirty = true; }
    //
    __always_inline void select_()
    {
        if (!m_isSelecting)
            m_selection->setStartingBufferPos(m_bufferCursorPos);
        m_isSelecting = true;
    }
    //
    // keys NOT restoring pos : arrow down, arrow right and derivatives threrof (ctrl+ etc)
    __always_inline void deselect_(bool _restore_pos=true)
    {
        if (m_isSelecting && _restore_pos)
            gotoBufferCursorPos(m_selection->startingBufferPos());
        m_selection->clear();
        m_isSelecting = false; 
    }

protected:
    std::string m_filename = "";
    bool m_isDirty = false;

    //
    LineBuffer m_lineBuffer;
    //m_currentLine in BufferWindowBase
    line_t *m_prevLine = NULL;
    line_t *m_pageFirstLine = NULL;
    
    //
    Cursor m_cursor;
    ivec2_t m_bufferCursorPos;
    ivec2_t m_prevBufferCursorPos;
    ivec2_t m_scrollPos;

    //
    BufferFormatter m_formatter;

    // selections; one (1) for now, could be array later? Or put in Selection class (probably)
    Selection *m_selection = NULL;
    bool m_isSelecting = false;

    // Line numbers window accompanying this one
    LineNumbers *m_lineNumbers = NULL;

};





#endif // __BUFFER_WINDOW_H
