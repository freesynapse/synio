
#include "ncurses_impl.h"

#include "../core.h"
#include "../utils/utils.h"


//
int Ncurses_Impl::initialize()
{
    m_screenPtr = initscr();

	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	clear();

    return RETURN_SUCCESS;

}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::shutdown()
{
    endwin();

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
    WINDOW *win = newwin(_frame->nrows(), _frame->ncols(), _frame->v0.y, _frame->v0.x);
    // wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
    box(win, 0, 0);
    wrefresh(win);
    return (API_WINDOW_PTR)win;

}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::getKey()
{
    return wgetch((WINDOW *)m_screenPtr);

}

//---------------------------------------------------------------------------------------
void Ncurses_Impl::moveCursor(API_WINDOW_PTR _w, int _x, int _y)
{
    wmove((WINDOW *)_w, _y, _x);
    
}

//---------------------------------------------------------------------------------------
void Ncurses_Impl::printBufferLine(API_WINDOW_PTR _w, int _cx, int _cy, char* _line)
{
    mvwprintw((WINDOW *)_w, _cy, _cx, "%s", _line);

}


