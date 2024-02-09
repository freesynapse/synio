
#include <assert.h>

#include "synio.h"
#include "utils/log.h"
#include "file_io.h"
#include "event_handler.h"
#include "core.h"

//
Synio::Synio(const char *_filename)
{
    Log::open();
    EventHandler::init();

    //
    set_backend();
    resize();

    // register callbacks
    EventHandler::register_callback(EventType::BUFFER_SCROLL, 
                                    EVENT_MEMBER_FNC(Synio::onBufferScroll));

    //
    mainLoop();

}

//---------------------------------------------------------------------------------------
Synio::~Synio()
{
    EventHandler::shutdown();
    Log::close();
    
    delete m_bufferWindow;

}

//---------------------------------------------------------------------------------------
void Synio::resize()
{
    api->getRenderSize(&m_screenSize);
    irect_t buffer_window_rect(ivec2_t(0, 0), ivec2_t(200, DEBUG_N_LINES));

    m_bufferWindow = new BufferWindow(&buffer_window_rect, "buffer_window");
    m_bufferWindow->readFromFile("synio.make");
    m_currentWindow = m_bufferWindow;

}

//---------------------------------------------------------------------------------------
void Synio::onBufferScroll(Event *_e)
{
    BufferScrollEvent *e = dynamic_cast<BufferScrollEvent*>(_e);
    
    //LOG_INFO("%s: scroll recieved from window %s: axis=%s, dir=%d, steps=%d\n", 
    //        __func__, 
    //        e->windowPtr()->ID().c_str(),
    //        e->axis() == X_AXIS ? "X" : "Y",
    //        e->dir(),
    //        e->steps());

    dynamic_cast<BufferWindow*>(m_currentWindow)->onScroll(e);

}

//---------------------------------------------------------------------------------------
void Synio::mainLoop()
{
    while (!m_shouldClose)
    {
        // api->clearScreen();
        // m_formatter.render(&m_lineBuffer, m_pageFirstLine, m_pageLastLine);

        m_currentWindow->clear();    // very good, clear() clears the borders...
        // ---> https://stackoverflow.com/questions/33986047/ncurses-is-it-possible-to-refresh-a-window-without-removing-its-borders
        
        // --- BEGIN DRAWING
        m_currentWindow->draw();
        m_cursor.update(m_currentWindow);

        // --- END DRAWING

        m_currentWindow->refresh();

        // #ifdef DEBUG
        #if 0
        char b[128];
        memset(b, 0, 128);
        snprintf(b, 128, "cursor: %d, %d", m_cursor.pos().x,
                                           m_cursor.pos().y);
        api->printBufferLine(api->screenPtr(), 0, 50, b);
        #endif

        int key = api->getKey();
        switch(key)
        {
            // movement
            case KEY_DOWN:  m_cursor.move(m_currentWindow, 0, 1);   break;
            case KEY_UP:    m_cursor.move(m_currentWindow, 0, -1);  break;
            case KEY_LEFT:  m_cursor.move(m_currentWindow, -1, 0);  break;
            case KEY_RIGHT: m_cursor.move(m_currentWindow, 1, 0);   break;
            case KEY_PPAGE: m_cursor.move(m_currentWindow, 0, -Config::PAGE_SIZE);  break;
            case KEY_NPAGE: m_cursor.move(m_currentWindow, 0, Config::PAGE_SIZE);   break;
            
            // command control
            case CTRL('x'):
                m_shouldClose = true;
                break;

        }

        // api->refreshScreen();
        m_currentWindow->refresh();

        EventHandler::process_events();

    }

}

//=======================================================================================
int main(int argc, char *argv[])
{
    const char *filename = "synio.make";
    Synio synio(filename);

    return 0;

}

