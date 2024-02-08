
#include <stdio.h>

#include "synio.h"
#include "utils/log.h"
#include "file_io.h"
#include "event_handler.h"

#define DEBUG_N_LINES 40

//
Synio::Synio(const char *_filename)
{
    Log::open();
    EventHandler::init();

    FileIO::readFileIntoBuffer(_filename, &m_lineBuffer);

    //
    set_backend();
    resize();

    //
    m_formatter = BufferFormatter(m_mainWindow->frame());
    
    // line pointers
    m_currentLine = m_lineBuffer.m_head;
    m_pageFirstLine = m_lineBuffer.m_head;
    m_pageLastLine = m_lineBuffer.ptrFromIdx(DEBUG_N_LINES+1);

    // register callbacks
    EventHandler::register_callback(EventType::BUFFER_SCROLL, EVENT_MEMBER_FNC(Synio::onBufferScroll));

    //
    mainLoop();

}

//---------------------------------------------------------------------------------------
Synio::~Synio()
{
    EventHandler::shutdown();
    Log::close();
    
    delete m_mainWindow;

}

//---------------------------------------------------------------------------------------
void Synio::resize()
{
    api->getRenderSize(&m_screenSize);
    irect_t screen_rect = { 
        .v0 = { 0, 1 },
        // .v1 = { m_screenSize.x, m_screenSize.y }
        .v1 = { 200, DEBUG_N_LINES-1 }
    };

    m_mainWindow = new Window(screen_rect, "main window");
    m_currentWindow = m_mainWindow;

}

//---------------------------------------------------------------------------------------
void Synio::onBufferScroll(Event *_e)
{
    BufferScrollEvent *e = dynamic_cast<BufferScrollEvent*>(_e);
    
    //LOG_INFO("%s: scroll recieved from window %s: axis=%s, dir=%d, steps=%d\n", 
    //         __func__, 
    //         e->windowPtr()->ID().c_str(),
    //         e->axis() == X_AXIS ? "X" : "Y",
    //         e->dir(),
    //         e->steps());
    
    // adjust pointers
    int n = 0;
    // scroll up
    if (e->dir() < 0)
    {
        while (n < e->steps() && m_pageFirstLine->prev != NULL)
        {
            m_pageFirstLine = m_pageFirstLine->prev;
            m_pageLastLine = m_pageLastLine->prev;
            n++;
        }

    }
    // scroll down
    else
    {
        while (n < e->steps() && m_pageLastLine->next != NULL)
        {
            m_pageFirstLine = m_pageFirstLine->next;
            m_pageLastLine = m_pageLastLine->next;
            n++;
        }

    }



}

//---------------------------------------------------------------------------------------
void Synio::mainLoop()
{
    while (!m_shouldClose)
    {
        api->clearScreen();

        m_formatter.render(&m_lineBuffer, m_pageFirstLine, m_pageLastLine);

        char b[128];
        memset(b, 0, 128);
        snprintf(b, 128, "cursor: %d, %d", m_currentWindow->cursor()->pos().x,
                                           m_currentWindow->cursor()->pos().y);
        api->printBufferLine(0, 50, b);

        memset(b, '-', 128);
        b[127] = 0;
        api->printBufferLine(0, 0, b);
        api->printBufferLine(0, DEBUG_N_LINES, b);

        m_currentWindow->cursor()->update();

        int key = api->getKey();
        switch(key)
        {
            case KEY_DOWN:
                m_currentWindow->cursor()->move(0, 1);
                break;

            case KEY_UP:
                m_currentWindow->cursor()->move(0, -1);
                break;

            case KEY_LEFT:
                m_currentWindow->cursor()->move(-1, 0);
                break;

            case KEY_RIGHT:
                m_currentWindow->cursor()->move(1, 0);
                break;

            case KEY_PPAGE:
                m_currentWindow->cursor()->move(0, -Config::PAGE_SIZE);
                break;

            case KEY_NPAGE:
                m_currentWindow->cursor()->move(0, Config::PAGE_SIZE);
                break;

            case CTRL('x'):
                m_shouldClose = true;
                break;
        }

        EventHandler::process_events();

        api->refreshScreen();

    }

}

//=======================================================================================
int main(int argc, char *argv[])
{
    const char *filename = "synio.make";
    Synio synio(filename);

    return 0;

}

