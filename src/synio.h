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
        if (!m_currentWindow) 
            return;
            
        m_currentWindow->updateCursor();
        m_currentWindow->clear();
        m_currentWindow->redraw();
        m_currentWindow->refresh();
    }

private:
    // flags
    bool m_shouldClose = false;
    bool m_commandMode = false;
    
    // windows
    FileBufferWindow *m_bufferWindow = NULL;      // TODO : to become an undordered map of windows?
    LineBufferWindow *m_commandWindow = NULL;

    BufferWindowBase *m_currentWindow = NULL;     // indicating use of multiple buffers...
    
    //
    ivec2_t m_screenSize;
    std::string m_filename = "";
    
};


#endif // __SYNIO_H
