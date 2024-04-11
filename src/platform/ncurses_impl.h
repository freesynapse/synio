#ifndef __NCURSES_IMPL_H
#define __NCURSES_IMPL_H

#include <ncurses.h>
#include <unordered_map>
#include <vector>

#include "platform_impl.h"
#include "../types.h"


//
class Ncurses_Impl : public Platform_Impl
{
public:
    Ncurses_Impl() {}
    ~Ncurses_Impl() {}
    //
    virtual int initialize() override;
    virtual int shutdown() override;
    virtual void getRenderSize(ivec2_t *_v) override;
    virtual API_WINDOW_PTR newWindow(frame_t *_frame) override;
    virtual API_WINDOW_PTR newBorderWindow(frame_t *_frame) override;
    virtual void deleteWindow(API_WINDOW_PTR _w) override;

    virtual API_WINDOW_PTR newVerticalBarWindow(int _x, int _y0, int _y1) override;

    //
    virtual void clearScreen() override     { wclear((WINDOW *)m_screenPtr);   }
    virtual void refreshScreen() override   { wrefresh((WINDOW *)m_screenPtr); }
    virtual void clearWindow(API_WINDOW_PTR _w) override     { wclear((WINDOW *)_w);   }
    // TODO : rename current refreshWindow to updateWindow and keep refreshWindow below
    // virtual void refreshWindow(API_WINDOW_PTR _w) override   { wrefresh((WINDOW *)_w); }
    virtual void refreshWindow(API_WINDOW_PTR _w) override   { wnoutrefresh((WINDOW *)_w); }
    virtual void redrawScreen() override { doupdate(); }
    virtual void refreshBorder(API_WINDOW_PTR _w) override;

    //
    virtual int getKey() override;
    virtual CtrlKeyAction getCtrlKeyAction(int _key) override;
    virtual int moveCursor(API_WINDOW_PTR _w, int _x, int _y) override;
    virtual int clearBufferLine(API_WINDOW_PTR _w, int _cy, int _win_maxx) override;
    virtual int clearSpace(API_WINDOW_PTR _w, int _cx, int _cy, int _n) override;
    virtual int printBufferLine(API_WINDOW_PTR _w, int _cx, int _cy, CHTYPE_PTR _line, size_t _len) override;
    virtual int printString(API_WINDOW_PTR _w, int _cx, int _cy, const CHTYPE_PTR _str, size_t _len) override;
    virtual int wprint(API_WINDOW_PTR _w, int _cx, int _cy, const char *_fmt, ...) override;

    // ncurses-only functions and variables
    void initKeycodeList();
    void setCtrlKeycodes();

private:
    std::unordered_map<int, CtrlKeyAction> m_ctrlKeyActionMap;
    std::vector<ctrl_keycode_t> m_ctrlKeycodesList;


};

#endif // __NCURSES_IMPL_H
