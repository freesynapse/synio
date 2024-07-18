#ifndef __COMMAND_WINDOW_H
#define __COMMAND_WINDOW_H

#include <filesystem>

#include "line_buffer_window.h"
#include "file_buffer_window.h"
#include "file_explorer_window.h"
#include "listbox_window.h"
#include "options_window.h"
#include "../command.h"

//
class Synio;
class CommandWindow : public LineBufferWindow
{
public:
    CommandWindow(const frame_t &_frame,
                  const std::string &_id,
                  Synio *_app_ptr,
                  int _wnd_params=false);
    ~CommandWindow();

    // LineBufferWindow overrides
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;
    virtual void redraw() override;
    virtual void clear() override;
    virtual void refresh() override;

    // set prefix (filename input, command etc, before user input)
    virtual void setQueryPrefix(const char *_prefix="");

    // show available options
    virtual void findCompletions();
    virtual void autocompleteInput();
    virtual void adjustFrame(int _d);

    // child window handling
    void openFileExplorerWindow(const std::string &_input_prompt);
    void callbackFileExplorerWindow(std::string _selected_file);
    //
    void openOptionsDialog(const std::string &_header, 
                           const std::string &_text,
                           const std::vector<std::string> &_options);
    void callbackOptionsDialog(std::string _selected_option);
    //
    void openListboxWindow(const std::string &_header, 
                           std::vector<listbox_entry_t> _entries);
    void callbackListboxWindow(std::string _selected_entry);

    // dispatch events based on entered command
    virtual void processCommandKeycode(int _c);
    virtual void processInput();
    virtual void dispatchCommand();

    virtual void __process_commands();

    ///// DEBUG /////
    void debugCommand();


protected:
    __always_inline virtual void clear_input_()
    {
        moveCursor(-m_currentLine->len, 0);
        delete m_currentLine;
        m_currentLine = create_line("");
    }
    __always_inline virtual void await_next_input_() { m_awaitNextInput = true; }
    __always_inline virtual void command_complete_() { m_commandCompleted = true; }
    __always_inline virtual bool diag_yes_()
    {
        return (m_currentLine->len == 0 ||
                strcmp(m_currentLine->__debug_str, "y") == 0 ||
                strcmp(m_currentLine->__debug_str, "Y") == 0);
    }
    __always_inline virtual bool diag_no_()
    {
        return (strcmp(m_currentLine->__debug_str, "n") == 0 ||
                strcmp(m_currentLine->__debug_str, "N") == 0);
    }
   

protected:
    // command structure
    std::stack<command_t> m_commands;
    command_t m_currentCommand;

    // child windows
    FileExplorerWindow *m_fileExplorerWndPtr = NULL;
    ListboxWindow *m_listboxWndPtr = NULL;
    OptionsWindow *m_optionsWndPtr = NULL;
    
    std::filesystem::path m_selectedFile = "";  // result of FileExploreWindow
    std::string m_selectedListBoxEntry = "";    // result of ListboxWindow
    std::string m_optionsResult = "";           // result of OptionsWindow
    
    // state flags
    bool m_awaitNextInput = false;      // e.g. yes/no, filename etc
    bool m_commandCompleted = false;
    bool m_awaitNextCommand = false;    // if a command is intercepted by another command

    // autocomplete-related
    prefix_node_t *m_ptree = NULL;
    std::vector<std::string> m_autocompletions;
    size_t m_selectedCompletion = 0;
    std::vector<std::string> m_utilMLBuffer;
    
    // misc
    Synio *m_app = NULL;
    CHTYPE_STR_PTR m_cmdPrefix = NULL;  // actually what's printed before the input
    ivec2_t m_cmdPrefixPos = ivec2_t(1, 0);
    
};




#endif // __COMMAND_WINDOW_H
