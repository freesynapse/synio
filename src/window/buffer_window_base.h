#ifndef __BUFFER_WINDOW_BASE_H
#define __BUFFER_WINDOW_BASE_H

#include "window.h"
#include "../types.h"

// Base class for every window centered around an editable buffer.
//
class BufferWindowBase : public Window
{
public:
    friend class Cursor;

public:
    BufferWindowBase(const frame_t &_frame, const std::string &_id, bool _border=false) :
        Window(_frame, _id, _border)
    { 
        m_cursor = Cursor(this);
    }
    ~BufferWindowBase() = default;

    // interaction with the outside
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override {};

    // multiline, editable, buffer windows
    virtual void scroll_(int _axis, int _dir, int _steps, bool _update_current_line) {}

    // Cursor functions
    //
    // (move cursor in line)
    virtual void moveCursor(int _dx, int _dy) {}
    virtual void moveCursorToLineBegin() {}
    virtual void moveCursorToLineEnd() {}
    virtual void moveCursorToColDelim(int _dir) {}
    virtual void insertCharAtCursor(char _c) {}
    virtual void insertStrAtCursor(char *_str, size_t _len) {}
    virtual void insertStrAtCursor(CHTYPE_PTR _str, size_t _len) {}
    virtual void deleteCharAtCursor() {}
    virtual void deleteCharBeforeCursor() {}

    // (not used for single line buffers)
    virtual void moveCursorToRowDelim(int _dir) {}
    virtual void moveFileBegin() {}
    virtual void moveFileEnd() {}
    virtual void movePageUp() {}
    virtual void movePageDown() {}
    virtual void insertNewLine() {}

    // update cursor (called during rendering and after keypress)
    virtual void updateCursor() {};

    //
    virtual void cut() {}
    virtual void deleteSelection() {}
    virtual void copy() {}
    virtual void paste() {}

protected:
    //
    bool is_delimiter_(const char *_delim, CHTYPE _c)
    {
        for (size_t i = 0; i < strlen(_delim); i++)
            if ((_c & CHTYPE_CHAR_MASK) == _delim[i])
                return true;
        return false;
    }

    //
    bool is_row_empty_(line_t *_line)
        {
        CHTYPE_PTR p = _line->content;
        while ((*p & CHTYPE_CHAR_MASK) != '\0')
        {
            if ((*p & CHTYPE_CHAR_MASK) != ' ' && 
                (*p & CHTYPE_CHAR_MASK) != '\t')
                return false;
            p++;
        }

        return true;
    }

    //
    int find_indentation_level_(line_t *_line)
    {
        int first_char_idx = find_first_non_empty_char_(_line);
        
        if ((_line->content[first_char_idx] & CHTYPE_CHAR_MASK) == '{')
            return first_char_idx + Config::TAB_SIZE;
        
        return first_char_idx;
    }

    //
    int find_first_non_empty_char_(line_t *_line)
    {
        CHTYPE_PTR p = _line->content;
        int x = 0;
        while (((*p & CHTYPE_CHAR_MASK) != '\0'))
        {
            if ((*p & CHTYPE_CHAR_MASK) == ' ')
                x++;
            else if ((*p & CHTYPE_CHAR_MASK) == '\t')
                x += Config::TAB_SIZE;
            else if ((*p & CHTYPE_CHAR_MASK) != ' ' && 
                    (*p & CHTYPE_CHAR_MASK) != '\t')
                break;

            p++;
        }

        return x;

    }
    
protected:
    line_t *m_currentLine = NULL;
    Cursor m_cursor;

};


#endif // __BUFFER_WINDOW_BASE_H