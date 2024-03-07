#ifndef __SELECTION_H
#define __SELECTION_H

// TODO : choose one or the other!
#include <unordered_map>
#include <vector>

#include "../types.h"


//
// TODO :
// some kind of hash is needed -> map<hash, selection_entry_t> entries - this way, the same
// text is not selected twice, maybe we could even deselect based on this?!
// 
// SOLUTION:
// Instead of using a 4 byte number per character (ie. CHTYPE), we use 64 bits (uint64_t)
// and use one of the high bits as a selection bit.
//

struct selection_entry_t
{
    line_t *line = NULL;
    int offset = 0;
    int nchars = 0;

    selection_entry_t() {}
    selection_entry_t(line_t *_line, int _offset, int _n) :
        line(_line), offset(_offset), nchars(_n)
    {}

};

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
    void select_deselect_(const selection_entry_t &_entry, int _selecting);
    // overload
    void select_deselect_(line_t *_line, int _offset, int _n, int _selecting)
    { select_deselect_(selection_entry_t(_line, _offset, _n), _selecting); }
    
    // selection_entry_t helpers
    __always_inline uint64_t hash_line_(line_t *_line, int _offset)
    {
        // Is this always unique? No. Does it matter? Not sure...
        return (uint64_t)(_line + _offset);
    }
    __always_inline void save_selection_entry_(const selection_entry_t &_entry)
    {
        uint64_t key = hash_line_(_entry.line, _entry.offset);
        if (m_entries.find(key) == m_entries.end())
            m_entries[key] = _entry;
    }
    
// private:
    // key = line ptr addr + offset (for now)
    std::unordered_map<uint64_t, selection_entry_t> m_entries;

};










#endif // __SELECTION_H
