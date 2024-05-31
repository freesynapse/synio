#ifndef __COMMAND_WINDOW_H
#define __COMMAND_WINDOW_H

#include "line_buffer_window.h"

//
class CommandWindow : public LineBufferWindow
{
public:
    CommandWindow(const frame_t &_frame,
                  const std::string &_id,
                  bool _border=false);
    ~CommandWindow();


    // LineBufferWindow overrides
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;
    virtual void redraw() override;

    // enter prefix (filename input, command etc, before user input)
    virtual void setQueryPrefix(const char *_prefix="");
    virtual void appendPrefix(const char *_str);

    //
    virtual void tabComplete();

    // dispatch event based on what kind of window this is, called on <ENTER>
    virtual void dispatchEvent();

protected:
    __always_inline virtual void show_util_buffer_next_frame_() { m_showTabCompletion = true; }
    __always_inline virtual void enable_default_state_()
    {
        // reset all flags, goto default 'enter query' mode
        m_showTabCompletion = false;
    }


protected:
    //
    CHTYPE_STR_PTR m_cmdPrefix = NULL;  // actually what's printed before the input
    ivec2_t m_cmdPrefixPos = ivec2_t(1, 0);

    std::vector<std::string> m_utilMLBuffer;

    // state flags
    bool m_waitNextCommand = true;
    bool m_waitQuery = false;   // e.g. yes/no, filename etc
    // bool m_showingHelp = false;
    bool m_showTabCompletion = false;

};




#endif // __COMMAND_WINDOW_H
