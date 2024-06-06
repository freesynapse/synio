#ifndef __LISTBOX_WINDOW_H
#define __LISTBOX_WINDOW_H

#include "line_buffer_window.h"

//
class ListboxWindow : public LineBufferWindow
{
public:
    ListboxWindow(const frame_t &_frame, const std::string &_id, bool _border=true);
    ~ListboxWindow() = default;

    // We only need to override some of the functionality of the LineBufferWindow for this.
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;
    virtual void redraw() override;

    // on <ENTER> in handleInput() this function will return the selected string, 
    // signaling to the parent window that selection is complete.
    //
    std::string getSelectedEntry();

private:
    int prev_entry_() { return ((m_currentEntryIdx - 1) + m_entries.size()) % m_entries.size(); }
    int next_entry_() { return (m_currentEntryIdx + 1) % m_entries.size(); }

private:
    std::vector<std::string> m_entries;
    int m_selectedEntryIdx = -1;    // returned to parent upon <ENTER>
    int m_currentEntryIdx = 0;      // 
};




#endif // __LISTBOX_WINDOW_H
