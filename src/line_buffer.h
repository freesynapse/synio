#ifndef __LINE_BUFFER_H
#define __LINE_BUFFER_H

#include <stdlib.h>
#include <string.h>

#include "types.h"  // for line_t 

// insert flags
#define INSERT_BEFORE   0x01
#define INSERT_AFTER    0x10

//
// TODO : make templated for _content type?
class LineBuffer
{
public:
    LineBuffer() {}
    ~LineBuffer() { clear(); }

    void push_front(line_t *_new_line);
    void push_back(line_t *_new_line);
    // void push_front(const char *_content)   { push_front(create_line(_content)); }
    // void push_back(const char *_content)    { push_back(create_line(_content));  }
    void insertAtPtr(line_t *_at_line, int _insert_flag, const char *_content) { insertAtPtr(_at_line, _insert_flag, create_line((char *)_content, strlen(_content))); }
    void insertAtPtr(line_t *_at_line, int _insert_flag, line_t *_new_line);
    void insertBeforeIdx(int _index, const char *_content)  { insertAtPtr(ptrFromIdx(_index), INSERT_BEFORE, _content); }
    void insertAfterIdx(int _index, const char *_content)   { insertAtPtr(ptrFromIdx(_index), INSERT_AFTER, _content);  }
    void deleteAtPtr(line_t *_line);
    void deleteBeforePtr(line_t *_line)     { deleteAtPtr(_line->prev);             }
    void deleteAfterPtr(line_t *_line)      { deleteAtPtr(_line->next);             }
    void deleteAtIdx(int _index)            { deleteAtPtr(ptrFromIdx(_index));      }
    void deleteBeforeIdx(int _index)        { deleteAtPtr(ptrFromIdx(_index - 1));  }
    void deleteAfterIdx(int _index)         { deleteAtPtr(ptrFromIdx(_index + 1));  }
    line_t *appendThisToPrev(line_t *_line);   // as when <BACKSPACE> is pressed at start of line
    void appendNextToThis(line_t *_line);      // as when <DEL> is pressed at end of line
    void clear();   // deletes all lines

    //
    void __debug_inspect();
    void __debug_print();

    // accessors
    line_t *ptrFromIdx(int _index);
    const int lineCount() const { return m_lineCount; }
    
    line_t *m_head  = NULL;
    line_t *m_tail  = NULL;
    int m_lineCount = 0;

};


#endif // __LINE_BUFFER_H



