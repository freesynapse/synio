
#include "command_window.h"
#include "../platform/ncurses_colors.h"

//
CommandWindow::CommandWindow(const frame_t &_frame,
                             const std::string &_id,
                             bool _border) :
    Window(_frame, _id, _border)
{
    resize(_frame);

}

//---------------------------------------------------------------------------------------
CommandWindow::~CommandWindow()
{
    free(m_statusChStr);

}

//---------------------------------------------------------------------------------------
void CommandWindow::resize(frame_t _new_frame)
{
    size_t len = m_frame.ncols - 1;
    m_statusChStr = (CHTYPE_PTR)malloc(CHTYPE_SIZE * len);
    for (int i = 0; i < len; i++)
    {
        m_statusChStr[i] = CHTYPE(' ' | COLOR_PAIR(SYNIO_COLOR_STATUS));
    }
    m_statusChStr[len] = 0;

}

//---------------------------------------------------------------------------------------
void CommandWindow::redraw() 
{
    if (m_isWindowVisible)
    {
        // draw info line
        if (m_currentBuffer)
        {

        }
        api->printBufferLine(m_apiWindowPtr, 0, 0, m_statusChStr, m_frame.ncols - 1);


        if (m_enteringCommand)
        {

        }
    }

}


