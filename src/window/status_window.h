#ifndef __STATUS_WINDOW_H
#define __STATUS_WINDOW_H

#include "line_buffer_window.h"


//
class FileBufferWindow;
class StatusWindow : public LineBufferWindow
{
public:
    StatusWindow(const frame_t &_frame,
                 const std::string &_id,
                 int _wnd_params=false) : 
        LineBufferWindow(_frame, _id, _wnd_params)
    {}
    ~StatusWindow() = default;

    // 
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override {}

    virtual void update(FileBufferWindow *_buffer_window);

    // (Window overrides) -- for the StatusWindow, these should be done each frame
    virtual void resize(frame_t _new_frame) override;
    virtual void redraw() override;
    virtual void clear() override { api->clearWindow(m_apiWindowPtr); }
    virtual void refresh() override { api->refreshWindow(m_apiWindowPtr); }

    // accessors
    bool wasUpdated() { return m_wasUpdated; }


protected:
    bool m_wasUpdated = false;
    std::string m_filename = "";
    std::string m_filetype = "";
    int m_lineCount = 0;
    ivec2_t m_bpos = { 0 };

    std::string m_statusStr = "";
    
};



#endif // __STATUS_WINDOW_H
