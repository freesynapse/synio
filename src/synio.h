#ifndef __SYNIO_H
#define __SYNIO_H

// define backend
#define NCURSES_IMPL
#include "core.h"
#include "types.h"

#include "utils/utils.h"
#include "window.h"
#include "line_buffer.h"
#include "buffer_formatter.h"
#include "cursor.h"
#include "events.h"
#include "config.h"

//
class Synio
{
public:
    Synio(const char *_filename);
    ~Synio();

    //
    void resize();
    void onBufferScroll(Event *_e);

    //
    void mainLoop();



private:
    bool m_shouldClose = false;
    
    Window *m_mainWindow = NULL;    // to become an undordered map of windows
    Window *m_currentWindow = NULL;

    LineBuffer m_lineBuffer;    // put into separate class and window -- for now only here
                                // also, could make it platform independent, i.e. provide 
                                // functions for windows, rendering etc for eg curses, GLFW
                                // SDL2 etc.
    BufferFormatter m_formatter;
    
    Cursor m_cursor;
    line_t *m_currentLine   = NULL;
    line_t *m_pageFirstLine = NULL;
    line_t *m_pageLastLine  = NULL;
    
    ivec2_t m_screenSize;

};


// entry point and command line interface
int main(int argc, char *argv[]);







#endif // __SYNIO_H