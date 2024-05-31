#ifndef __COMMAND_WINDOW_H
#define __COMMAND_WINDOW_H

#include "line_buffer_window.h"
#include "file_buffer_window.h"
#include "../command.h"

//
class CommandWindow : public LineBufferWindow
{
public:
    CommandWindow(const frame_t &_frame,
                  const std::string &_id,
                  FileBufferWindow *_buffer_ptr,
                  bool _border=false);
    ~CommandWindow();

    // LineBufferWindow overrides
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;
    virtual void redraw() override;

    // set prefix (filename input, command etc, before user input)
    virtual void setQueryPrefix(const char *_prefix="");

    // show available options
    // TODO : implement tree as in tsodings video for autocomplete
    virtual void tabComplete();

    // dispatch events based on entered command
    virtual void processCommandKeycode(int _c);
    virtual void processInput();
    virtual void dispatchCommand();

    // accessors
    virtual void setCurrentBufferPtr(FileBufferWindow *_ptr) { m_currentBufferPtr = _ptr; }
    virtual FileBufferWindow *currentBufferPtr() { return m_currentBufferPtr; }


protected:
    __always_inline virtual void clear_input_()
    {
        moveCursor(-m_currentLine->len, 0);
        delete m_currentLine;
        m_currentLine = create_line("");
    }
    __always_inline virtual void show_util_buffer_next_frame_() { m_showUtilBuffer = true; }
    __always_inline virtual void enable_default_state_()
    {
        // reset all flags, goto default 'input' mode
        m_showUtilBuffer = false;
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
    //
    FileBufferWindow *m_currentBufferPtr = NULL;

    CHTYPE_STR_PTR m_cmdPrefix = NULL;  // actually what's printed before the input
    ivec2_t m_cmdPrefixPos = ivec2_t(1, 0);
    command_t m_currentCommand;

    std::vector<std::string> m_utilMLBuffer;

    // state flags
    bool m_showUtilBuffer = false;
    bool m_awaitNextInput = false; // e.g. yes/no, filename etc
    bool m_commandCompleted = false;

};




#endif // __COMMAND_WINDOW_H
