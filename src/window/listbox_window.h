#ifndef __LISTBOX_WINDOW_H
#define __LISTBOX_WINDOW_H

#include "file_buffer_window.h"
#include "../utils/prefix_tree.h"
#include "../types.h"


//
class ListboxWindow : public FileBufferWindow
{
public:
    ListboxWindow(const frame_t &_frame, 
                  const std::string &_id, 
                  bool _border=true,
                  const std::string &_header="", 
                  const std::vector<listbox_entry_t> &_entries={});
    ~ListboxWindow();

    // We only need to override some of the functionality of the LineBufferWindow for this.
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;
    virtual void redraw() override;
    virtual void moveCursor(int _dx, int _dy) override;

    // ListbowWindow-specific functions
    void pushCharToInput(char _c);
    void popCharFromInput();
    void changeSelectedRow(int _dy=0);
    void autocompleteInput();
    void findCompletions();

    // accessors
    const std::string &getEntry() { return m_selectedEntry; }

private:
    int prev_entry_() { return ((m_highlightedEntry - 1) + m_lineBuffer.size()) % m_lineBuffer.size(); }
    int next_entry_() { return (m_highlightedEntry + 1) % m_lineBuffer.size(); }

    //
    __always_inline void clear_input_()
    {
        if (m_inputLine->len == 0)
            return;
        
        delete m_inputLine;
        m_inputLine = create_line("");
    }

private:
    std::string m_selectedEntry = "";   // returned to parent caller
    int m_highlightedEntry = 0;         // currently highlighted entry, selected with <UP>/<DOWN>
    frame_t m_renderFrame = frame_t(0);

    std::vector<std::string> m_keys;    // internal representation, different from the listing
    std::vector<std::string> m_values;  // if listbox displays files, this is the searchable entries

    prefix_node_t *m_ptree = NULL;
    std::string m_autocompleteLine = "";
    std::vector<std::string> m_autocompletions;

    std::string m_header = "";
    line_t *m_inputLine;            // m_currentLine is pointing into the line buffer,
                                    // this is for keyboard input (e.g. filename)

};


#endif // __LISTBOX_WINDOW_H
