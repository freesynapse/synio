
#include "synio.h"

#include <assert.h>

#include "utils/log.h"
#include "utils/backtrace.h"
#include "utils/prefix_tree.h"
#include "event_handler.h"
#include "command.h"

//
Synio::Synio(const std::string &_filename)
{
    EventHandler::initialize();
    PrefixTree::initialize();
    Command::initialize();

    //
    m_currentFilename = _filename;
    if (m_currentFilename == "" || !FileIO::does_file_exists(_filename))
    {
        m_currentFilename = FileIO::create_temp_file();
        LOG_INFO("no file entered, created this instead: '%s'", m_currentFilename.c_str());
    }

    // set up minimal window configuration (buffer and status windows)
    initialize();

    // register callbacks
    EventHandler::register_callback(EventType::EXIT, 
                                    EVENT_MEMBER_FNC(Synio::onExitEvent));
    EventHandler::register_callback(EventType::ADJUST_BUFFER_WINDOW,
                                    EVENT_MEMBER_FNC(Synio::onAdjustBufferWindowEvent));
    EventHandler::register_callback(EventType::DELETE_COMMAND_WINDOW,
                                    EVENT_MEMBER_FNC(Synio::deleteCommandWindow));

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
    FileIO::delete_temp_files();

    //
    delete m_currentBufferWindow;
    delete m_commandWindow;
    delete m_statusWindow;

}

//---------------------------------------------------------------------------------------
void Synio::initialize()
{
    api->getRenderSize(&m_screenSize);
    m_bufferWndFrame = frame_t(ivec2_t(8, 0), // taking line numbers space into account
                               ivec2_t(m_screenSize.x, m_screenSize.y - 1));
    m_currentBufferWindow = newFileBufferWindow(m_currentFilename);
    m_focusedWindow = m_currentBufferWindow;

    // command window
    m_commandWndFrame = frame_t(ivec2_t(0, m_screenSize.y - Config::COMMAND_WINDOW_HEIGHT),
                                ivec2_t(m_screenSize.x, m_screenSize.y));

    // status window (below buffer, bottom of the screen)
    m_statusWndFrame = frame_t(ivec2_t(0, m_screenSize.y - 1),
                               ivec2_t(m_screenSize.x, m_screenSize.y));
    m_statusWindow = new StatusWindow(m_statusWndFrame, "status_window", false);

}

//---------------------------------------------------------------------------------------
FileBufferWindow *Synio::newFileBufferWindow(const std::string &_filename)
{
    m_currentFilename = _filename;

    FileBufferWindow *buffer_wnd = new FileBufferWindow(m_bufferWndFrame, 
                                                        m_currentFilename, 
                                                        false);
    // store in map
    m_bufferWindows[_filename] = buffer_wnd;

    return buffer_wnd;

}

//---------------------------------------------------------------------------------------
void Synio::deleteFileBufferWindow(Event *_e)
{

}

//---------------------------------------------------------------------------------------
CommandWindow *Synio::newCommandWindow()
{
    delete m_commandWindow;

    CommandWindow *cmd_wnd = new CommandWindow(m_commandWndFrame,
                                               "command_window",
                                               this,
                                               false);    
    cmd_wnd->setQueryPrefix("cmd:");
    
    return cmd_wnd;
    
}

//---------------------------------------------------------------------------------------
void Synio::deleteCommandWindow(Event *_e)
{
    // N.B.: memory freed before creation in newCommandWindow()
    adjustBufferWindowFrameY(m_commandWindow->frame().nrows);
    m_commandMode = false;

    m_focusedWindow = m_currentBufferWindow;
    clear_redraw_refresh_window_ptr_(m_currentBufferWindow);
    m_focusedWindow->refresh_next_frame_();   // needed for correct cursor behaviour

}

//---------------------------------------------------------------------------------------
void Synio::adjustBufferWindowFrameY(int _dy)
{
    adjustBufferWindowFrame(m_currentBufferWindow, &m_bufferWndFrame, 0, 0, 0, _dy);
    adjustBufferWindowFrame(m_statusWindow, &m_statusWndFrame, 0, _dy, 0, _dy);

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
    adjustBufferWindowFrameY(dy);

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
            m_statusWindow->update(m_currentBufferWindow);
            if (m_statusWindow->wasUpdated())
            {
                m_statusWindow->clear();
                m_statusWindow->redraw();
                m_statusWindow->refresh();
            }            
        }

        m_focusedWindow->clear();
        m_focusedWindow->redraw();
        m_focusedWindow->refresh();

        // actually swap the buffers
        api->redrawScreen();

        //
        // --- END DRAWING

        //
        int key = api->getKey();

        // check for control characters (e.g. ctrl, shift, alt key combinations)
        CtrlKeyAction ctrl_action = api->getCtrlKeyAction(key);
            
        
        // Command mode control
        // -- Entering command mode is handled here, leaving command mode is done through
        //    the command window (as an DeleteCommandWindowEvent).
        if (key == CTRL('x') && !m_commandMode)
        {
            m_commandMode = true;
            m_commandWindow = newCommandWindow();

            // make space for the command window
            adjustBufferWindowFrameY(-Config::COMMAND_WINDOW_HEIGHT);

            m_focusedWindow = m_commandWindow;
            clear_redraw_refresh_window_ptr_(m_commandWindow);
            m_focusedWindow->refresh_next_frame_();   // needed for correct cursor behaviour
        }

        // interactive mode
        //
        else
            m_focusedWindow->handleInput(key, ctrl_action);

        //
        EventHandler::process_events();

    }

}

