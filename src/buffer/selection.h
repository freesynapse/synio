#ifndef __SELECTION_H
#define __SELECTION_H

#include "../types.h"


//
class Selection
{
public:
    Selection() {}
    Selection(line_t *_start_line, size_t _offset) :
        m_startLine(_start_line), m_endLine(_start_line), m_startOffset(_offset)
    {}
    ~Selection() = default;

    // TODO : update pointers and offsets based on which is first? Possible w linked list?
    void update_end_line(line_t *_line)
    { 
        m_endLine = _line; 
        select_deselect_sequence_();
    }
    
    void update_end_line(line_t *_line, size_t _offset)
    { 
        m_endLine = _line; 
        m_endOffset = _offset; 
        select_deselect_sequence_();
    }
    
    void update_end_offset(size_t _offset)
    { 
        m_endOffset = _offset; 
        select_deselect_sequence_();
    }

    //void clear_sequence() -- TODO : clear a certain sequence from selection
    // clears all characters in the selection from being selected
    void clear_all();


    //
    #ifdef DEBUG
    void __debug_selection()
    {
        LOG_INFO("selection: line %p (%zu) -> %p (%zu)",
                 m_startLine,
                 m_startOffset,
                 m_endLine,
                 m_endOffset);
    };
    #endif


// private:
    // updates a sequence to being selected.
    // TODO : only update new characters
    void select_deselect_sequence_(bool _selecting=true);


// private:
    // member variables
    line_t *m_startLine = NULL;
    size_t m_startOffset = 0;
    
    line_t *m_endLine = NULL;
    size_t m_endOffset = 0;

};










#endif // __SELECTION_H
