
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
    EventHandler::register_callback(EventType::EXIT, 
                                    EVENT_MEMBER_FNC(Synio::onExitEvent));
    EventHandler::register_callback(EventType::ADJUST_BUFFER_WINDOW,
                                    EVENT_MEMBER_FNC(Synio::onAdjustBufferWindowEvent));

    // register signal handler
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;   // backtrace.h|.cpp
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
    m_bufferWndFrame = frame_t(ivec2_t(8, 0), 
                               ivec2_t(m_screenSize.x, m_screenSize.y - 1));
    m_bufferWindow = new FileBufferWindow(m_bufferWndFrame, "buffer_window", false);
    m_bufferWindow->readFileToBuffer(m_filename);
    
    //
    m_focusedWindow = m_bufferWindow;

    // command window
    m_commandWndFrame = frame_t(ivec2_t(0, m_screenSize.y - Config::COMMAND_WINDOW_HEIGHT),
                               ivec2_t(m_screenSize.x, m_screenSize.y));

    // status window (below buffer, bottom of the screen)
    m_statusWndFrame = frame_t(ivec2_t(0, m_screenSize.y - 1),
                               ivec2_t(m_screenSize.x, m_screenSize.y));
    m_statusWindow = new StatusWindow(m_statusWndFrame, "status_window", false);

}

//---------------------------------------------------------------------------------------
CommandWindow *Synio::newCommandWindow()
{
    CommandWindow *cmd_wnd = new CommandWindow(m_commandWndFrame,
                                               "command_window",
                                               false);
    cmd_wnd->setVisibility(true);
    cmd_wnd->setQueryPrefix("C-x");
    cmd_wnd->setVisibility(true);
    return cmd_wnd;
    
}

//---------------------------------------------------------------------------------------
void Synio::adjustBufferWindowFrame(BufferWindowBase *_w, frame_t *_w_frame, int _dx0, 
                                    int _dy0, int _dx1, int _dy1)
{
    _w_frame->v0.x = MAX(_w_frame->v0.x + _dx0, 0);
    _w_frame->v0.y = MAX(_w_frame->v0.y + _dy0, 0);
    _w_frame->v1.x = MAX(_w_frame->v1.x + _dx1, 1);
    _w_frame->v1.y = MAX(_w_frame->v1.y + _dy1, 1);
    _w_frame->update_dims();

    _w->resize(*_w_frame);
    clear_redraw_refresh_window_ptr_(_w);

}

//---------------------------------------------------------------------------------------
void Synio::onExitEvent(Event *_e)
{
    LOG_INFO("app exit event received.");
    m_shouldClose = true;
}
//---------------------------------------------------------------------------------------
void Synio::onAdjustBufferWindowEvent(Event *_e)
{
    AdjustBufferWindowEvent *e = dynamic_cast<AdjustBufferWindowEvent *>(_e);
    int dy = e->dy;

    adjustBufferWindowFrame(m_bufferWindow, &m_bufferWndFrame, 0, 0, 0, dy);
    adjustBufferWindowFrame(m_statusWindow, &m_statusWndFrame, 0, dy, 0, dy);

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

        m_focusedWindow->clear();
        m_focusedWindow->redraw();
        m_focusedWindow->updateCursor();
        m_focusedWindow->refresh();

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
            m_commandWindow = newCommandWindow();
            // make space for the command window
            adjustBufferWindowFrame(m_bufferWindow, &m_bufferWndFrame, 
                                    0, 0, 
                                    0, -Config::COMMAND_WINDOW_HEIGHT);
            adjustBufferWindowFrame(m_statusWindow, &m_statusWndFrame, 
                                    0, -Config::COMMAND_WINDOW_HEIGHT, 
                                    0, -Config::COMMAND_WINDOW_HEIGHT);

            m_focusedWindow = m_commandWindow;
            clear_redraw_refresh_window_ptr_(m_commandWindow);
            m_focusedWindow->refresh_next_frame_();   // needed for correct cursor behaviour
        }

        else if (key == CTRL('x') && m_commandMode)
        {
            m_commandMode = false;
            delete m_commandWindow;
            m_commandWindow = NULL;
            // restore buffer and status windows
            adjustBufferWindowFrame(m_bufferWindow, &m_bufferWndFrame, 
                                    0, 0, 
                                    0, Config::COMMAND_WINDOW_HEIGHT);
            adjustBufferWindowFrame(m_statusWindow, &m_statusWndFrame, 
                                    0, Config::COMMAND_WINDOW_HEIGHT, 
                                    0, Config::COMMAND_WINDOW_HEIGHT);

            m_focusedWindow = m_bufferWindow;
            clear_redraw_refresh_window_ptr_(m_bufferWindow);
            m_focusedWindow->refresh_next_frame_();   // needed for correct cursor behaviour
        }
        
        // interactive (editing) mode
        //
        else
        {
            m_focusedWindow->handleInput(key, ctrl_action);

            // TODO : move to command mode
            // if (key == CTRL('q'))
                // m_shouldClose = true;
        }

        EventHandler::process_events();

    }

}

