
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

}

//---------------------------------------------------------------------------------------
void Synio::initialize()
{
    api->getRenderSize(&m_screenSize);
    frame_t buffer_window_rect(ivec2_t(8, 0), ivec2_t(m_screenSize.x, m_screenSize.y));
    m_bufferWindow = new FileBufferWindow(buffer_window_rect, "buffer_window", false);
    m_bufferWindow->readFromFile(m_filename);
    m_currentBuffer = m_bufferWindow;

}

//---------------------------------------------------------------------------------------
void Synio::mainLoop()
{
    while (!m_shouldClose)
    {
        // --- BEGIN DRAWING : order matters!
        //

        // 
        m_currentBuffer->clear();    // very good, clear() clears the borders...
        // ---> https://stackoverflow.com/questions/33986047/ncurses-is-it-possible-to-refresh-a-window-without-removing-its-borders
        // -- Changed in the Window class so that all extra 'border' windows can be drawn.
        m_currentBuffer->redraw();
        m_currentBuffer->refresh();
        
        //
        // --- END DRAWING

        // actually swap the buffers
        api->redrawScreen();

        // get keypress; may be important
        int key = api->getKey();

        // check for control characters (e.g. ctrl, shift, alt key combinations)
        CtrlKeyAction ctrl_action = api->getCtrlKeyAction(key);
        
        // DEBUG
        const char *debug_insert_str = "__TEST_INSERT_STRING__";
        frame_t debug_new_frame = frame_t(ivec2_t(8, 0), ivec2_t(m_screenSize.x-10, m_screenSize.y-10));

        // command mode
        //
        if (m_commandMode)
        {
            
        }
        // interactive (editing) mode
        //
        else
        {
            if (ctrl_action != CtrlKeyAction::NONE)
            {
                switch (ctrl_action)
                {
                    case CtrlKeyAction::CTRL_LEFT:  m_currentBuffer->moveCursorToColDelim(-1);  break;
                    case CtrlKeyAction::CTRL_RIGHT: m_currentBuffer->moveCursorToColDelim(1);   break;
                    case CtrlKeyAction::CTRL_UP:    m_currentBuffer->moveCursorToRowDelim(-1);  break;
                    case CtrlKeyAction::CTRL_DOWN:  m_currentBuffer->moveCursorToRowDelim(1);   break;
                    case CtrlKeyAction::CTRL_HOME:   m_currentBuffer->moveHome();               break;
                    case CtrlKeyAction::CTRL_END:   m_currentBuffer->moveEnd();                 break;
                    default: LOG_INFO("ctrl keycode %d : %s", key, ctrlActionStr(ctrl_action)); break;

                }
            }
            else
            {
                switch (key)
                {
                    // cursor movement
                    case KEY_DOWN:  m_currentBuffer->moveCursor(0, 1);  break;
                    case KEY_UP:    m_currentBuffer->moveCursor(0, -1); break;
                    case KEY_LEFT:  m_currentBuffer->moveCursor(-1, 0); break;
                    case KEY_RIGHT: m_currentBuffer->moveCursor(1, 0);  break;
                    case KEY_PPAGE: m_currentBuffer->movePageUp();      break;
                    case KEY_NPAGE: m_currentBuffer->movePageDown();    break;
                    case KEY_HOME:  m_currentBuffer->moveCursorToLineBegin();   break;
                    case KEY_END:   m_currentBuffer->moveCursorToLineEnd();     break;

                    // TODO : enter command mode (eventually)
                    case CTRL('x'):
                        m_shouldClose = true;
                        break;

                    case CTRL('a'):
                        m_currentBuffer->resize(debug_new_frame);
                        break;

                    // test insert string in line
                    case CTRL('v'):
                        m_currentBuffer->insertStrAtCursor((char *)debug_insert_str, strlen(debug_insert_str));
                        break;

                    case CTRL('d'):
                        LOG_INFO("current line: '%s' (%zu)", 
                                 m_currentBuffer->currentLine()->__debug_str(),
                                 m_currentBuffer->currentLine()->len);
                        break;

                    case KEY_DC:
                        m_currentBuffer->deleteCharAtCursor();
                        break;

                    case KEY_BACKSPACE:
                        m_currentBuffer->deleteCharBeforeCursor();
                        break;
                    
                    case 10:    // <ENTER>
                        m_currentBuffer->insertNewLine();
                        break;

                    default:
                        m_currentBuffer->insertCharAtCursor((char)key);
                        break;

                }
            }
        }

        // m_currentBuffer->updateCursor();

        EventHandler::process_events();

    }

}

