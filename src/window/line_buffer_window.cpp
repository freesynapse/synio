
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
    // change lines if at beginning or end of line
    if (_dir > 0 && m_cursor.cx() >= m_currentLine->len - 1)    { if (_move_cursor) moveCursor( 1, 0); return  1; }
    else if (_dir < 0 && m_cursor.cx() <= 1)                    { if (_move_cursor) moveCursor(-1, 0); return -1; }

    int cx = m_cursor.cx();
    int dx = 0;
    //
    CHTYPE_PTR p = m_currentLine->content + cx;
    bool start_on_delimiter = is_col_delimiter_(*p);

    p += _dir;
    start_on_delimiter = is_col_delimiter_(*p);
    dx += _dir;
    char c = (*p & CHTYPE_CHAR_MASK);
    // starting on a delimiter -> skip all delimiters and then find the next
    if (start_on_delimiter)
    {
        while (is_col_delimiter_((*p & CHTYPE_CHAR_MASK)) && cx + dx < m_currentLine->len && cx + dx >= 0)
        {
            p += _dir;
            c = (*p & CHTYPE_CHAR_MASK);
            dx += _dir;
        }
    }
    // just jump to next delimiter
    else
    {
        while (!is_col_delimiter_((*p & CHTYPE_CHAR_MASK)) && cx + dx < m_currentLine->len && cx + dx >= 0)
        {
            p += _dir;
            c = (*p & CHTYPE_CHAR_MASK);
            dx += _dir;
        }

    }

    // special case where the first char is preceeded by only whitespace (i.e. tabs),
    // then move to the first actual character
    // if (_dir < 0 && find_first_non_empty_char_(m_currentLine) > cx + dx)
    // Actually, we want to jump to the non-delimieter when going backwards, always
    if (_dir < 0)
        dx++;

    if (_move_cursor)
        moveCursor(dx, 0);

    return dx;

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::insertCharAtCursor(char _c)
{
    // only allowed characters (for now)
    if (Config::ALLOWED_CHAR_SET.find(_c) == Config::ALLOWED_CHAR_SET.end())
    {
        moveCursor(0, 0);
        return;
    }

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
    int cx = m_cursor.cx();

    int nspaces = find_empty_chars_from_pos_(cx, FORWARD);
    int next_delim_pos_dx = findColDelim(FORWARD, false);
    
    int ndels = next_delim_pos_dx;
    if (nspaces < next_delim_pos_dx && nspaces > 0)
        ndels = nspaces;
    
    //
    if (cx + ndels <= m_currentLine->len)
        m_currentLine->delete_n_at(cx, ndels);

    m_windowLinesUpdateList.insert(m_cursor.cy());

    refresh_next_frame_();

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
    int prev_delim_pos_dx = findColDelim(BACKWARD, false);
    int cx = m_cursor.cx();
    int nspaces = find_empty_chars_from_pos_(cx, BACKWARD);

    if (abs(prev_delim_pos_dx) > m_currentLine->len)
        prev_delim_pos_dx++;
    
    int ndels = abs(prev_delim_pos_dx);
    if (nspaces < abs(prev_delim_pos_dx) && nspaces > 0)
        ndels = nspaces;
    
    m_currentLine->delete_n_at(cx - ndels, ndels);

    moveCursor(-ndels, 0);
    // m_windowLinesUpdateList.insert(m_cursor.cy());

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
