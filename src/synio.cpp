
#include <assert.h>

#include "synio.h"
#include "utils/log.h"
#include "file_io.h"
#include "event_handler.h"
#include "core.h"

//
Synio::Synio(const std::string &_filename)
{
    EventHandler::init();

    m_filename = _filename;
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
    delete m_lineNumbersWindow;

}

//---------------------------------------------------------------------------------------
void Synio::resize()
{
    api->getRenderSize(&m_screenSize);
    irect_t buffer_window_rect(ivec2_t(8, 0), ivec2_t(m_screenSize.x, m_screenSize.y));
    m_bufferWindow = new Buffer(&buffer_window_rect, "buffer_window", false);
    m_bufferWindow->readFromFile(m_filename);
    m_currentBuffer = m_bufferWindow;

    //irect_t buffer_window_rect(ivec2_t(8, 0), ivec2_t(m_screenSize.x, m_screenSize.y));
    //irect_t line_numbers_rect(ivec2_t(0), ivec2_t(6, m_screenSize.y));
    //m_lineNumbersWindow = new LineNumbers(&line_numbers_rect, "line_numbers", false);
    //if (!Config::SHOW_LINE_NUMBERS)
    //    m_lineNumbersWindow->setVisibility(false);
    //m_lineNumbersWindow->setBuffer(m_currentBuffer);

}

//---------------------------------------------------------------------------------------
void Synio::onBufferScroll(Event *_e)
{
    BufferScrollEvent *e = dynamic_cast<BufferScrollEvent*>(_e);
    dynamic_cast<Buffer*>(m_currentBuffer)->onScroll(e);

}

//---------------------------------------------------------------------------------------
void Synio::mainLoop()
{
    while (!m_shouldClose)
    {
        // --- BEGIN DRAWING

        // probably move into the Buffer class so that frame can be auto determined
        //m_lineNumbersWindow->clear();
        //m_lineNumbersWindow->draw();
        //m_lineNumbersWindow->refresh();
        // did just that -- now a part of the Buffer class

        m_currentBuffer->clear();    // very good, clear() clears the borders...
        // ---> https://stackoverflow.com/questions/33986047/ncurses-is-it-possible-to-refresh-a-window-without-removing-its-borders
        // -- Changed in the Window class so that all extra 'border' windows can be drawn.
        m_currentBuffer->draw();
        m_currentBuffer->updateCursor();
        m_currentBuffer->refresh();

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
                case KEY_DOWN:  m_currentBuffer->moveCursor(0, 1);   break;
                case KEY_UP:    m_currentBuffer->moveCursor(0, -1);  break;
                case KEY_LEFT:  m_currentBuffer->moveCursor(-1, 0);  break;
                case KEY_RIGHT: m_currentBuffer->moveCursor(1, 0);   break;
                case KEY_PPAGE: m_currentBuffer->moveCursor(0, -Config::PAGE_SIZE);  break;
                case KEY_NPAGE: m_currentBuffer->moveCursor(0, Config::PAGE_SIZE);   break;
                case KEY_HOME:  m_currentBuffer->moveCursorToLineBegin(); break;
                case KEY_END:   m_currentBuffer->moveCursorToLineEnd(); break;

                // command control
                case CTRL('x'):
                    m_shouldClose = true;
                    break;

                // test insert string in line
                case CTRL('v'):
                    m_currentBuffer->insertStrAtCursor((char *)s, strlen(s));
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

        m_currentBuffer->updateCursor();
        m_currentBuffer->refresh();

        EventHandler::process_events();

    }

}

//=======================================================================================
int main(int argc, char *argv[])
{
    const char *filename = "test.txt";

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

