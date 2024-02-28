#ifndef __BUFFER_WINDOW_BASE_H
#define __BUFFER_WINDOW_BASE_H

#include "window.h"
#include "../types.h"

// base class for every window centered around an editable buffer
//
class BufferWindowBase : public Window
{
public:
    friend class Cursor;

public:
    BufferWindowBase(const frame_t &_frame, const std::string &_id, bool _border=false) :
        Window(_frame, _id, _border)
    {}
    ~BufferWindowBase() = default;

    // multiline, editable, buffer windows
    virtual void scroll_(int _axis, int _dir, int _steps, bool _update_current_line) {}

    //
    // Cursor functions
    //
    // move cursor inside window
    virtual void moveCursor(int _dx, int _dy) {}
    virtual void moveCursorToLineBegin() {}
    virtual void moveCursorToLineEnd() {}
    virtual void moveCursorToColDelim(int _dir) {}
    virtual void moveCursorToRowDelim(int _dir) {}
    virtual void movePageUp() {}
    virtual void movePageDown() {}
    virtual void moveHome() {}
    virtual void moveEnd() {}
    virtual void insertCharAtCursor(char _c) {}
    virtual void insertStrAtCursor(char *_str, size_t _len) {}
    virtual void insertNewLine() {}
    virtual void deleteCharAtCursor() {}
    virtual void deleteCharBeforeCursor() {}

    // update window cursor (called during rendering and after keypress)
    virtual void updateCursor() {};



protected:
    line_t *m_currentLine = NULL;

};







#endif // __BUFFER_WINDOW_BASE_H