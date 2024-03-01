
#include "synio.h"
#include "utils/log.h"
#include "event_handler.h"
#include "core.h"

// DEBUG
#include "platform/ncurses_colors.h"

//
Synio::Synio(const std::string &_filename)
{
    EventHandler::initialize();

    m_filename = _filename;
    initialize();

    // register callbacks
    // EventHandler::register_callback(EventType::BUFFER_SCROLL, 
    //                                 EVENT_MEMBER_FNC(Synio::onBufferScroll));

    //
    mainLoop();

}

//---------------------------------------------------------------------------------------
Synio::~Synio()
{
    EventHandler::shutdown();

    //
    delete m_bufferWindow;
    delete m_dialog;
}

//---------------------------------------------------------------------------------------
void Synio::initialize()
{
    api->getRenderSize(&m_screenSize);
    frame_t buffer_window_rect(ivec2_t(8, 0), ivec2_t(m_screenSize.x, m_screenSize.y - 1));
    m_bufferWindow = new FileBufferWindow(buffer_window_rect, "buffer_window", false);
    m_bufferWindow->readFromFile(m_filename);
    
    // -- DEBUG
    m_currentWindow = m_bufferWindow;
    m_bufferWindow->clear();
    m_bufferWindow->redraw();
    m_bufferWindow->refresh();

    // -- DEBUG
    frame_t f = frame_t(ivec2_t(0, m_screenSize.y - 1), m_screenSize);
    m_dialog = new LineBufferWindow(f, "test_dialog");
    m_dialog->setQuery("test query: ", ivec2_t(0, 0));
    // m_currentWindow = m_dialog;

}

//---------------------------------------------------------------------------------------
void Synio::mainLoop()
{
    while (!m_shouldClose)
    {
        // --- BEGIN DRAWING : order matters!
        //

        // 
        m_currentWindow->clear();    // very good, clear() clears the borders...
        // ---> https://stackoverflow.com/questions/33986047/ncurses-is-it-possible-to-refresh-a-window-without-removing-its-borders
        // -- Changed in the Window class so that all extra 'border' windows can be drawn.
        m_currentWindow->redraw();
        m_currentWindow->refresh();
        
        //
        // --- END DRAWING

        // actually swap the buffers
        api->redrawScreen();

        // get keypress; may be important
        int key = api->getKey();

        // check for control characters (e.g. ctrl, shift, alt key combinations)
        CtrlKeyAction ctrl_action = api->getCtrlKeyAction(key);
        
        // command mode
        //
        if (m_commandMode)
        {
            
        }
        // interactive (editing) mode
        //
        else
        {
            m_currentWindow->handleInput(key, ctrl_action);

            // TODO : enter command mode (eventually)
            if (key == CTRL('x'))
                m_shouldClose = true;
        }

        EventHandler::process_events();

    }

}

