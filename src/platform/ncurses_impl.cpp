
#include "ncurses_impl.h"

#include "../utils/utils.h"


//
int Ncurses_Impl::initialize()
{
    m_mainWindow = initscr();

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
    getmaxyx(m_mainWindow, y, x);
    _v->x = x;
    _v->y = y;

}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::getKey()
{
    return wgetch(m_mainWindow);

}

//---------------------------------------------------------------------------------------
void Ncurses_Impl::moveCursor(int _x, int _y)
{
    wmove(m_mainWindow, _y, _x);
    
}

//---------------------------------------------------------------------------------------
void Ncurses_Impl::printBufferLine(int _cx, int _cy, char* _line)
{
    mvwprintw(m_mainWindow, _cy, _cx, "%s", _line);

}


