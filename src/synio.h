#ifndef __SYNIO_H
#define __SYNIO_H

// define backend
#include "core.h"
#include "types.h"

#include "utils/utils.h"
#include "window/window.h"
#include "window/buffer_window.h"
#include "window/command_window.h"
#include "line_buffer.h"
#include "buffer_formatter.h"
#include "cursor.h"
#include "events.h"
#include "config.h"

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
    Buffer *m_bufferWindow = NULL;      // TODO : to become an undordered map of windows?
    CommandWindow *m_commandWindow = NULL;

    Buffer *m_currentBuffer = NULL;     // indicating use of multiple buffers...

    //
    ivec2_t m_screenSize;

    //
    std::string m_filename = "";
    
};


// entry point and command line interface
int main(int argc, char *argv[]);







#endif // __SYNIO_H