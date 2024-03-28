#ifndef __BUFFER_WINDOW_H
#define __BUFFER_WINDOW_H

#include "buffer_window_base.h"
#include "line_numbers_window.h"
#include "../cursor.h"
#include "../buffer/selection.h"
#include "../buffer/lexer.h"

//
class FileBufferWindow : public BufferWindowBase
{
public:
    friend class LineNumbers;
    friend class Selection; 

public:

    FileBufferWindow(const frame_t &_frame, const std::string &_id, bool _border=false);
    ~FileBufferWindow();

    #ifdef DEBUG
    void __debug_fnc();
    #endif

    // (BufferWindowBase overrides)
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;
    virtual void scroll_(int _axis, int _dir, int _steps, bool _current_line) override;

    // Cursor functions (BufferWindowBase overrides)
    virtual void moveCursor(int _dx, int _dy) override;
    virtual void moveCursorToLineBegin() override;
    virtual void moveCursorToLineEnd() override;
    virtual int findColDelim(int _dir, bool _move_cursor=true) override;
    virtual void moveCursorToRowDelim(int _dir) override;
    virtual void movePageUp() override;
    virtual void movePageDown() override;
    virtual void moveFileBegin() override;
    virtual void moveFileEnd() override;
    virtual void insertCharAtCursor(char _c) override;
    virtual void insertStrAtCursor(char *_str, size_t _len) override;
    virtual void insertStrAtCursor(CHTYPE_PTR _str, size_t _len) override;
    virtual void insertNewLine(bool _auto_align=true) override;
    virtual void insertTab() override;
    virtual void removeLeadingTab() override;
    virtual void insertStructuralLiteral(char _c) override;
    virtual void deleteCharAtCursor() override;
    virtual void deleteToNextColDelim() override;
    virtual void deleteCharBeforeCursor() override;
    virtual void deleteToPrevColDelim() override;

    //
    virtual void updateCursor() override;
    virtual void copy() override;
    virtual void deleteSelection() override;
    virtual void cut() override;
    virtual void paste() override;

    // inserts a new line and updates the cursor
    void insertLineAtCursor(char *_content, size_t _len);
    // for restoring cursor position after selections/cutting
    void gotoBufferCursorPos(const ivec2_t &_pos);
    // calculate the position of the cursor in the buffer
    void updateBufferCursorPos();
    // update the current line in the buffer based on cursor y movement
    // (called both my moveCursor() and onScroll())
    void updateCurrentLinePtr(int _dy);

    // load/save buffer
    void readFileToBuffer(const std::string &_filename);
    void saveBufferToFile();

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
            m_windowLinesUpdateList.insert(i);
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
    // do not restore pos if action is aligned with direction of selection
    __always_inline void deselect_(int _move_direction)
    {
        if (m_isSelecting && m_dirOfSelection != _move_direction)
            gotoBufferCursorPos(m_selection->startingBufferPos());
        m_selection->clear();
        m_isSelecting = false; 
    }
    // deletes selected text, for standard keypresses. Returns number of deleted lines
    __always_inline int delete_selection_()
    {
        int deleted = m_selection->lineCount();
        if (deleted)
            deleteSelection();
        return deleted;
    }
    // specific for multi-line buffers; syntax HL next frame (in redraw()).
    __always_inline void syntax_highlight_buffer_() { m_syntaxHLNextFrame = true; }
    // inserts a tab into a line
    __always_inline void insert_leading_tab_(line_t *_line)
    {
        char buffer[Config::TAB_SIZE];
        memset(buffer, ' ', Config::TAB_SIZE);
        _line->insert_str(buffer, Config::TAB_SIZE, 0);        
    }
    // removes a leading tab from line
    __always_inline void remove_leading_tab_(line_t *_line) {}

protected:
    std::string m_filename = "";
    bool m_isDirty = false;
    bool m_syntaxHLNextFrame = false;

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

    // output and syntax highlighting
    BufferFormatter m_formatter;
    Lexer m_lexer;

    // TODO : selections; one (1) for now, could be array later? Or put in Selection class (probably not though)
    Selection *m_selection = NULL;
    bool m_isSelecting = false;
    int m_dirOfSelection = FORWARD;

    std::vector<copy_line_t> m_copyBuffer;

    // Line numbers window accompanying this one
    LineNumbers *m_lineNumbers = NULL;

};


#endif // __BUFFER_WINDOW_H
