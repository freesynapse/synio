
#include "command_window.h"

#include <assert.h>

#include "../synio.h"
#include "../event_handler.h"
#include "../command.h"
#include "../callbacks.h"

//
CommandWindow::CommandWindow(const frame_t &_frame,
                             const std::string &_id,
                            //  FileBufferWindow *_buffer_ptr,
                             Synio *_app_ptr,
                             int _wnd_params) :
    LineBufferWindow(_frame, _id, _wnd_params)
{
    m_app = _app_ptr;
    
}

//---------------------------------------------------------------------------------------
CommandWindow::~CommandWindow()
{ 
    delete m_cmdPrefix;
    delete m_listboxWndPtr;
    delete m_fileExplorerWndPtr;
    delete m_optionsWndPtr;
    
}

//---------------------------------------------------------------------------------------
void CommandWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{
    if (_c == CTRL('x'))
    {
        EventHandler::push_event(new CloseCommandWindowEvent());
        return;
    }

    // TODO :   these statements could be replaced by shifting focus to the child window
    //          in a future WindowManager class. For now, let's handle it like this.
    // if there is an open listbox, redirect input
    if (m_listboxWndPtr != NULL)
    {
        m_listboxWndPtr->handleInput(_c, _ctrl_action);
        return;
    }
    // redirect input to open file explorer window
    else if (m_fileExplorerWndPtr != NULL)
    {
        m_fileExplorerWndPtr->handleInput(_c, _ctrl_action);
        return;
    }
    // redirect input to open options window
    else if (m_optionsWndPtr != NULL)
    {
        m_optionsWndPtr->handleInput(_c, _ctrl_action);
        return;
    }

    // special keys
    if (_ctrl_action != CtrlKeyAction::NONE)
    {
        switch (_ctrl_action)
        {
            case CtrlKeyAction::CTRL_LEFT:  findColDelim(-1);   break;
            case CtrlKeyAction::CTRL_RIGHT: findColDelim(1);    break;
            case CtrlKeyAction::CTRL_UP:    moveCursorToRowDelim(-1);   break;
            case CtrlKeyAction::CTRL_DOWN:  moveCursorToRowDelim(1);    break;
            case CtrlKeyAction::CTRL_DEL:   deleteToNextColDelim();     break;
            // default: LOG_WARNING("ctrl keycode %d : %s", _c, ctrlActionStr(_ctrl_action)); break;
            default: break;
        }
    }
    //
    else
    {
        // check for command keyboard shortcut
        if (Command::is_cmd_code(_c))
            processCommandKeycode(_c);

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

                case 8:     // <Ctrl> + <Backspace>
                    deleteToPrevColDelim();
                    break;

                case 9:     // <TAB>
                    tabComplete();
                    moveCursor(0, 0);
                    break;

                case 10:    // <ENTER>
                    if (m_currentCommand.id == CommandID::NONE)
                        processInput();
                    else
                        dispatchCommand();
                    // moveCursor(0, 0);
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
    int line_count = 0;

    // Check use case
    //
    
    // All file I/O commands should list the current dir on <TAB> -- should we use some
    // sort of dialog here?
    if (Command::is_cmd_fileio_(m_currentCommand.id))
    {

    }

    // show autocompletion for entered characters
    else
    {
        static prefix_node_t *ptree = Command::s_cmdPTree;
        std::string sstr(m_currentLine->__debug_str);
        
        //
        prefix_node_t *stree = PrefixTree::find_subtree(ptree, sstr);

        // string not found in tree, no completions
        if (stree == nullptr)
        {
            m_utilMLBuffer.push_back("(no autocompletions found)");
            show_util_buffer_next_frame_();
            clear_next_frame_();
            refresh_next_frame_();
            return;
        }
        
        // find longest common prefix, if this exists, we auto-insert that to the search
        // string
        std::string longest_prefix = "";
        PrefixTree::find_longest_prefix(stree, sstr, &longest_prefix);

        // if the longest prefix is longer than the search string, the longest prefix
        // is inserted.
        if (longest_prefix.length() != sstr.length())
        {
            delete m_currentLine;
            m_currentLine = create_line(longest_prefix.c_str());
            moveCursor(longest_prefix.length() - sstr.length(), 0);
        }
        // longest prefix is the same as input; show possible autocompletions
        else
        {
            // accumulate autocompletions for this 
            std::vector<std::string> autocomps;
            PrefixTree::find_completions(stree, &autocomps, sstr);

            for (auto &ac : autocomps)
            {
               if (line.length() + ac.length() + 2 > m_frame.ncols)
               {
                   m_utilMLBuffer.push_back(line);
                   line = ac + "  ";
               }
               else
                   line += (ac + "  ");
            }
            m_utilMLBuffer.push_back(line);

            show_util_buffer_next_frame_();

            int dy = -(m_utilMLBuffer.size() - m_frame.nrows);
            if (dy != 0)
            {
                m_frame.v0.y += dy;
                m_frame.update_dims();
                resize(m_frame);
                EventHandler::push_event(new AdjustBufferWindowEvent(dy));
            }
        }

    }

    clear_next_frame_();
    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void CommandWindow::openFileExplorerWindow(const std::string &_input_prompt)
{
    ivec2_t ssize;
    api->getRenderSize(&ssize);
    float p = Config::FILE_DIALOG_SIZE_PERCENT; 
    float p2 = (1.0f - p) * 0.5f;
    int w0 = p2 * ssize.x;
    int h0 = p2 * ssize.y;

    static frame_t explorer_wnd_frame = frame_t(ivec2_t(w0, h0), 
                                                ivec2_t(ssize.x - w0, ssize.y - h0));
    delete m_fileExplorerWndPtr;
    m_fileExplorerWndPtr = new FileExplorerWindow(explorer_wnd_frame, 
                                                  "cmp_file_explorer_window",
                                                  true,
                                                  SYNIO_MEMBER_FNC1(CommandWindow::callbackFileExplorerWindow),
                                                  _input_prompt);

}

//---------------------------------------------------------------------------------------
void CommandWindow::callbackFileExplorerWindow(std::string _selected_file)
{
    m_selectedFile = std::filesystem::path(_selected_file);

    delete m_fileExplorerWndPtr;
    m_fileExplorerWndPtr = NULL;
    refresh_next_frame_();
    m_app->refreshBufferWindow();

    dispatchCommand();

}

//---------------------------------------------------------------------------------------
void CommandWindow::openOptionsDialog(const std::string &_header, 
                                      const std::string &_text,
                                      const std::vector<std::string> &_options)
{
    ivec2_t ssize;
    api->getRenderSize(&ssize);
    // height is irrelevant, since the OptionsWindow auto-resized to fit contents
    ivec2_t v0 = { (ssize.x / 2) - (Config::OPTIONS_DIALOG_DEFAULT_WIDTH / 2), 1 };
    ivec2_t v1 = { (ssize.x / 2) + (Config::OPTIONS_DIALOG_DEFAULT_WIDTH / 2), 5 };
    frame_t options_frame = { v0, v1 };
    
    delete m_optionsWndPtr;
    m_optionsWndPtr = new OptionsWindow(options_frame, 
                                        "cmd_options_wnd", 
                                        WPARAMS_BORDER,
                                        SYNIO_MEMBER_FNC1(CommandWindow::callbackOptionsDialog),
                                        _header,
                                        _text,
                                        _options);

}

//---------------------------------------------------------------------------------------
void CommandWindow::callbackOptionsDialog(std::string _selected_option)
{
    m_optionsResult = _selected_option;

    delete m_optionsWndPtr;
    m_optionsWndPtr = NULL;
    refresh_next_frame_();
    m_app->refreshBufferWindow();

    dispatchCommand();

}

//---------------------------------------------------------------------------------------
void CommandWindow::openListboxWindow(const std::string &_header,
                                      std::vector<listbox_entry_t> _entries)
{
    // window starts at x=1 and y=(no entry rows up from buffer window border) and 
    // extends down.
    auto buffer_frame = m_app->bufferWndFrame();
    int start_row = buffer_frame.v1.y - _entries.size() - 4;
    ivec2_t v0 = ivec2_t(1, start_row); 
    // make small, the window will resize to fit entries in it's constructor
    ivec2_t v1 = ivec2_t(10, buffer_frame.v1.y - 1);

    frame_t listbox_frame = frame_t(v0, v1);

    delete m_listboxWndPtr;
    m_listboxWndPtr = new ListboxWindow(listbox_frame, 
                                        "cmd_listbox_window",
                                        true,
                                        SYNIO_MEMBER_FNC1(CommandWindow::callbackListboxWindow),
                                        _header,
                                        _entries);

}

//---------------------------------------------------------------------------------------
void CommandWindow::callbackListboxWindow(std::string _selected_entry)
{
    m_selectedListBoxEntry = _selected_entry;

    delete m_listboxWndPtr;
    m_listboxWndPtr = NULL;
    refresh_next_frame_();
    m_app->refreshBufferWindow();

    dispatchCommand();

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
    {
        api->enableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_INACTIVE));
        api->wprintml(m_apiWindowPtr, m_cursor.offset_x() + m_currentLine->len + 2, 0, m_utilMLBuffer);
        api->disableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_INACTIVE));
    }

    if (m_listboxWndPtr || m_fileExplorerWndPtr || m_optionsWndPtr)
    {
        if (m_listboxWndPtr != NULL)
            m_listboxWndPtr->redraw();
        
        if (m_fileExplorerWndPtr != NULL)
            m_fileExplorerWndPtr->redraw();
        
        if (m_optionsWndPtr != NULL)
            m_optionsWndPtr->redraw();
    }
    else
        updateCursor();


}

//---------------------------------------------------------------------------------------
void CommandWindow::clear()
{
    if (m_listboxWndPtr != NULL)
        m_listboxWndPtr->clear();

    if (m_fileExplorerWndPtr != NULL)
        m_fileExplorerWndPtr->clear();

    if (m_optionsWndPtr != NULL)
        m_optionsWndPtr->clear();

    //
    if (m_clearNextFrame)
    {
        api->clearWindow(m_apiWindowPtr);
        m_clearNextFrame = false;
    }
    
};

//---------------------------------------------------------------------------------------
void CommandWindow::refresh()
{
    if (m_listboxWndPtr != NULL)
        m_listboxWndPtr->refresh();

    if (m_fileExplorerWndPtr != NULL)
        m_fileExplorerWndPtr->refresh();

    if (m_optionsWndPtr != NULL)
        m_optionsWndPtr->refresh();
        
    //
    if (m_refreshNextFrame)
    {
        api->refreshWindow(m_apiWindowPtr);
        m_refreshNextFrame = false;
    }

};

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
        FileIO::is_file_temp(m_app->currentBufferWindow()->fileName()))
        m_currentCommand = Command::cmd(CommandID::SAVE_TEMP_BUFFER);

    dispatchCommand();

}

//---------------------------------------------------------------------------------------
void CommandWindow::processInput()
{
    // find if input matches command
    const char *cmd = m_currentLine->__debug_str;
    for (auto &it : Command::s_commandMap)
    {
        if (strcmp(it.second.id_str.c_str(), cmd) == 0)
            m_currentCommand = Command::cmd(it.second.id);
    }
    
    // was it set?
    if (Command::is_cmd(m_currentCommand.id))
    {
        if (m_currentCommand.id == CommandID::SAVE_BUFFER && 
            FileIO::is_file_temp(m_app->currentBufferWindow()->fileName()))
            m_currentCommand = Command::cmd(CommandID::SAVE_TEMP_BUFFER);

        clear_input_(); // <--- this prevents the cursor to move to a new window..?
                        //      (it was the refresh_next_frame_() called by moveCursor),
                        //      but fixed through refresh_next_frame_() call by this on 
                        //      return from FileExplorerWindow.
        dispatchCommand();
        return;
    }
    else
        LOG_WARNING("invalid command '%s'.", m_currentLine->__debug_str);

    //
    clear_input_();

}

//---------------------------------------------------------------------------------------
void CommandWindow::dispatchCommand()
{ 
    static std::vector<listbox_entry_t> listbox_entries;

    if (!m_awaitNextInput)
    {
        switch (m_currentCommand.id)
        {
            //
            #ifdef DEBUG
            case CommandID::DEBUG_COMMAND:
                debugCommand();
                await_next_input_();
                break;
            #endif
            
            //---------------------------------------------------------------------------
            case CommandID::SAVE_BUFFER:
                if (m_app->currentBufferWindow() == NULL) break;
                m_app->currentBufferWindow()->writeBufferToFile();
                command_complete_();
                break;

            //---------------------------------------------------------------------------
            case CommandID::SWITCH_TO_BUFFER:
                listbox_entries.clear();
                for (auto &it : m_app->openBufferWindows())
                {
                    listbox_entries.push_back(listbox_entry_t(it.path, it.path.filename()));
                    LOG_INFO("%s", it.path.c_str());
                }
                openListboxWindow("Switch to buffer", listbox_entries);
                await_next_input_();                            
                break;
                
            //---------------------------------------------------------------------------
            // Opens a FileExplorerWindow
            case CommandID::SAVE_TEMP_BUFFER:
            case CommandID::SAVE_BUFFER_AS:
            case CommandID::OPEN_BUFFER:
                openFileExplorerWindow(m_currentCommand.command_prompt);
                await_next_input_();
                break;

            //---------------------------------------------------------------------------
            case CommandID::CLOSE_THIS_BUFFER:
                // ask for save before save
                if (m_app->currentBufferWindow()->bufferChanged())
                {
                    // ask for save
                    // openOptionsDialog("Save buffer '" + m_app->currentFilename() + "' before closing?");
                    await_next_input_();
                }
                else
                {
                    EventHandler::push_event(new CloseFileBufferEvent(true));
                    command_complete_();
                }
                
                    // m_app->closeFileBufferWindow()
                break;

            //---------------------------------------------------------------------------
            // Simple query for filename
            case CommandID::NEW_BUFFER:
                setQueryPrefix(m_currentCommand.command_prompt.c_str());
                await_next_input_();
                break;

            //---------------------------------------------------------------------------
            // Opens Yes/No dialog
            case CommandID::EXIT_SAVE_YN:
            case CommandID::EXIT_NO_SAVE_YN:
                // setQueryPrefix(m_currentCommand.command_prompt.c_str());
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

    else // command dispatched, this is now awaiting second signal
    {
        switch (m_currentCommand.id)
        {
            
            //
            case CommandID::SWITCH_TO_BUFFER:
                // switch active window to the selected
                m_app->switchToBuffer(std::filesystem::path(m_selectedListBoxEntry));
                command_complete_();
                break;
                
            //---------------------------------------------------------------------------
            case CommandID::SAVE_TEMP_BUFFER:
                if (m_currentLine->len > 0)
                {
                    // delete entry from temp file list
                    FileIO::remove_temp_file(m_app->currentBufferWindow()->fileName());
                    // save as
                    m_app->currentBufferWindow()->writeBufferToFile(m_currentLine->__debug_str);
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
                    int ret = m_app->currentBufferWindow()->writeBufferToFile(m_currentLine->__debug_str);
                    command_complete_();
                }
                break;

            //---------------------------------------------------------------------------
            case CommandID::SAVE_ALL:
                // skip temp files(?) -- but signal this to user
                // for buffer in buffers -> writeBufferToFile
                command_complete_();
                break;

            //---------------------------------------------------------------------------
            case CommandID::OPEN_BUFFER:
                if (FileIO::does_file_exists(m_selectedFile))
                {
                    FileBufferWindow *w = m_app->openFileBufferWindow(m_selectedFile);
                    m_app->setCurrentBufferWindow(w);
                }
                else
                {
                    LOG_ERROR("File '%s' does not exist.", m_selectedFile.c_str());
                }
                command_complete_();

                break;

            //---------------------------------------------------------------------------
            case CommandID::CLOSE_THIS_BUFFER:
                // reached here if a YesNo dialog was issued due to unsaved buffer
                if (m_optionsResult == "Yes")
                {
                    if (FileIO::is_file_temp(m_app->currentFilename()))
                    {
                        LOG_ERROR("CLOSE_THIS_BUFFER not yet implemented for temp buffers.");
                    }
                    else
                    {
                        m_app->currentBufferWindow()->writeBufferToFile();
                        EventHandler::push_event(new CloseFileBufferEvent(true));
                        command_complete_();
                    }
                }
                break;

            //---------------------------------------------------------------------------
            case CommandID::NEW_BUFFER:
                // default filename is the next temp filename (based on the number of temp files)
                command_complete_();
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
        EventHandler::push_event(new CloseCommandWindowEvent);

}

//---------------------------------------------------------------------------------------
void CommandWindow::debugCommand()
{
    openOptionsDialog("DEBUG1 DEBUG2 DEBUG3 DEBUG4 DEBUG5 DEBUG6 DEBUG7 DEBUG8 DEBUG9", "test1 texttest2 texttest3 texttest4 texttest5 texttest6 texttest7 text8", {"Yes", "No", "Cancel"});

}
