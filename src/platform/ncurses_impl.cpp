
#include <locale.h>

#include "ncurses_impl.h"

#include "../core.h"
#include "../utils/utils.h"
#include "../utils/log.h"


//
int Ncurses_Impl::initialize()
{
    setlocale(LC_ALL, "");

    m_screenPtr = initscr();

	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	clear();
    refresh();

    LOG_INFO("ncurses initialized.");

    return RETURN_SUCCESS;

}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::shutdown()
{
    endwin();

    LOG_INFO("ncurses shutdown gracefully.");
    
    return RETURN_SUCCESS;

}

//---------------------------------------------------------------------------------------
void Ncurses_Impl::getRenderSize(ivec2_t *_v)
{
    int x, y;
    getmaxyx((WINDOW *)m_screenPtr, y, x);
    _v->x = x;
    _v->y = y;

}

//---------------------------------------------------------------------------------------
API_WINDOW_PTR Ncurses_Impl::newWindow(irect_t *_frame)
{
    WINDOW *win = newwin(_frame->nrows, _frame->ncols, _frame->v0.y, _frame->v0.x);
    wrefresh(win);
    return (API_WINDOW_PTR)win;

}

//---------------------------------------------------------------------------------------
API_WINDOW_PTR Ncurses_Impl::newBorderWindow(irect_t *_frame)
{
    // takes the original window frame as argument, so have to check that within bounds
    irect_t new_frame = irect_t(ivec2_t(_frame->v0.x - 1, _frame->v0.y - 1),
                                ivec2_t(_frame->v1.x + 1, _frame->v1.y + 1));
    ivec2_t screen_dim;
    getRenderSize(&screen_dim);
    if (new_frame.v0.x < 0 || new_frame.v0.y < 0 ||
        new_frame.v1.x >= screen_dim.x || new_frame.v1.y >= screen_dim.y)
        LOG_WARNING("window border out of bounds");

    WINDOW *win = newwin(new_frame.nrows, new_frame.ncols, 
                         new_frame.v0.y, new_frame.v0.x);

    wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
    wrefresh(win);

    return win;

}

//---------------------------------------------------------------------------------------
void Ncurses_Impl::deleteWindow(API_WINDOW_PTR _w)
{
    wborder((WINDOW *)_w, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh((WINDOW *)_w);
    delwin((WINDOW *)_w);

}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::getKey()
{
    return wgetch((WINDOW *)m_screenPtr);

}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::moveCursor(API_WINDOW_PTR _w, int _x, int _y)
{
    return wmove((WINDOW *)_w, _y, _x);
    
}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::printBufferLine(API_WINDOW_PTR _w, int _cx, int _cy, char* _line)
{
    int len = mvwprintw((WINDOW *)_w, _cy, _cx, "%s", _line);
    return len;
}


