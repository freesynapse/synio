#ifndef __SYNIO_H
#define __SYNIO_H

#include "utils/utils.h"
#include "window/file_buffer_window.h"
#include "window/line_buffer_window.h"

//
class Synio
{
public:
    Synio(const std::string &_filename);
    ~Synio();

    //
    void initialize();

    //
    void mainLoop();


private:
    void clear_redraw_refresh_window_()
    {
        if (m_focusWindow == NULL) 
            return;
            
        m_focusWindow->clear_next_frame_();
        m_focusWindow->clear();
        m_focusWindow->redraw();
        m_focusWindow->updateCursor();
        m_focusWindow->refresh_next_frame_();
        m_focusWindow->refresh();
        api->redrawScreen();
    }

private:
    // flags
    bool m_shouldClose = false;
    bool m_commandMode = false;
    
    // windows
    FileBufferWindow *m_bufferWindow = NULL;    // TODO : to become an undordered map of windows, allowing multiple buffers?
    LineBufferWindow *m_commandWindow = NULL;
    LineBufferWindow *m_statusWindow = NULL;

    BufferWindowBase *m_focusWindow = NULL;     // status, command, dialog, buffer, etc
    
    //
    ivec2_t m_screenSize;
    std::string m_filename = "";
    
};


#endif // __SYNIO_H
