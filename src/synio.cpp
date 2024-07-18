
#include "synio.h"

#include <assert.h>

#include "utils/log.h"
#include "utils/backtrace.h"
#include "utils/prefix_tree.h"
#include "event_handler.h"
#include "command.h"

//
Synio::Synio(const std::filesystem::path &_filename)
{
    EventHandler::initialize();
    PrefixTree::initialize();
    Command::initialize();

    //
    m_currentFilename = _filename;
    if (m_currentFilename.filename() == "" || !FileIO::does_file_exists(_filename))
    {
        m_currentFilename = FileIO::create_temp_file();
        LOG_INFO("no filename provided, created temp file '%s'", m_currentFilename.c_str());
    }

    // set up minimal window configuration (buffer and status windows)
    initialize();

    // register callbacks
    EventHandler::register_callback(EventType::EXIT, 
                                    EVENT_MEMBER_FNC(Synio::onExitEvent));
    EventHandler::register_callback(EventType::ADJUST_BUFFER_WINDOW,
                                    EVENT_MEMBER_FNC(Synio::onAdjustBufferWindowEvent));
    EventHandler::register_callback(EventType::CLOSE_COMMAND_WINDOW,
                                    EVENT_MEMBER_FNC(Synio::onCloseCommandWindow));
    EventHandler::register_callback(EventType::CLOSE_BUFFER_WINDOW,
                                    EVENT_MEMBER_FNC(Synio::onCloseFileBufferWindow));

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
    // delete m_currentBufferWindow;
    delete m_commandWindow;
    delete m_statusWindow;
    for (auto &it : m_bufferWindowsList)
        delete it.bufferWindowPtr;

}

//---------------------------------------------------------------------------------------
void Synio::initialize()
{
    api->getRenderSize(&m_screenSize);
    m_bufferWndFrame = frame_t(ivec2_t(8, 0), // taking line numbers space into account
                               ivec2_t(m_screenSize.x, m_screenSize.y - 1));
    m_currentBufferWindow = openFileBufferWindow(m_currentFilename);
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
FileBufferWindow *Synio::openFileBufferWindow(const std::filesystem::path &_filepath)
{
    auto fbe = find_file_buffer_entry_(_filepath);

    if (fbe != m_bufferWindowsList.end())
        return fbe->bufferWindowPtr;

    m_currentFilename = _filepath;

    // new entry
    FileBufferWindow *buffer_wnd = new FileBufferWindow(m_bufferWndFrame, 
                                                        m_currentFilename, 
                                                        false);
    file_buffer_entry_t new_fbe(_filepath, buffer_wnd);
    m_bufferWindowsList.emplace_front(new_fbe);

    return buffer_wnd;

}

//---------------------------------------------------------------------------------------
void Synio::switchToBuffer(const std::filesystem::path &_filepath)
{
    // find iterator to buffer
    auto fbe = find_file_buffer_entry_(_filepath);

    if (fbe == m_bufferWindowsList.end())
        return;

    file_buffer_entry_t fbe_copy = *fbe;

    // update list with new buffer
    m_bufferWindowsList.erase(fbe);
    m_bufferWindowsList.emplace_front(fbe_copy);

    // make the switch
    m_currentBufferWindow = fbe_copy.bufferWindowPtr;
    m_currentFilename = fbe_copy.path;
    m_currentBufferWindow->refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
size_t Synio::closeFileBufferWindow(const std::filesystem::path &_filepath)
{
    auto fbe = find_file_buffer_entry_(_filepath);
    if (fbe != m_bufferWindowsList.end())
    {
        auto last = m_bufferWindowsList.back();
        if (m_bufferWindowsList.size() == 1)
        {
            delete fbe->bufferWindowPtr;
            m_bufferWindowsList.erase(fbe);
        }
        else
        {
            if (fbe->bufferWindowPtr != last.bufferWindowPtr)
            {
                auto next = std::next(fbe);
                m_currentBufferWindow = next->bufferWindowPtr;
                m_currentFilename = next->path;
            }
            else
            {
                auto prev = std::prev(fbe);
                m_currentBufferWindow = prev->bufferWindowPtr;
                m_currentFilename = prev->path;
            }
            delete fbe->bufferWindowPtr;
            m_bufferWindowsList.erase(fbe);
    
            m_currentBufferWindow->refresh_next_frame_();
        }

    }

    return m_bufferWindowsList.size();

}

//---------------------------------------------------------------------------------------
CommandWindow *Synio::openCommandWindow()
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
void Synio::adjustBufferWindowFrameY(int _dy)
{
    // adjustBufferWindowFrame(m_currentBufferWindow, &m_bufferWndFrame, 0, 0, 0, _dy);
    adjustBufferWindowFrame(m_statusWindow, &m_statusWndFrame, 0, _dy, 0, _dy);

}

//---------------------------------------------------------------------------------------
void Synio::onExitEvent(Event *_e)
{
    LOG_INFO("Synio exit event received.");
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
void Synio::onCloseCommandWindow(Event *_e)
{
    // N.B.: memory freed before creation in newCommandWindow()
    adjustBufferWindowFrameY(m_commandWindow->frame().nrows);
    m_commandMode = false;

    m_focusedWindow = m_currentBufferWindow;
    clear_redraw_refresh_window_ptr_(m_currentBufferWindow);
    m_focusedWindow->refresh_next_frame_();   // needed for correct cursor behaviour

}

//---------------------------------------------------------------------------------------
void Synio::onCloseFileBufferWindow(Event *_e)
{
    CloseFileBufferEvent *e = dynamic_cast<CloseFileBufferEvent *>(_e);
    // check if we're closing the current buffer
    size_t buffers_left;
    if (e->close_this)
        buffers_left = closeFileBufferWindow(m_currentFilename);
    else
        buffers_left = closeFileBufferWindow(e->filepath);

    // create a temp buffer if last buffer was closed
    if (buffers_left == 0)
    {
        m_currentFilename = FileIO::create_temp_file();
        m_currentBufferWindow = openFileBufferWindow(m_currentFilename);
        m_currentBufferWindow->refresh_next_frame_();
    }

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
        //    the command window (as an CloseCommandWindowEvent).
        if (key == CTRL('x') && !m_commandMode)
        {
            m_commandMode = true;
            m_commandWindow = openCommandWindow();

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

