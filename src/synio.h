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
    // flags
    bool m_shouldClose = false;
    bool m_commandMode = false;
    
    // windows
    FileBufferWindow *m_bufferWindow = NULL;      // TODO : to become an undordered map of windows?

    BufferWindowBase *m_currentWindow = NULL;     // indicating use of multiple buffers...
    LineBufferWindow *m_dialog = NULL;
    

    //
    ivec2_t m_screenSize;

    //
    std::string m_filename = "";
    
};


#endif // __SYNIO_H
