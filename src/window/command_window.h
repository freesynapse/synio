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
    virtual void showCommands();

    // dispatch event based on what kind of window this is, called on <ENTER>
    virtual void dispatchEvent();

protected:
    //
    CHTYPE_STR_PTR m_query = NULL;
    ivec2_t m_queryPos = ivec2_t(1, 0);

    #define CMD_UTIL_BUF_SZ 4096
    char m_utilBuffer[CMD_UTIL_BUF_SZ];

    // state flags
    bool m_waitNextCommand = true;
    bool m_waitQuery = false;   // e.g. yes/no, filename etc
    bool m_showingHelp = false;

};




#endif // __COMMAND_WINDOW_H
