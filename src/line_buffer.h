#ifndef __LINE_BUFFER_H
#define __LINE_BUFFER_H

#include <stdlib.h>
#include <string.h>

// insert flags
#define INSERT_BEFORE   0x01
#define INSERT_AFTER    0x10

//
typedef struct line_t line_t;
struct line_t
{
    line_t *next  = NULL;
    line_t *prev  = NULL;
    char *content = NULL;

    ~line_t() { free(content); }
    void __debug_print(bool _show_ptrs=true, const char *_str="");
    
};

//
line_t *create_line(char *_content, size_t _len);
// line_t *create_line(const char *_content) { create_line((char*)_content, strlen(_content)); }

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
    void insertAtPtr(line_t *_at_line, int _insert_flag, const char *_content);
    void insertBeforeIdx(int _index, const char *_content)  { insertAtPtr(ptrFromIdx(_index), INSERT_BEFORE, _content); }
    void insertAfterIdx(int _index, const char *_content)   { insertAtPtr(ptrFromIdx(_index), INSERT_AFTER, _content);  }
    void deleteAtPtr(line_t *_line);
    void deleteBeforePtr(line_t *_line)     { deleteAtPtr(_line->prev);             }
    void deleteAfterPtr(line_t *_line)      { deleteAtPtr(_line->next);             }
    void deleteAtIdx(int _index)            { deleteAtPtr(ptrFromIdx(_index));      }
    void deleteBeforeIdx(int _index)        { deleteAtPtr(ptrFromIdx(_index - 1));  }
    void deleteAfterIdx(int _index)         { deleteAtPtr(ptrFromIdx(_index + 1));  }
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



