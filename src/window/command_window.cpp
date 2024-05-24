
#include "command_window.h"

#include <assert.h>

#include "../event_handler.h"
#include "../command.h"


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

            case 9:
                showCommands();
                break;

            case CTRL('q'):
                EventHandler::push_event(new ExitEvent());
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

    m_cursor.set_offset(ivec2_t(m_query->len + 2, 0));

    refresh_next_frame_();

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

    m_cursor.set_offset(ivec2_t(m_query->len + 2, 0));

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void CommandWindow::showCommands()
{
    m_showingHelp = true;

    memset(m_utilBuffer, 0, CMD_UTIL_BUF_SZ);

    size_t n = 0;
    size_t current_line_width = m_cursor.offset_x();
    int line_count = 0;

    //
    for (auto &it : Command::s_commandMap)
    {
        LOG_INFO("%d : [%s] '%s'", it.first, it.second.id_str.c_str(), it.second.command_str.c_str());
        if (current_line_width + it.second.id_str.size() > m_frame.v1.x)
        {
            // move this to platform -- e.g. wprintml(m_utilBuffer, &m_frame) (multi-line): returns the number of lines printed
            n += sprintf(m_utilBuffer + n, "\n");
            size_t new_line_width = sprintf(m_utilBuffer + n, " %s ", it.second.id_str.c_str());
            current_line_width = new_line_width;
            n += new_line_width;
            line_count++;
        }
        else
        {
            n += sprintf(m_utilBuffer + n, " %s ", it.second.id_str.c_str());
            current_line_width += n;
        }

    }

    int dy = -(line_count - m_frame.nrows);
    m_frame.v0.y += dy;
    resize(m_frame);
    clear_next_frame_();
    refresh_next_frame_();

    EventHandler::push_event(new AdjustBufferWindowEvent(dy));

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

    if (m_showingHelp)
    {
    }
    else
    {
        // print query prefix
        api->printBufferLine(m_apiWindowPtr, m_queryPos.x, m_queryPos.y, m_query->str, m_query->len);
        
        // clear line and draw query
        api->clearSpace(m_apiWindowPtr, m_cursor.offset_x(), m_queryPos.y, m_frame.ncols - 1 - m_cursor.cx());
        api->printBufferLine(m_apiWindowPtr, m_cursor.offset_x(), m_queryPos.y, m_currentLine->content, m_currentLine->len);

        updateCursor();
    }

}

