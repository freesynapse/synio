
#include <locale.h>

#include "ncurses_impl.h"
#include "ncurses_colors.h"
#include "../core.h"
#include "../utils/utils.h"
#include "../utils/log.h"
#include "../config.h"

//
int Ncurses_Impl::initialize()
{
    setlocale(LC_ALL, "");

    m_screenPtr = initscr();

    ncurses_init_colors(m_screenPtr);

    TABSIZE = Config::TAB_SIZE;

	// cbreak();
    raw();
	keypad(stdscr, TRUE);
	noecho();
	clear();
    refresh();

    // control characters (defined in termifo for current $TERM, eg TERM=xterm-256color)
    // NOTE : kitty have its own terminfo, kitty-xterm, not compliant with ncurses control
    //        codes. Hence, extra cursor movement flavour will not work in kitty...
    initKeycodeList();
    setCtrlKeycodes();

    LOG_INFO("ncurses initialized.");

    return RETURN_SUCCESS;

}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::shutdown()
{
    echo();
    noraw();
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
API_WINDOW_PTR Ncurses_Impl::newWindow(frame_t *_frame)
{
    WINDOW *win = newwin(_frame->nrows, _frame->ncols, _frame->v0.y, _frame->v0.x);
    wrefresh(win);
    return (API_WINDOW_PTR)win;

}

//---------------------------------------------------------------------------------------
API_WINDOW_PTR Ncurses_Impl::newBorderWindow(frame_t *_frame)
{
    // takes the original window frame as argument, so have to check that within bounds
    frame_t new_frame = frame_t(ivec2_t(_frame->v0.x - 1, _frame->v0.y - 1),
                                ivec2_t(_frame->v1.x + 1, _frame->v1.y + 1));
    ivec2_t screen_dim;
    getRenderSize(&screen_dim);
    if (new_frame.v0.x < 0 || new_frame.v0.y < 0 ||
        new_frame.v1.x >= screen_dim.x || new_frame.v1.y >= screen_dim.y)
        LOG_WARNING("window border out of bounds");

    if (new_frame.v0.x < 0) new_frame.v0.x = 0;
    if (new_frame.v0.y < 0) new_frame.v0.y = 0;
    if (new_frame.v1.x >= screen_dim.x) new_frame.v1.x = screen_dim.x - 1;
    if (new_frame.v1.y >= screen_dim.y) new_frame.v1.y = screen_dim.y - 1;

    WINDOW *win = newwin(new_frame.nrows, new_frame.ncols, 
                         new_frame.v0.y, new_frame.v0.x);

    wborder(win, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,
                 ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);

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
API_WINDOW_PTR Ncurses_Impl::newVerticalBarWindow(int _x, int _y0, int _y1)
{
    frame_t frame = frame_t(ivec2_t(_x, _y0), ivec2_t(_x + 10, _y1));
    
    ivec2_t screen_dim;
    getRenderSize(&screen_dim);
    if (frame.v0.x < 0 || frame.v0.y < 0 ||
        frame.v1.x >= screen_dim.x || frame.v1.y >= screen_dim.y)
        LOG_WARNING("window border out of bounds");

    if (frame.v0.x < 0) frame.v0.x = 0;
    if (frame.v0.y < 0) frame.v0.y = 0;
    if (frame.v1.x >= screen_dim.x) frame.v1.x = screen_dim.x - 1;
    if (frame.v1.y >= screen_dim.y) frame.v1.y = screen_dim.y - 1;

    WINDOW *win = newwin(frame.nrows, frame.ncols, frame.v0.y, frame.v0.x);
    wborder(win, ACS_VLINE, ACS_VLINE, ACS_VLINE, ACS_VLINE, 
                 ACS_VLINE, ACS_VLINE, ACS_VLINE, ACS_VLINE);
    
    wrefresh(win);

    return win;

}

//---------------------------------------------------------------------------------------
void Ncurses_Impl::refreshBorder(API_WINDOW_PTR _w)
{
    wborder((WINDOW *)_w, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,
                          ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
    wrefresh((WINDOW *)_w);

}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::getKey()
{
    return wgetch((WINDOW *)m_screenPtr);

}

//---------------------------------------------------------------------------------------
CtrlKeyAction Ncurses_Impl::getCtrlKeyAction(int _key)
{
    if (m_ctrlKeyActionMap.find(_key) == m_ctrlKeyActionMap.end())
        return CtrlKeyAction::NONE;
    
    return m_ctrlKeyActionMap[_key];
    
}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::moveCursor(API_WINDOW_PTR _w, int _x, int _y)
{
    int ret = wmove((WINDOW *)_w, _y, _x);
    // wrefresh((WINDOW *)_w);

    return ret;

}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::clearBufferLine(API_WINDOW_PTR _w, int _cy, int _win_maxx)
{
    memset(m_clearBuffer, ' ', _win_maxx);
    m_clearBuffer[_win_maxx] = 0;
    return mvwaddnstr((WINDOW *)_w, _cy, 0, m_clearBuffer, _win_maxx);

}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::clearSpace(API_WINDOW_PTR _w, int _cx, int _cy, int _n)
{
    memset(m_clearBuffer, ' ', _n);
    m_clearBuffer[_n] = 0;
    return mvwaddnstr((WINDOW *)_w, _cy, _cx, m_clearBuffer, _n);

}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::printBufferLine(API_WINDOW_PTR _w, int _cx, int _cy, CHTYPE_PTR _line, size_t _len)
{
    // since mvwaddchstr can't handle '\t', let's implement our own...
    WINDOW *w = (WINDOW *)_w;
    wmove(w, _cy, _cx);

    int x = _cx;
    for (size_t i = 0; i < _len; i++)
    {
        if ((_line[i] & A_CHARTEXT) == '\t')
        {
            x = (x + (Config::TAB_SIZE - (x % Config::TAB_SIZE)));
            waddch(w, '|' | A_BOLD);
            wmove(w, _cy, x);
        }
        else
            waddch(w, _line[i]);

        x++;
    }

    return RETURN_SUCCESS;
}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::printString(API_WINDOW_PTR _w, int _cx, int _cy, CHTYPE_PTR _str, size_t _len)
{
    WINDOW *w = (WINDOW *)_w;
    wmove(w, _cy, _cx);
    // waddchstr(w, (const chtype *)_str);
    for (size_t i = 0; i < _len; i++)
        waddch(w, _str[i]);

    return RETURN_SUCCESS;
}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::wprint(API_WINDOW_PTR _w, int _cx, int _cy, const char *_fmt, ...)
{
    char buffer[128] = { 0 };
    va_list arg_list;
    va_start(arg_list, _fmt);
    vsnprintf(buffer, 128, _fmt, arg_list);
    va_end(arg_list);

    int len = mvwprintw((WINDOW *)_w, _cy, _cx, "%s", buffer);
    return len;

}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::wprintml(API_WINDOW_PTR _w, int _cx0, int _cy0, 
                           const std::vector<std::string> &_ml_buffer)
{
    int len = 0;
    int y = _cy0;
    for (size_t i = 0; i < _ml_buffer.size(); i++)
    {
        if (i == 0)
            mvwprintw((WINDOW *)_w, y, _cx0, "%s", _ml_buffer[i].c_str());
        else
            mvwprintw((WINDOW *)_w, y, 0, "%s", _ml_buffer[i].c_str());
        y++;
    }

    return len;
}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::verticalDivider(API_WINDOW_PTR _w, int _cx, int _cy, int _n)
{
    mvwvline((WINDOW *)_w, _cy, _cx, ACS_VLINE, _n);
    return RETURN_SUCCESS;
}

//---------------------------------------------------------------------------------------
int Ncurses_Impl::horizontalDivider(API_WINDOW_PTR _w, int _cx, int _cy, int _n)
{
    mvwhline((WINDOW *)_w, _cy, _cx, ACS_HLINE, _n);
    return RETURN_SUCCESS;

}

//---------------------------------------------------------------------------------------
void Ncurses_Impl::initKeycodeList()
{
    m_ctrlKeycodesList = {
        ctrl_keycode_t("kLFT5", CtrlKeyAction::CTRL_LEFT),
        ctrl_keycode_t("kRIT5", CtrlKeyAction::CTRL_RIGHT),
        ctrl_keycode_t("kUP5",  CtrlKeyAction::CTRL_UP),
        ctrl_keycode_t("kDN5",  CtrlKeyAction::CTRL_DOWN),
        ctrl_keycode_t("kHOM5", CtrlKeyAction::CTRL_HOME),
        ctrl_keycode_t("kEND5", CtrlKeyAction::CTRL_END),
        ctrl_keycode_t("kDC5",  CtrlKeyAction::CTRL_DEL),
        ctrl_keycode_t("kDC6",  CtrlKeyAction::CTRL_SHIFT_DELETE),
        ctrl_keycode_t("kUP",   CtrlKeyAction::SHIFT_UP),
        ctrl_keycode_t("kDN",   CtrlKeyAction::SHIFT_DOWN),
        ctrl_keycode_t("kLFT6", CtrlKeyAction::SHIFT_CTRL_LEFT),
        ctrl_keycode_t("kRIT6", CtrlKeyAction::SHIFT_CTRL_RIGHT),
        ctrl_keycode_t("kUP6",  CtrlKeyAction::SHIFT_CTRL_UP),
        ctrl_keycode_t("kDN6",  CtrlKeyAction::SHIFT_CTRL_DOWN),
        ctrl_keycode_t("kHOM6", CtrlKeyAction::SHIFT_CTRL_HOME),
        ctrl_keycode_t("kEND6", CtrlKeyAction::SHIFT_CTRL_END),
        ctrl_keycode_t("kLFT3", CtrlKeyAction::ALT_LEFT),
        ctrl_keycode_t("kRIT3", CtrlKeyAction::ALT_RIGHT),
        ctrl_keycode_t("kUP3",  CtrlKeyAction::ALT_UP),
        ctrl_keycode_t("kDN3",  CtrlKeyAction::ALT_DOWN),
        ctrl_keycode_t("kPRV3", CtrlKeyAction::ALT_PAGEUP),
        ctrl_keycode_t("kNXT3", CtrlKeyAction::ALT_PAGEDOWN),
        ctrl_keycode_t("kIC3",  CtrlKeyAction::ALT_INSERT),
        ctrl_keycode_t("kDC3",  CtrlKeyAction::ALT_DELETE),
        ctrl_keycode_t("kLFT4", CtrlKeyAction::SHIFT_ALT_LEFT),
        ctrl_keycode_t("kRIT4", CtrlKeyAction::SHIFT_ALT_RIGHT),
        ctrl_keycode_t("kUP4",  CtrlKeyAction::SHIFT_ALT_UP),
        ctrl_keycode_t("kDN4",  CtrlKeyAction::SHIFT_ALT_DOWN),

    };

}

//---------------------------------------------------------------------------------------
void Ncurses_Impl::setCtrlKeycodes()
{
    for (auto &key : m_ctrlKeycodesList)
    {
        const char *keyvalue = tigetstr(key.id.c_str());
        if (keyvalue != 0 && keyvalue != (char *)-1 && key_defined(keyvalue))
            m_ctrlKeyActionMap[key_defined(keyvalue)] = key.action;
        else
        {
            LOG_ERROR("Control keycode %s (%s) not set.", 
                      key.id.c_str(), ctrlActionStr(key.action));
        }
    }

}
