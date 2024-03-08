#ifndef __SELECTION_H
#define __SELECTION_H

#include <unordered_set>

#include "../types.h"

//
class FileBufferWindow;
class Selection
{
public:
    Selection() {}
    ~Selection() = default;

    // clears all characters in the entry from being selected
    void clear();

    void add(line_t *_start_line, int _offset, int _n);

    //Borde vara att selection_entry_t endast är en offset och n tecken, så får man göra allt
    //on the fly, både select och deselect.
    // Seems stupid not to take advantage of the current line pointer (m_currentLine)...
    void selectChars(line_t *_start_line, int _offset, int _n);
    // void remove? how to step through? linear search or hash map? maybe clear() and add()?

    // accessors
    size_t selectionCount() { return m_entries.size(); }


// private:
    void select_deselect_(line_t *_line, int _offset, int _n, int _selecting);

// private:
    std::unordered_set<line_t *> m_entries;

};




#endif // __SELECTION_H
