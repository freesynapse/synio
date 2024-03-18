
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "line_buffer.h"
#include "../types.h"
#include "../utils/log.h"

//
void LineBuffer::push_front(line_t *_new_line)
{
    if (m_head == NULL)
    {
        m_head = _new_line;
        m_tail = _new_line;
    }
    else
    {
        m_head->prev = _new_line;
        _new_line->next = m_head;
        _new_line->prev = NULL;
        m_head = _new_line;

    }

    m_lineCount++;

}

//---------------------------------------------------------------------------------------
void LineBuffer::push_back(line_t *_new_line)
{
    if (m_head == NULL)
    {
        m_head = _new_line;
        m_tail = _new_line;
    }
    else
    {        
        m_tail->next = _new_line;
        _new_line->prev = m_tail;
        m_tail = _new_line;

    }

    m_lineCount++;

}

//---------------------------------------------------------------------------------------
void LineBuffer::insertAtPtr(line_t *_at_line, int _insert_flag, line_t *_new_line)
{
    // _insert_flag is either INSERT_BEFORE or INSERT_AFTER

    //
    if (m_head == NULL || _at_line == NULL)
        return;

    // before first line
    if ((_at_line == m_head) && (_insert_flag & INSERT_BEFORE))
    {
        push_front(_new_line);

    }

    // after tail
    else if ((_at_line == m_tail) && (_insert_flag & INSERT_AFTER))
    {
        push_back(_new_line);

    }

    // in the middle, after head or before tail
    else
    {
        switch (_insert_flag)
        {
            case INSERT_BEFORE:
                _new_line->prev = _at_line->prev;
                _new_line->next = _at_line;
                _at_line->prev->next = _new_line;
                _at_line->prev = _new_line;
                break;

            case INSERT_AFTER:
                _new_line->prev = _at_line;
                _new_line->next = _at_line->next;
                _at_line->next->prev = _new_line;
                _at_line->next = _new_line;
                break;

            default:
                LOG_WARNING("unknown insert flag %d.", _insert_flag);
                break;
        }

        m_lineCount++;

    }

}

//---------------------------------------------------------------------------------------
void LineBuffer::deleteAtPtr(line_t *_line)
{
    // empty
    if (m_head == NULL || _line == NULL)
        return;

    // first line
    else if (_line == m_head)
    {
        // basically deletes the list
        if (m_head->next == NULL)
        {
            m_head = NULL;
            m_tail = NULL;
            free(_line);
            _line = NULL;
        }
        // delete and update m_head
        else
        {
            m_head = m_head->next;
            m_head->prev = NULL;
            free(_line);
            _line = NULL;
        }

    }

    // last line
    else if (_line == m_tail)
    {
        m_tail->prev->next = NULL;
        m_tail = m_tail->prev;
        free(_line);
        _line = NULL;
    }

    // otherwise, somewhere in the middle (i.e. line count > 2)
    else
    {
        _line->prev->next = _line->next;
        _line->next->prev = _line->prev;
        free(_line);
        _line = NULL;
    }

    m_lineCount--;

}

//---------------------------------------------------------------------------------------
line_t *LineBuffer::appendThisToPrev(line_t *_line)
{
    if (_line->prev == NULL)
        return _line;
    
    line_t *prev = _line->prev;
    // realloc to fit both strings
    size_t alloc_sz = CHTYPE_SIZE *  (prev->len + _line->len + 1);
    if ((prev->content = (CHTYPE_PTR)realloc(prev->content, alloc_sz)) == NULL)
        RAM_panic(prev);
    // copy this to prev
    memcpy(prev->content + prev->len, _line->content, CHTYPE_SIZE * _line->len);
    // update len of prev
    prev->len +=_line->len;
    prev->content[prev->len] = 0;

    deleteAtPtr(_line);

    #ifdef DEBUG
    prev->__debug_content_to_str_();
    #endif

    // return the updated 'this line' (which is now this->prev)
    return prev;

}

//---------------------------------------------------------------------------------------
void LineBuffer::appendNextToThis(line_t *_line)
{
    if (_line->next == NULL)
        return;

    line_t *next = _line->next;
    // realloc to fit both strings
    size_t alloc_sz = CHTYPE_SIZE *  (_line->len + next->len + 1);
    if ((_line->content = (CHTYPE_PTR)realloc(_line->content, alloc_sz)) == NULL) 
        RAM_panic(_line);
    // copy next to this
    memcpy(_line->content+_line->len, next->content, CHTYPE_SIZE * next->len);
    // update len
    _line->len = _line->len + next->len;
    _line->content[_line->len] = 0;

    #ifdef DEBUG
    _line->__debug_content_to_str_();
    #endif

    deleteAtPtr(next);

}

//---------------------------------------------------------------------------------------
line_t *LineBuffer::ptrFromIdx(int _index)
{
    /* TODO (?):
     * Could chunk this every eg 500 lines, so that we start the search either forwards 
     * backwards from the nearest chunk boundary. Problem: we need to update the chunks
     * periodically, which I guess could be done threaded.
     * 
     * For now, let's start from the back if that is closer.
     * 
     */

    // trivial cases
    if (m_lineCount == 0) 
        return NULL;
    if (m_lineCount == 1)
        return m_head;

    // out-of bounds indexing
    if (_index < 0 || _index > m_lineCount - 1)
        return NULL;

    line_t *ret = NULL;
    line_t *p = m_head;
    int i;

    // 'smart' search
    //
    int halfway = m_lineCount / 2;
    
    // backward search
    if (_index > halfway)
    {
        i = m_lineCount - 1;
        for (p = m_tail; p != NULL; p = p->prev, i--)
        {
            if (_index == i)
            {
                ret = p;
                break;
            }
        }
    }

    // forward search
    else
    {
        i = 0;
        for (p = m_head; p != NULL; p = p->next, i++)
        {
            if (_index == i)
            {
                ret = p;
                break;
            }
        }
    }
    
    return ret;

}

//---------------------------------------------------------------------------------------
int LineBuffer::idxFromPtr(line_t *_line)
{
    if (m_head == NULL)
        return -1;

    // have to do linear search, unfortunately
    line_t *p = m_head;
    int idx = -1;
    int i = 0;
    while (p->next != NULL)
    {
        if (p == _line)
        {
            idx = i;
            break;
        }
        
        i++;
        p = p->next;

    }

    return idx;

}

//---------------------------------------------------------------------------------------
void LineBuffer::clear()
{
    if (m_head == NULL)
        return;

    line_t *p = m_head;
    if (p->next == NULL)
    {
        free(p);
        return;
    }

    else
    {
        line_t *p1 = p->next;
        while(p1 != NULL)
        {
            free(p);
            p = p1;
            p1 = p1->next;
        }

        free(p);
        
    }

    m_lineCount = 0;
    m_head = NULL;
    m_tail = NULL;

}

//---------------------------------------------------------------------------------------
#ifdef DEBUG
void LineBuffer::__debug_inspect()
{
    printf("== __DEBUG_DUMP() =========================\n");
    LOG_INFO("%d line(s)", m_lineCount);

    //
    line_t *p = m_head;
    size_t n = 0;
    while (p != NULL)
    {
        char ht[7] = { 0 };
        if      (p == m_head)   sprintf(ht, "(HEAD)");
        else if (p == m_tail)   sprintf(ht, "(TAIL)");
        LOG_INFO("[%2zu] %p: %s %s", n, p, p->__debug_str, ht);
        LOG_INFO("        next: %s", p->next == NULL ? "NULL" : p->next->__debug_str);
        LOG_INFO("        prev: %s", p->prev == NULL ? "NULL" : p->prev->__debug_str);
        
        p = p->next;
        n++;

    }

}
#endif

//---------------------------------------------------------------------------------------
#ifdef DEBUG
void LineBuffer::__debug_print()
{
    line_t *p = m_head;
    while (p != NULL)
    {
        printf("%s\n", p->__debug_str);
        p = p->next;

    }
    
    printf("\n");

}
#endif