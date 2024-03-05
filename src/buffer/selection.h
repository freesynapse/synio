#ifndef __SELECTION_H
#define __SELECTION_H

// TODO : choose one or the other!
#include <unordered_map>
#include <vector>

#include "../types.h"


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
    void selectChars(line_t *_start_line, int _offset, int _n);
    // void remove? how to step through? linear search or hash map? maybe clear() and add()?


// private:
    void select_deselect_(const selection_entry_t &_entry, int _selecting);
    // overload
    void select_deselect_(line_t *_line, int _offset, int _n, int _selecting)
    { select_deselect_(selection_entry_t(_line, _offset, _n), _selecting); }

// private:
    // key = line ptr addr + offset
    // std::unordered_map<int, selection_entry_t> m_entries;
    std::vector<selection_entry_t> m_entries;

};










#endif // __SELECTION_H
