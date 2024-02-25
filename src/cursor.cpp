
#include "cursor.h"
#include "window/window.h"
#include "event_handler.h"

//
void Cursor::set_cpos(int _x, int _y)
{
    m_cpos.x = _x;
    m_cpos.y = _y;

    clamp_to_frame_();
    m_rpos.y = m_cpos.y;
    m_rpos.x = calc_rposx_from_cposx_();

    api->moveCursor(m_parent->m_apiWindowPtr, m_rpos.x, m_rpos.y);

}

//---------------------------------------------------------------------------------------
void Cursor::set_cx(int _x)
{
    m_cpos.x = _x;

    clamp_to_frame_();
    m_rpos.y = m_cpos.y;
    m_rpos.x = calc_rposx_from_cposx_();
    
    api->moveCursor(m_parent->m_apiWindowPtr, m_rpos.x, m_rpos.y);

}

//---------------------------------------------------------------------------------------
void Cursor::set_cy(int _y)
{
    m_cpos.y = _y;

    clamp_to_frame_();
    m_rpos.y = m_cpos.y;
    m_rpos.x = calc_rposx_from_cposx_();

    api->moveCursor(m_parent->m_apiWindowPtr, m_rpos.x, m_rpos.y);
   
}

//---------------------------------------------------------------------------------------
void Cursor::update()
{
    if (m_dy != 0 && m_dx == 0)
    {
        // we switched row, so we want to preserve rpos.x
        m_rpos.x = m_last_rx;
        // calculate cpos x position from rpos
        m_cpos.x = calc_cposx_from_rposx_();
        // correct in relation to line len
        if (m_cpos.x >m_parent-> m_currentLine->len)
           m_cpos.x = (int)(m_parent->m_currentLine->len);
    }
    
    clamp_to_frame_();

    // calculate rpos from cpos
    m_rpos.y = m_cpos.y;
    m_rpos.x = calc_rposx_from_cposx_();

    // update last pos if needed
    if (m_dx != 0)
        m_last_rx = m_rpos.x;

    
    if (api->moveCursor(m_parent->m_apiWindowPtr, m_rpos.x, m_rpos.y) == ERR)
       LOG_WARNING("%s : m_pos (%d, %d), window lim (%d, %d)",
                   __func__,
                   m_rpos.x,
                   m_rpos.y,
                   m_parent->m_frame.v1.x,
                   m_parent->m_frame.v1.y);
    
    //if (api->moveCursor(m_parent->m_apiWindowPtr, m_cpos.x, m_cpos.y) == ERR)
    //    LOG_WARNING("%s : m_pos (%d, %d), window lim (%d, %d)",
    //                __func__,
    //                m_cpos.x,
    //                m_cpos.y,
    //                m_parent->m_frame.v1.x,
    //                m_parent->m_frame.v1.y);

    m_dx = 0;
    m_dy = 0;

}

//---------------------------------------------------------------------------------------
// void Cursor::calc_rposx_from_cposx_()
// {
//     m_rpos.y = m_cpos.y;
//     line_t *line = m_parent->m_currentLine;

//     int r = 0;
//     int c = 0;
//     for (c = 0; c < m_cpos.x; c++)
//     {
//         if ((line->content[c] & A_CHARTEXT) == '\t')
//             r = (r + (Config::TAB_SIZE - (r % Config::TAB_SIZE)));
//         else
//             r++;
//     }

//     m_rpos.x = r;
    
// }

// //---------------------------------------------------------------------------------------
// void Cursor::calc_cpos_from_rpos_()
// {
//     m_cpos.y = m_rpos.y;
//     line_t *line = m_parent->m_currentLine;

//     int c = 0;
//     int r = 0;
//     for (r = 0; r < m_rpos.x; c++)
//     {
//         if ((line->content[c] & A_CHARTEXT) == '\t')
//             r = (r + (Config::TAB_SIZE - (r % Config::TAB_SIZE)));
//         else
//             r++;
//     }
    
//     m_cpos.x = c;

// }

//---------------------------------------------------------------------------------------
void Cursor::move(int _dx, int _dy)
{
    m_cpos.x += _dx;
    m_cpos.y += _dy;

    m_dx = _dx;
    m_dy = _dy;

    // x scrolling

    // y scrolling
    if (m_cpos.y < 0)
    {
        EventHandler::push_event(new BufferScrollEvent(
                                        Y_AXIS,
                                        m_dy,
                                        abs(m_cpos.y),
                                        m_parent));
        // keep cursor inside window
    }
    else if (m_cpos.y > m_parent->m_frame.nrows - 1)
    {
        EventHandler::push_event(new BufferScrollEvent(
                                        Y_AXIS,
                                        m_dy,
                                        abs(m_cpos.y - (m_parent->m_frame.nrows - 1)),
                                        m_parent));
    }

    clamp_to_frame_();

}

//---------------------------------------------------------------------------------------
void Cursor::clamp_to_frame_()
{
    m_cpos.x = CLAMP(m_cpos.x, 0, m_parent->m_frame.ncols - 1);
    m_cpos.y = CLAMP(m_cpos.y, 0, m_parent->m_frame.nrows - 1);

}

//---------------------------------------------------------------------------------------
int Cursor::calc_rposx_from_cposx_()
{
   line_t *line = m_parent->m_currentLine;

   int r = 0;
   int c = 0;
   for (c = 0; c < m_cpos.x; c++)
   {
       if ((line->content[c] & A_CHARTEXT) == '\t')
           r = (r + (Config::TAB_SIZE - (r % Config::TAB_SIZE)));
       else
           r++;
   }

   return r;
   
}

//---------------------------------------------------------------------------------------
int Cursor::calc_cposx_from_rposx_()
{
   line_t *line = m_parent->m_currentLine;

   int c = 0;
   int r = 0;
   for (r = 0; r < m_rpos.x; c++)
   {
       if ((line->content[c] & A_CHARTEXT) == '\t')
           r = (r + (Config::TAB_SIZE - (r % Config::TAB_SIZE)));
       else
           r++;
   }

   return c;

}

