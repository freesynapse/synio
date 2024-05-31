
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
    delete m_cmdPrefix;

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
                tabComplete();
                break;

            case CTRL('q'):
                if (m_currentLine->len == 0)
                    EventHandler::push_event(new ExitEvent());
                else
                    moveCursor(0, 0);
                break;

            case CTRL('s'):
                setQueryPrefix("Save as (filename):");
                break;

            case CTRL('o'):
                setQueryPrefix("Open (filename):");
                break;

            default:
                enable_default_state_();
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

    delete m_cmdPrefix;

    m_cmdPrefix = new CHTYPE_STR(_prefix);
    assert(m_cmdPrefix->len < m_frame.ncols);

    m_cursor.set_offset(ivec2_t(m_cmdPrefix->len + 2, 0));

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void CommandWindow::appendPrefix(const char *_str)
{
    if (m_cmdPrefix->len == 0)
    {
        setQueryPrefix(_str);
        return;
    }

    // add an extra space
    m_cmdPrefix->append(" ");
    m_cmdPrefix->append(_str);

    m_cursor.set_offset(ivec2_t(m_cmdPrefix->len + 2, 0));

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void CommandWindow::tabComplete()
{
    m_utilMLBuffer.clear();

    size_t n = 0;
    std::string line = "";
    size_t current_line_width = m_cursor.offset_x() + line.size();
    int line_count = 0;

    //
    for (auto &it : Command::s_commandMap)
    {
        if (line.size() + it.second.id_str.size() > m_frame.ncols)
        {
            m_utilMLBuffer.push_back(line);
            line = it.second.id_str + " |";
        }
        else
            line += (" " + it.second.id_str + " |");

    }
    m_utilMLBuffer.push_back(line);

    for (auto &it : m_utilMLBuffer)
        LOG_ERROR("%s", it.c_str());
    
    int dy = -(m_utilMLBuffer.size() - m_frame.nrows);
    m_frame.v0.y += dy;
    m_frame.update_dims();
    resize(m_frame);
    
    clear_next_frame_();
    refresh_next_frame_();

    //
    show_util_buffer_next_frame_();

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

    // print query prefix
    api->printString(m_apiWindowPtr, m_cmdPrefixPos.x, m_cmdPrefixPos.y, m_cmdPrefix->str, m_cmdPrefix->len);
    
    // clear line and draw query
    api->clearSpace(m_apiWindowPtr, m_cursor.offset_x(), m_cmdPrefixPos.y, m_frame.ncols - 1 - m_cursor.cx());
    api->printBufferLine(m_apiWindowPtr, m_cursor.offset_x(), m_cmdPrefixPos.y, m_currentLine->content, m_currentLine->len);

    if (m_showTabCompletion)   // tab was pressed
        api->wprintml(m_apiWindowPtr, m_cursor.offset_x(), 0, m_utilMLBuffer);

    updateCursor();

}

