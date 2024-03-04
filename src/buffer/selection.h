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

    // clears all characters in the selection from being selected
    void clear();

    void add(line_t *_start_line, int _offset, int _n);
    // void remove? how to step through? linear search or hash map?


// private:
    void select_deselect_(selection_entry_t *_entry, int _selecting);

// private:
    // key = line ptr addr + offset
    // std::unordered_map<int, selection_entry_t> m_entries;
    std::vector<selection_entry_t> m_entries;

};










#endif // __SELECTION_H
