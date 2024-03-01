
#include "line_buffer_window.h"
#include "../platform/ncurses_colors.h"


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
    free(m_query);
}

//---------------------------------------------------------------------------------------
void LineBufferWindow::setQuery(const std::string &_query, const ivec2_t &_pos)
{
    m_queryLen = _query.length();
    m_queryPos = _pos;
    
    if (m_query) free(m_query);
    m_query = (CHTYPE_PTR)malloc(CHTYPE_SIZE * m_queryLen + 1);

    for (size_t i = 0; i < m_queryLen; i++)
    {
        #ifdef NCURSES_IMPL
        m_query[i] = CHTYPE(_query[i] | COLOR_PAIR(SYNIO_COLOR_TEXT));
        #else
        m_query[i] = _query[i];
        #endif
    }
    m_query[m_queryLen] = 0;

    m_cursor.set_cx((int)m_queryLen);
    m_cursor.set_cy(m_queryPos.y);

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{
    if (_ctrl_action != CtrlKeyAction::NONE)
    {
        switch (_ctrl_action)
        {
            case CtrlKeyAction::CTRL_LEFT:  moveCursorToColDelim(-1);   break;
            case CtrlKeyAction::CTRL_RIGHT: moveCursorToColDelim(1);    break;
            case CtrlKeyAction::CTRL_UP:    moveCursorToRowDelim(-1);   break;
            case CtrlKeyAction::CTRL_DOWN:  moveCursorToRowDelim(1);    break;
            // default: LOG_INFO("ctrl keycode %d : %s", _c, ctrlActionStr(_ctrl_action)); break;
            default: break;

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
            
            case 10:    // <ENTER>
                dispatchEvent();
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
    if (_dx < 0)
        LOG_INFO("");

    int dx = _dx;
    int cx = m_cursor.cx();
    
    if (cx == m_queryLen && dx < 0) dx = 0;
    else if (cx == m_queryLen + m_currentLine->len && dx > 0) dx = 0;

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
void LineBufferWindow::moveCursorToColDelim(int _dir)
{


}

//---------------------------------------------------------------------------------------
void LineBufferWindow::insertCharAtCursor(char _c)
{
    m_currentLine->insert_char(_c, m_cursor.cx());
    moveCursor(1, 0);

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void LineBufferWindow::insertStrAtCursor(char *_str, size_t _len)
{
    m_currentLine->insert_str(_str, _len, m_cursor.cpos().x);
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
    if (m_isWindowVisible)
    {
        api->printBufferLine(m_apiWindowPtr, m_queryPos.x, m_queryPos.y, m_query, m_queryLen);
        api->clearBufferLine(m_apiWindowPtr, m_cursor.frame_v0_x(), m_queryPos.y, m_frame.ncols - 1);
        api->printBufferLine(m_apiWindowPtr, m_cursor.frame_v0_x(), m_queryPos.y, m_currentLine->content, m_currentLine->len);

    }

    updateCursor();

}
