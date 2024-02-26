#ifndef __COMMAND_WINDOW_H
#define __COMMAND_WINDOW_H

#include "window.h"
#include "buffer_window.h"

//
class Command
{
public:
    Command() {}
    ~Command() = default;

};

//
class CommandWindow : public Window
{
public:
    CommandWindow(const frame_t &_frame, const std::string &_id, bool _border=false);
    ~CommandWindow();

    virtual void resize(frame_t _new_frame) override;

    virtual void redraw() override;

    //
    void setBuffer(Buffer *_buffer) { m_currentBuffer = _buffer; }


private:
    Command m_lastCommand;
    Buffer *m_currentBuffer = NULL;

    bool m_enteringCommand = false;

    //
    CHTYPE_PTR m_statusChStr = NULL;

};






#endif // __COMMAND_WINDOW_H
