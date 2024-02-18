#ifndef __NCURSES_IMPL_H
#define __NCURSES_IMPL_H

#include <ncurses.h>
#include <unordered_map>
#include <vector>

#include "platform_impl.h"
#include "../core.h"
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
    virtual API_WINDOW_PTR newWindow(irect_t *_frame) override;
    virtual API_WINDOW_PTR newBorderWindow(irect_t *_frame) override;
    virtual void deleteWindow(API_WINDOW_PTR _w) override;

    //
    virtual void clearScreen() override     { wclear((WINDOW *)m_screenPtr);   }
    virtual void refreshScreen() override   { wrefresh((WINDOW *)m_screenPtr); }
    virtual void clearWindow(API_WINDOW_PTR _w) override     { wclear((WINDOW *)_w);   }
    virtual void refreshWindow(API_WINDOW_PTR _w) override   { wrefresh((WINDOW *)_w); }
    virtual void refreshBorder(API_WINDOW_PTR _w) override;

    //
    virtual int getKey() override;
    virtual CtrlKeyAction getCtrlKeyAction(int _key) override;
    virtual int moveCursor(API_WINDOW_PTR _w, int _x, int _y) override;
    virtual int printBufferLine(API_WINDOW_PTR _w, int _cx, int _cy, char* _line) override;
    virtual int wprint(API_WINDOW_PTR _w, int _cx, int _cy, const char *_fmt, ...) override;

    // ncurses-only functions and variables
    void initKeycodeList();
    void setCtrlKeycodes();

private:
    std::unordered_map<int, CtrlKeyAction> m_ctrlKeyActionMap;
    std::vector<ctrl_keycode_t> m_ctrlKeycodesList;


};

#endif // __NCURSES_IMPL_H
