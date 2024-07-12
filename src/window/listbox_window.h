#ifndef __LISTBOX_WINDOW_H
#define __LISTBOX_WINDOW_H

#include "mline_input_window.h"
#include "../utils/prefix_tree.h"
#include "../types.h"


//
class ListboxWindow : public MLineInputWindow
{
public:
    ListboxWindow(const frame_t &_frame, 
                  const std::string &_id, 
                  bool _border=true,
                  const std::string &_header="", 
                  const std::vector<listbox_entry_t> &_entries={});
    ~ListboxWindow() = default;

    // overrides
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;
    virtual void redraw() override;
    virtual void moveCursor(int _dx, int _dy) override;
    virtual void findCompletions() override;

    // ListbowWindow-specific functions
    void changeSelectedRow(int _dy=0);

    // accessors
    const std::string &getEntry() { return m_selectedEntry; }

private:
    int prev_entry_() { return ((m_highlightedEntry - 1) + m_lineBuffer.size()) % m_lineBuffer.size(); }
    int next_entry_() { return (m_highlightedEntry + 1) % m_lineBuffer.size(); }

private:
    std::string m_selectedEntry = "";   // returned to parent caller
    int m_highlightedEntry = 0;         // currently highlighted entry, selected with <UP>/<DOWN>

    std::vector<std::string> m_keys;    // internal representation, different from the listing
    std::vector<std::string> m_values;  // if listbox displays files, this is the searchable entries

};


#endif // __LISTBOX_WINDOW_H
