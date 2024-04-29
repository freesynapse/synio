
#include "synio.h"

#include "utils/log.h"
#include "utils/backtrace.h"
#include "event_handler.h"


//
Synio::Synio(const std::string &_filename)
{
    EventHandler::initialize();

    m_filename = _filename;
    initialize();

    // register callbacks
    // EventHandler::register_callback(EventType::BUFFER_SCROLL, 
    //                                 EVENT_MEMBER_FNC(Synio::onBufferScroll));

    // register signal handler
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;   // in backtrace.h|.cpp
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    if (sigaction(SIGSEGV, &sa, (struct sigaction *)NULL) != 0)
        fprintf(stderr, "could not set signal handler for %d (%s)", SIGSEGV, strsignal(SIGSEGV));

    //
    mainLoop();

}

//---------------------------------------------------------------------------------------
Synio::~Synio()
{
    EventHandler::shutdown();

    //
    delete m_bufferWindow;
    delete m_commandWindow;
}

//---------------------------------------------------------------------------------------
void Synio::initialize()
{
    api->getRenderSize(&m_screenSize);
    frame_t buffer_wnd_rect(ivec2_t(8, 0), ivec2_t(m_screenSize.x, m_screenSize.y));
    m_bufferWindow = new FileBufferWindow(buffer_wnd_rect, "buffer_window", false);
    m_bufferWindow->readFileToBuffer(m_filename);
    
    // -- DEBUG
    m_currentWindow = m_bufferWindow;
    //m_bufferWindow->clear();
    //m_bufferWindow->redraw();
    //m_bufferWindow->refresh();
    //m_currentWindow->updateCursor();

    // -- DEBUG -- working
    //frame_t f = frame_t(ivec2_t(0, m_screenSize.y - 1), m_screenSize);
    //m_commandWindow = new LineBufferWindow(f, "test_dialog");
    //m_commandWindow->setQuery("test query: ", ivec2_t(0, 0));
    //m_currentWindow = m_commandWindow;

    // clear_redraw_refresh_window_();

    // TEST -- command window
    frame_t command_wnd_rect = frame_t(ivec2_t(0, m_screenSize.y - 2),
                                       ivec2_t(m_screenSize.x, m_screenSize.y));
    m_commandWindow = new LineBufferWindow(command_wnd_rect, "command_window");
    m_commandWindow->setVisibility(true);

}

//---------------------------------------------------------------------------------------
void Synio::mainLoop()
{
    while (!m_shouldClose)
    {
        // --- BEGIN DRAWING : order matters!
        //

        m_currentWindow->clear();
        m_currentWindow->redraw();
        m_currentWindow->refresh();

        // actually swap the buffers
        api->redrawScreen();

        //
        // --- END DRAWING

        //
        int key = api->getKey();

        // check for control characters (e.g. ctrl, shift, alt key combinations)
        CtrlKeyAction ctrl_action = api->getCtrlKeyAction(key);
            
        
        // command mode
        //
        if (key == CTRL('x') && !m_commandMode)
        {
            m_commandMode = !m_commandMode;
            m_currentWindow = m_commandWindow;
            clear_redraw_refresh_window_();
        }

        else if (key == CTRL('x') && m_commandMode)
        {
            m_commandMode = !m_commandMode;
            m_currentWindow = m_bufferWindow;
            clear_redraw_refresh_window_();
        }
        
        // interactive (editing) mode
        //
        else
        {
            m_currentWindow->handleInput(key, ctrl_action);

            // TODO : move to command mode
            if (key == CTRL('q'))
                m_shouldClose = true;
        }

        EventHandler::process_events();

    }

}

