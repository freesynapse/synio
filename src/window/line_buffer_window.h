#ifndef __LINE_BUFFER_WINDOW_H
#define __LINE_BUFFER_WINDOW_H

#include <string>

#include "buffer_window_base.h"


//
class LineBufferWindow : public BufferWindowBase
{
public:
    LineBufferWindow(const frame_t &_frame,
                     const std::string &_id,
                     bool _border=false);
    LineBufferWindow(const frame_t &_frame,
                     const std::string &_id,
                     const std::string &_query,
                     const ivec2_t &_query_pos,
                     bool _border=false);
    ~LineBufferWindow();

    // sets query string before editable line begins
    void setQuery(const std::string &_query, const ivec2_t &_pos);

    // 
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;

    // overrides for editing a single line
    virtual void moveCursor(int _dx, int _dy=0) override;
    virtual void moveCursorToLineBegin() override;
    virtual void moveCursorToLineEnd() override;
    virtual void moveCursorToColDelim(int _dir) override;
    virtual void insertCharAtCursor(char _c) override;
    virtual void insertStrAtCursor(char *_str, size_t _len) override;
    virtual void deleteCharAtCursor() override;
    virtual void deleteCharBeforeCursor() override;

    // update cursor (called during rendering and after keypress)
    virtual void updateCursor() override;

    // dispatch event based on what kind of window this is, called on <ENTER>
    virtual void dispatchEvent();

    // (Window overrides)
    virtual void redraw() override;


protected:
    CHTYPE_PTR m_query = NULL;
    size_t m_queryLen = 0;
    ivec2_t m_queryPos = ivec2_t(0);
    ivec2_t m_lineStart; // where does the editable line start?

};




#endif // __LINE_BUFFER_WINDOW_H
