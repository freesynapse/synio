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
    BufferWindowBase(const frame_t &_frame, const std::string &_id, int _wnd_params=false) :
        Window(_frame, _id, _wnd_params)
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
    virtual void moveCursor(int _dx, int _dy) {}
    virtual void moveCursorToLineBegin() {}
    virtual void moveCursorToLineEnd() {}
    virtual int findColDelim(int _dir, bool _move_cursor=true) { return 0; }
    virtual void insertCharAtCursor(char _c) {}
    virtual void insertCharAtPos(char _c, size_t _pos, bool _update_cursor=true) {}
    virtual void insertStrAtCursor(char *_str, size_t _len, bool _update_cursor) {}
    virtual void insertStrAtCursor(CHTYPE_PTR _str, size_t _len, bool _update_cursor) {}
    virtual void insertTab() {}
    virtual void removeLeadingTab() {}
    virtual void insertStructuralLiteral(char _c) {};
    virtual void deleteCharAtCursor() {}
    virtual void deleteToNextColDelim() {}
    virtual void deleteCharBeforeCursor() {}
    virtual void deleteToPrevColDelim() {}

    // (not used for single line buffers)
    virtual void moveCursorToRowDelim(int _dir) {}
    virtual void moveFileBegin() {}
    virtual void moveFileEnd() {}
    virtual void movePageUp() {}
    virtual void movePageDown() {}
    virtual void insertNewLine(bool _auto_align=true) {}

    // update cursor (called during rendering and after keypress)
    virtual void updateCursor() {};

    //
    virtual void cutSelection() {}
    virtual void deleteSelection() {}
    virtual void copySelection(std::vector<copy_line_t> *_store_buffer) {}
    virtual void paste() {}

protected:
    //
    bool is_col_delimiter_(CHTYPE _c)
    {
        for (size_t i = 0; i < strlen(Config::COL_DELIMITERS); i++)
           if ((_c & CHTYPE_CHAR_MASK) == Config::COL_DELIMITERS[i])
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
    char is_structural_literal_(CHTYPE _c)
    {
        for (size_t i = 0; i < strlen(Config::STRUCTURAL_LITERALS); i++)
            if ((_c & CHTYPE_CHAR_MASK) == Config::STRUCTURAL_LITERALS[i])
                return Config::STRUCTURAL_LITERALS[i];
        return 0;
    }

    //
    __always_inline int find_next_tab_stop_(int _pos) { return (_pos + (Config::TAB_SIZE - (_pos % Config::TAB_SIZE))); }
    __always_inline int find_prev_tab_stop_(int _pos) { return (_pos - (Config::TAB_SIZE + (_pos % Config::TAB_SIZE))) + (_pos % Config::TAB_SIZE ? Config::TAB_SIZE : 0); }
    
    //
    int find_indentation_level_(line_t *_line) { return find_first_non_empty_char_(_line); }

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

    //
    int find_empty_chars_from_pos_(int _pos, int _dir=FORWARD)//, line_t *_line=m_currentLine)
    {
        // if (_pos == m_currentLine->len && _dir < 0) _pos--;
        // char *s = _line->__debug_str;
        char *s = m_currentLine->__debug_str;
        int nspaces = 0;
        int x = _pos;
        // while (x >= 0 && x < _line->len)
        while (x >= 0 && x <= m_currentLine->len)
        {
            if (_dir < 0) x += _dir;
            
            char c = s[x];
            if (s[x] != ' ' && s[x] != '\t')
                break;
            else if (s[x] == ' ') nspaces++;
            else if (s[x] == '\t') nspaces += Config::TAB_SIZE;
            
            if (_dir > 0) x += _dir;
        }
        return nspaces;
        
    }
    
protected:
    line_t *m_currentLine = NULL;
    Cursor m_cursor;

};


#endif // __BUFFER_WINDOW_BASE_H