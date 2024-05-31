
#include "line_buffer_window.h"

#include <assert.h>

#include "../platform/ncurses_colors.h"
#include "../utils/str_utils.h"


//
LineBufferWindow::LineBufferWindow(const frame_t &_frame,
                                   const std::string &_id,
                                   bool _border) :
    BufferWindowBase(_frame, _id, _border)
{
    m_currentLine = create_line("");

}

//---------------------------------------------------------------------------------------
LineBufferWindow::~LineBufferWindow()
{ 
    delete m_currentLine;

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{
    if (_ctrl_action != CtrlKeyAction::NONE)
    {
        switch (_ctrl_action)
        {
            case CtrlKeyAction::CTRL_LEFT:  findColDelim(-1);   break;
            case CtrlKeyAction::CTRL_RIGHT: findColDelim(1);    break;
            case CtrlKeyAction::CTRL_UP:    moveCursorToRowDelim(-1);   break;
            case CtrlKeyAction::CTRL_DOWN:  moveCursorToRowDelim(1);    break;
            default: LOG_WARNING("ctrl keycode %d : %s", _c, ctrlActionStr(_ctrl_action)); break;
            // default: break;

        }
    }
    else
    {
        switch (_c)
        {
            // cursor movement
            case KEY_LEFT:  moveCursor(-1, 0);          break;
            case KEY_RIGHT: moveCursor(1, 0);           break;
            case KEY_HOME:  moveCursorToLineBegin();    break;
            case KEY_END:   moveCursorToLineEnd();      break;

            case KEY_DC:
                deleteCharAtCursor();
                break;

            case KEY_BACKSPACE:
                deleteCharBeforeCursor();
                break;
            
            default:
                insertCharAtCursor((char)_c);
                break;

        }
    }

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::moveCursor(int _dx, int _dy)
{
    int dx = _dx;
    int cx = m_cursor.cx();
    
    if (cx == 0 && dx < 0) dx = 0;
    else if (cx == m_currentLine->len && dx > 0) dx = 0;

    // move the cursor (if possible)
    m_cursor.move(dx, 0);

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::moveCursorToLineBegin()
{
    moveCursor(-m_cursor.cx());

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::moveCursorToLineEnd()
{
    moveCursor(m_currentLine->len - m_cursor.cx());

}

//---------------------------------------------------------------------------------------
int LineBufferWindow::findColDelim(int _dir, bool _move_cursor)
{
    // TODO : implement me!
    // assert(1 == 0);
    return -1;
 
#if 0
    if (_dir > 0 && m_cursor.cx() == m_currentLine->len - 1)    { moveCursor(1, 0); return;  }
    else if (_dir > 0 && m_cursor.cx() == m_currentLine->len)   { moveCursor(1, 0); return;  }
    else if (_dir < 0 && m_cursor.cx() == 1)                    { moveCursor(-1, 0); return; }
    else if (_dir < 0 && m_cursor.cx() == 0)                    { moveCursor(-1, 0); return; }

    CHTYPE_PTR p = m_currentLine->content + m_cursor.cx();
    size_t line_len = m_currentLine->len;
    int x = (_dir < 0 ? m_cursor.cx() : 0);
    bool on_delimiter = is_col_delimiter_(*p);
    bool do_continue = (_dir < 0 ? x > 0 : (*p & CHTYPE_CHAR_MASK) != 0);

    //
    while (do_continue)
    {
        x += _dir;
        p += _dir;

        // -- DEBUG
        char c = (*p & CHTYPE_CHAR_MASK);
        char c2 = (*(p + _dir) & CHTYPE_CHAR_MASK);

        // if starting on a delimiter,<>=!,,, we search for non-delimiters
        if (on_delimiter && is_col_delimiter_(*p))
        {
            while (is_col_delimiter_(*p) && do_continue)
            {
                x += _dir;
                p += _dir;

                do_continue = (_dir < 0 ? x > 0 : (*p & CHTYPE_CHAR_MASK) != 0);
            }
            x -= 1;
            break;
        }
        else if (on_delimiter && !is_col_delimiter_(*p))
        {
            // ex: 'char *p = ...'
            // find next delimiter
            while (!is_col_delimiter_(*p) && do_continue)
            {
                x += _dir;
                p += _dir;

                do_continue = (_dir < 0 ? x > 0 : (*p & CHTYPE_CHAR_MASK) != 0);
            }
            break;
        }
        else // not starting on delimiter
        {
            if (is_col_delimiter_(*p))
                break;
        }

        do_continue = (_dir < 0 ? x > 0 : (*p & CHTYPE_CHAR_MASK) != 0);
    }

    int dx = (_dir < 0 ? -(m_cursor.cx() - x) : x);
    moveCursor(dx);
#endif
    return 0;

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::insertCharAtCursor(char _c)
{
    m_currentLine->insert_char(_c, m_cursor.cx());
    moveCursor(1, 0);

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::insertStrAtCursor(char *_str, size_t _len, bool _update_cursor)
{
    m_currentLine->insert_str(_str, _len, m_cursor.cpos().x);
    
    if (_update_cursor)
        moveCursor(_len, 0);

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::deleteCharAtCursor()
{
    // <DEL>
    if (m_cursor.cpos().x == m_currentLine->len)
        return;

    if (m_cursor.cpos().x < m_currentLine->len)
        m_currentLine->delete_at(m_cursor.cpos().x);

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::deleteToNextColDelim()
{
    // TODO : implement me!
    // assert(1 == 0);
}

//---------------------------------------------------------------------------------------
void LineBufferWindow::deleteCharBeforeCursor()
{
    // <BACKSPACE>    
    if (m_cursor.cx() == 0)
    {
        m_cursor.move(0, 0);
        refresh_next_frame_();
        return;
    }
    else if (m_cursor.cx() > 0) // inside line
    {
        m_cursor.move(-1, 0);
        m_currentLine->delete_at(m_cursor.cx() + 1);

    }

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::deleteToPrevColDelim()
{
    // TODO : implement me!
    assert(1 == 0);
}

//---------------------------------------------------------------------------------------
void LineBufferWindow::updateCursor()
{
    static ivec2_t prev_pos = m_cursor.cpos();

    // actually move the cursor
    m_cursor.update();

    //
    if (prev_pos != m_cursor.cpos())
    {
        refresh_next_frame_();
        prev_pos = m_cursor.cpos();
    }

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::redraw()
{
    if (!m_isWindowVisible)
        return;

    // print query prefix
    // api->printBufferLine(m_apiWindowPtr, m_cmdPrefixPos.x, m_cmdPrefixPos.y, m_cmdPrefix->str, m_cmdPrefix->len);
    
    // clear line and draw query
    // api->clearSpace(m_apiWindowPtr, m_cursor.offset_x(), m_cmdPrefixPos.y, m_frame.ncols - 1 - m_cursor.cx());
    // api->printBufferLine(m_apiWindowPtr, m_cursor.offset_x(), m_cmdPrefixPos.y, m_currentLine->content, m_currentLine->len);

    // updateCursor();

}
