
#include <assert.h>

#include "synio.h"
#include "utils/log.h"
#include "file_io.h"
#include "event_handler.h"
#include "core.h"

//
Synio::Synio(const char *_filename)
{
    EventHandler::init();

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

    delete m_bufferWindow;

}

//---------------------------------------------------------------------------------------
void Synio::resize()
{
    api->getRenderSize(&m_screenSize);
    irect_t buffer_window_rect(ivec2_t(0), ivec2_t(m_screenSize.x, m_screenSize.y));

    m_bufferWindow = new Buffer(&buffer_window_rect, "buffer_window", false);
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

    dynamic_cast<Buffer*>(m_currentWindow)->onScroll(e);

}

//---------------------------------------------------------------------------------------
void Synio::mainLoop()
{
    while (!m_shouldClose)
    {
        m_currentWindow->clear();    // very good, clear() clears the borders...
        // ---> https://stackoverflow.com/questions/33986047/ncurses-is-it-possible-to-refresh-a-window-without-removing-its-borders
        // -- Changed in the Window class so that all extra 'border' windows can be drawn.
        
        // --- BEGIN DRAWING
        m_currentWindow->draw();
        m_currentWindow->updateCursor();
        m_currentWindow->refresh();

        // --- END DRAWING

        //
        int key = api->getKey();

        // -- DEBUG
        const char *s = "-----";

        if (m_commandMode)
        {

        }
        // interactive mode
        else
        {
            switch(key)
            {
                // cursor movement
                case KEY_DOWN:  m_currentWindow->moveCursor(0, 1);   break;
                case KEY_UP:    m_currentWindow->moveCursor(0, -1);  break;
                case KEY_LEFT:  m_currentWindow->moveCursor(-1, 0);  break;
                case KEY_RIGHT: m_currentWindow->moveCursor(1, 0);   break;
                case KEY_PPAGE: m_currentWindow->moveCursor(0, -Config::PAGE_SIZE);  break;
                case KEY_NPAGE: m_currentWindow->moveCursor(0, Config::PAGE_SIZE);   break;
                case KEY_HOME:  m_currentWindow->moveCursorToLineBegin(); break;
                case KEY_END:   m_currentWindow->moveCursorToLineEnd(); break;

                // command control
                case CTRL('x'):
                    m_shouldClose = true;
                    break;

                // test insert string in line
                case CTRL('v'):
                    m_currentWindow->insertStrAtCursor((char *)s, strlen(s));
                    break;

                case KEY_DC:
                    m_currentWindow->deleteCharAtCursor();
                    break;

                case KEY_BACKSPACE:
                    m_currentWindow->deleteCharBeforeCursor();
                    break;

                default:
                    m_currentWindow->insertCharAtCursor((char)key);
                    break;

            }

        }

        m_currentWindow->updateCursor();
        m_currentWindow->refresh();

        EventHandler::process_events();

    }

}

//=======================================================================================
int main(int argc, char *argv[])
{
    const char *filename = "test.make";

    //
    Log::open();

    set_backend();
    api->initialize();

    {
        Synio synio(filename);
    }

    api->shutdown();

    Log::close();

    return 0;

}

