#ifndef __LISTBOX_WINDOW_H
#define __LISTBOX_WINDOW_H

#include "file_buffer_window.h"

//
class ListboxWindow : public FileBufferWindow
{
public:
    ListboxWindow(const frame_t &_frame, const std::string &_id, bool _border=true,
                  const std::string &_header="", std::vector<std::string> _entries={});
    ~ListboxWindow() = default;

    // We only need to override some of the functionality of the LineBufferWindow for this.
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;
    virtual void redraw() override;

    // on <ENTER> in handleInput() this function will return the selected string, 
    // signaling to the parent window that selection is complete.
    //
    std::string getSelectedEntry();

private:
    int prev_entry_() { return ((m_highlightedEntry - 1) + m_lineBuffer.size()) % m_lineBuffer.size(); }
    int next_entry_() { return (m_highlightedEntry + 1) % m_lineBuffer.size(); }

private:
    int m_selectedEntryIdx = -1;    // returned to parent upon <ENTER>
    int m_highlightedEntry = 0;     // currently highlighted entry, selected with <UP>/<DOWN>
    frame_t m_renderFrame = frame_t(0);

    std::string m_header = "";

};




#endif // __LISTBOX_WINDOW_H
