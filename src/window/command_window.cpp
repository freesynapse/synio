
#include "command_window.h"

#include <assert.h>


//
CommandWindow::CommandWindow(const frame_t &_frame,
                             const std::string &_id,
                             bool _border) :
    LineBufferWindow(_frame, _id, _border)
{

}

//---------------------------------------------------------------------------------------
CommandWindow::~CommandWindow()
{ 
    // delete m_currentLine;    // deleted in parent (LineBufferWindow)
    delete m_query;

}

//---------------------------------------------------------------------------------------
void CommandWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
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
            
            case 10:    // <ENTER>
                dispatchEvent();
                break;

            case CTRL('s'):
                setQueryPrefix("Save as (filename):");
                break;

            case CTRL('o'):
                setQueryPrefix("Open (filename):");
                break;

            default:
                insertCharAtCursor((char)_c);
                break;

        }
    }

}

//---------------------------------------------------------------------------------------
void CommandWindow::setQueryPrefix(const char *_prefix)
{
    if (strcmp(_prefix, "") == 0)
    {
        LOG_WARNING("empty command entered.");
        return;
    }

    delete m_query;

    m_query = new CHTYPE_STR(_prefix);
    assert(m_query->len < m_frame.ncols);

    m_cursor.set_offset(ivec2_t(m_query->len + 1, 0));

}

//---------------------------------------------------------------------------------------
void CommandWindow::appendPrefix(const char *_str)
{
    if (m_query->len == 0)
    {
        setQueryPrefix(_str);
        return;
    }

    // add an extra space
    m_query->append(" ");
    m_query->append(_str);

    m_cursor.set_offset(ivec2_t(m_query->len + 1, 0));

}

//---------------------------------------------------------------------------------------
void CommandWindow::dispatchEvent()
{
    LOG_INFO("pressed enter. hiding window.");
    m_isWindowVisible = false;
}

//---------------------------------------------------------------------------------------
void CommandWindow::redraw()
{
    if (!m_isWindowVisible)
        return;

    // print query prefix
    api->printBufferLine(m_apiWindowPtr, m_queryPos.x, m_queryPos.y, m_query->str, m_query->len);
    
    // clear line and draw query
    api->clearSpace(m_apiWindowPtr, m_cursor.offset_x(), m_queryPos.y, m_frame.ncols - 1 - m_cursor.cx());
    api->printBufferLine(m_apiWindowPtr, m_cursor.offset_x(), m_queryPos.y, m_currentLine->content, m_currentLine->len);

    updateCursor();

}

