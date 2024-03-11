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

    // TODO : remove this?
    void selectChars(line_t *_start_line, int _offset, int _n);
    //
    void selectLineChars(line_t *_line, int _offset, int _n);
    void selectLines(line_t *_start_line, int _start_offset, line_t *_end_line, int _end_offset);
    // void remove? how to step through? linear search or hash map? maybe clear() and add()?

    // accessors
    size_t selectionCount() { return m_entries.size(); }
    void setStartingBufferPos(const ivec2_t &_start_pos) { m_startingBufferPos = _start_pos; }
    const ivec2_t &startingBufferPos() { return m_startingBufferPos; }

// private:
    void select_deselect_(line_t *_line, int _offset, int _n, int _selecting);

// private:
    ivec2_t m_startingBufferPos;
    std::unordered_set<line_t *> m_entries;

};




#endif // __SELECTION_H
