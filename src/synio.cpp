
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
    delete m_statusWindow;

}

//---------------------------------------------------------------------------------------
void Synio::initialize()
{
    api->getRenderSize(&m_screenSize);
    frame_t buffer_wnd_rect(ivec2_t(8, 0), ivec2_t(m_screenSize.x, m_screenSize.y - 1));
    m_bufferWindow = new FileBufferWindow(buffer_wnd_rect, "buffer_window", false);
    m_bufferWindow->readFileToBuffer(m_filename);
    
    // -- DEBUG
    m_focusWindow = m_bufferWindow;

    // command window
    frame_t command_wnd_rect = frame_t(ivec2_t(0, m_screenSize.y - 1),
                                       ivec2_t(m_screenSize.x, m_screenSize.y));
    m_commandWindow = new LineBufferWindow(command_wnd_rect, "command_window", false);
    m_commandWindow->setVisibility(false);

    // TEST -- status window
    frame_t status_wnd_rect = frame_t(ivec2_t(0, m_screenSize.y - 1),
                                      ivec2_t(m_screenSize.x, m_screenSize.y));
    m_statusWindow = new StatusWindow(status_wnd_rect, "status_window", false);

}

//---------------------------------------------------------------------------------------
void Synio::mainLoop()
{
    while (!m_shouldClose)
    {
        // --- BEGIN DRAWING : order matters!
        //
        if (m_statusWindow)
        {
            m_statusWindow->update(m_bufferWindow);
            if (m_statusWindow->wasUpdated())
            {
                m_statusWindow->clear();
                m_statusWindow->redraw();
                m_statusWindow->refresh();
            }            
        }

        m_focusWindow->clear();
        m_focusWindow->redraw();
        m_focusWindow->updateCursor();
        m_focusWindow->refresh();

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
            m_commandMode = true;
            m_focusWindow = m_commandWindow;
            m_commandWindow->setQueryPrefix("C-x");
            m_commandWindow->setVisibility(true);
            clear_redraw_refresh_window_();
            m_focusWindow->refresh_next_frame_();   // needed for correct cursor behaviour
        }

        else if (key == CTRL('x') && m_commandMode)
        {
            m_commandMode = false;
            m_focusWindow = m_bufferWindow;
            m_commandWindow->setVisibility(false);
            clear_redraw_refresh_window_();
            m_focusWindow->refresh_next_frame_();   // needed for correct cursor behaviour
        }
        
        // interactive (editing) mode
        //
        else
        {
            m_focusWindow->handleInput(key, ctrl_action);

            // TODO : move to command mode
            if (key == CTRL('q'))
                m_shouldClose = true;
        }

        EventHandler::process_events();

    }

}

