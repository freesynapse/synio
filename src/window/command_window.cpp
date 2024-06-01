
#include "command_window.h"

#include <assert.h>

#include "../event_handler.h"
#include "../command.h"


//
CommandWindow::CommandWindow(const frame_t &_frame,
                             const std::string &_id,
                             FileBufferWindow *_buffer_ptr,
                             bool _border) :
    LineBufferWindow(_frame, _id, _border)
{
    m_currentBufferPtr = _buffer_ptr;
    
}

//---------------------------------------------------------------------------------------
CommandWindow::~CommandWindow()
{ 
    delete m_cmdPrefix;

}

//---------------------------------------------------------------------------------------
void CommandWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{
    if (_c == CTRL('x'))
    {
        EventHandler::push_event(new DeleteCommandWindowEvent());
        return;
    }


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
        // check for command
        if (Command::is_cmd_code(_c))
        {
            processCommandKeycode(_c);
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
                    if (m_currentCommand.id == CommandID::NONE)
                        processInput();
                    else
                        dispatchCommand();
                    moveCursor(0, 0);
                    break;

                case 9:     // <TAB>
                    tabComplete();
                    break;

                default:
                    enable_default_state_();
                    insertCharAtCursor((char)_c);
                    break;

            }
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
        if (it.first == CommandID::INVALID_COMMAND) continue;

        if (line.size() + it.second.id_str.size() + 2 > m_frame.ncols)
        {
            m_utilMLBuffer.push_back(line);
            line = it.second.id_str + "  ";
        }
        else
            line += (it.second.id_str + "  ");

    }
    m_utilMLBuffer.push_back(line);
    
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
void CommandWindow::redraw()
{
    if (!m_isWindowVisible)
        return;

    // print query prefix
    api->printString(m_apiWindowPtr, m_cmdPrefixPos.x, m_cmdPrefixPos.y, m_cmdPrefix->str, m_cmdPrefix->len);
    
    // clear line and draw query
    api->clearSpace(m_apiWindowPtr, m_cursor.offset_x(), m_cmdPrefixPos.y, m_frame.ncols - 1 - m_cursor.cx());
    api->printBufferLine(m_apiWindowPtr, m_cursor.offset_x(), m_cmdPrefixPos.y, m_currentLine->content, m_currentLine->len);

    if (m_showUtilBuffer)
        api->wprintml(m_apiWindowPtr, m_cursor.offset_x(), 0, m_utilMLBuffer);

    updateCursor();

}

//---------------------------------------------------------------------------------------
void CommandWindow::processCommandKeycode(int _c)
{
    if (m_currentLine->len != 0)
    {
        moveCursor(0, 0);
        return;
    }

    m_currentCommand = Command::cmd_from_key_code(_c);

    if (m_currentCommand.id == CommandID::SAVE_BUFFER && 
        FileIO::is_file_temp(m_currentBufferPtr->fileName()))
        m_currentCommand = Command::cmd(CommandID::SAVE_TEMP_BUFFER);

    if (m_currentCommand.id != CommandID::NONE) // redundant but still here!
        dispatchCommand();

}

//---------------------------------------------------------------------------------------
void CommandWindow::processInput()
{
    // if in raw input mode (ie not awaiting further input), enter that commands mode 
    // and clear input
    //if (m_currentCommand.id == CommandID::NONE)
    //{
    // find if input matches command
    const char *cmd = m_currentLine->__debug_str;
    for (auto &it : Command::s_commandMap)
    {
        if (strcmp(it.second.id_str.c_str(), cmd) == 0)
            // m_currentCommand = Command:: it.second.id;
            m_currentCommand = Command::cmd(it.second.id);
    }
    
    // was it set?
    if (Command::is_cmd(m_currentCommand.id))
    {
        if (m_currentCommand.id == CommandID::SAVE_BUFFER && 
            FileIO::is_file_temp(m_currentBufferPtr->fileName()))
            m_currentCommand = Command::cmd(CommandID::SAVE_TEMP_BUFFER);

        clear_input_();
        dispatchCommand();
        return;
    }
    else
        LOG_WARNING("invalid command '%s'.", m_currentLine->__debug_str);

    //
    clear_input_();
    //}

    // processing current command (eg save, open, search, etc)
    //else
    //{
    //    LOG_WARNING("input was supposed to go to '%s'!", Command::cmd2Str(m_currentCommand.id));
    //}

}

//---------------------------------------------------------------------------------------
void CommandWindow::dispatchCommand()
{
    if (!m_awaitNextInput)
    {
        switch (m_currentCommand.id)
        {
            //
            case CommandID::SAVE_BUFFER:
                if (m_currentBufferPtr != NULL) break;
                m_currentBufferPtr->writeBufferToFile();
                command_complete_();
                break;

            //---------------------------------------------------------------------------
            case CommandID::SAVE_TEMP_BUFFER:
            case CommandID::SAVE_BUFFER_AS:
            case CommandID::OPEN_BUFFER:
            case CommandID::EXIT_SAVE_YN:
            case CommandID::EXIT_NO_SAVE_YN:
                setQueryPrefix(m_currentCommand.command_prompt.c_str());
                await_next_input_();
                break;

            //---------------------------------------------------------------------------
            case CommandID::JUST_EXIT:
                EventHandler::push_event(new ExitEvent());
                break;
            
            //---------------------------------------------------------------------------
            default:
                break;
        }

    }

    else // command dispatched, but needs more input
    {
        switch (m_currentCommand.id)
        {
            //
            case CommandID::SAVE_TEMP_BUFFER:
                if (m_currentLine->len > 0)
                {
                    // delete entry from temp file list
                    FileIO::remove_temp_file(m_currentBufferPtr->fileName());
                    // save as
                    m_currentBufferPtr->writeBufferToFile(m_currentLine->__debug_str);
                    command_complete_();
                }
                else
                {
                    clear_input_();
                    await_next_input_();
                }
                break;

            //---------------------------------------------------------------------------
            case CommandID::SAVE_BUFFER_AS:
                if (m_currentLine->len > 0)
                {
                    int ret = m_currentBufferPtr->writeBufferToFile(m_currentLine->__debug_str);
                    command_complete_();
                }
                break;

            //---------------------------------------------------------------------------
            case CommandID::SAVE_ALL:
                // skip temp files -- but signal this to user
                // for buffer in buffers -> writeBufferToFile
                break;

            //---------------------------------------------------------------------------
            case CommandID::EXIT_SAVE_YN:
                // Save modified buffers, then exit. 
                // N.B.: This is the default exit option.
                //
                // for buffer : open_buffers:
                    // save?
                command_complete_();
                break;

            //---------------------------------------------------------------------------
            case CommandID::EXIT_NO_SAVE_YN:
                // exit without save on no input, 'y' or 'Y'
                if (diag_yes_())
                {
                    EventHandler::push_event(new ExitEvent());
                    command_complete_();
                }
                else if (diag_no_())
                    command_complete_();
                else
                {
                    setQueryPrefix(m_currentCommand.command_prompt.c_str());
                    clear_input_();
                }
                break;

            //---------------------------------------------------------------------------
            default:
                LOG_WARNING("command not handled: %s", Command::cmd2Str(m_currentCommand.id));
                command_complete_();
                break;
        }

        m_awaitNextInput = false;

    }


    // is command done? if so, close command window
    if (m_commandCompleted)
        EventHandler::push_event(new DeleteCommandWindowEvent);

}

