#ifndef __YES_NO_DIALOG_H
#define __YES_NO_DIALOG_H

#include "line_buffer_window.h"
#include "../cursor.h"
#include "../callbacks.h"

//
class OptionsWindow : public LineBufferWindow   // should maybe inherit Window, but cursor etc.
{
public:
    friend class Cursor;

public:
    // Window(const frame_t &_frame, const std::string &_id, int _wnd_params);
    OptionsWindow(const frame_t &_frame, 
                  const std::string &_id, 
                  int _wnd_params,
                  WindowCallback _callback,
                  const std::string &_header,
                  const std::string &_text,
                  const std::vector<std::string> &_options);

    // Window overrides
    virtual void redraw() override;
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;

    int prev_option_() { return ((m_selectedOption - 1) + m_options.size()) % m_options.size(); }
    int next_option_() { return (m_selectedOption + 1) % m_options.size(); }
    void move_cursor_to_selected_() { m_cursor.set_cpos(m_optOffsets[m_selectedOption]); }


private:
    std::vector<std::string> m_mlHeader;
    std::vector<std::string> m_mlText;
    
    std::vector<std::string> m_options;
    std::vector<ivec2_t> m_optOffsets;

    WindowCallback m_callback;

    int m_selectedOption = 0;

};


#endif // __YES_NO_DIALOG_H
