#ifndef __SYNIO_H
#define __SYNIO_H

// define backend
#define NCURSES_IMPL
#include "core.h"
#include "types.h"

#include "utils/utils.h"
#include "window/window.h"
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
    void resize();
    void onBufferScroll(Event *_e);

    //
    void mainLoop();



private:
    // flags
    bool m_shouldClose = false;
    bool m_commandMode = false;
    
    // windows
    Buffer *m_bufferWindow = NULL;    // to become an undordered map of windows?
    LineNumbers *m_lineNumbersWindow = NULL;
    Buffer *m_currentBuffer = NULL;

    //
    ivec2_t m_screenSize;

    // TODO : remove and read cl args properly
    std::string m_filename = "";
    
};


// entry point and command line interface
int main(int argc, char *argv[]);







#endif // __SYNIO_H