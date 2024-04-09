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

    void clear();   // clears all entries

    // TODO : threaded implementation for large files?
    
    //
    void selectChars(line_t *_line, size_t _start, size_t _end);
    void selectLines(line_t *_start_line, size_t _start_offset, line_t *_end_line, size_t _end_offset);
    void expandSelection(line_t *_line, size_t _start_expand, size_t _end_expand);

    // accessors
    size_t lineCount(const ivec2_t &_curr_pos);
    void setStartingBufferPos(const ivec2_t &_start_pos) { m_startingBufferPos = _start_pos; }
    const ivec2_t &startingBufferPos() { return m_startingBufferPos; }
    bool isLineEntry(line_t *_line) { return (m_entries.find(_line) != m_entries.end()); }
    const std::unordered_set<line_t *> &getSelectedLines() { return m_entries; }

private:
    void select_(line_t *_line, size_t _start, size_t _end);


private:
    ivec2_t m_startingBufferPos;
    std::unordered_set<line_t *> m_entries;

};




#endif // __SELECTION_H
