#ifndef __LINE_NUMBERS_WINDOW_H
#define __LINE_NUMBERS_WINDOW_H

#include "window.h"

//
class FileBufferWindow;
class LineNumbers : public Window
{
public:
    using Window::Window;

    //
    virtual void resize(frame_t _new_frame) override;

    // virtual compulsory functions
    virtual void handleInput(int, CtrlKeyAction) override {}
    virtual void redraw() override;
    virtual void refresh() override;

    // set associated buffer
    void setBuffer(FileBufferWindow *_buffer) { m_associatedBuffer = _buffer; }

    //
    void setWidth(int _w) { m_width = _w; }

private:
    FileBufferWindow *m_associatedBuffer = NULL;
    int m_width = Config::LINE_NUMBERS_MIN_WIDTH;

};



#endif // __LINE_NUMBERS_WINDOW_H
