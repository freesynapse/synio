
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "line_buffer.h"
#include "utils/log.h"

//
line_t *create_line(char *_content, size_t _len)
{
    line_t *new_line = (line_t *)malloc(sizeof(line_t));
    new_line->next = NULL;
    new_line->prev = NULL;
    new_line->content = (char *)malloc(_len+1);
    new_line->len = _len;
    memset(new_line->content, 0, _len+1);
    memcpy(new_line->content, _content, _len);

    return new_line;

}

//---------------------------------------------------------------------------------------
void line_t::__debug_print(bool _show_ptrs, const char *_str)
{
    if (strcmp(_str, "") != 0)
        LOG_INFO("%s", _str);
    LOG_INFO("%p: [%s]", this, content);
    if (_show_ptrs)
    {
        LOG_INFO("    next: %s", next == NULL ? "NULL" : next->content);
        LOG_INFO("    prev: %s", prev == NULL ? "NULL" : prev->content);
    }

}

//---------------------------------------------------------------------------------------
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
void LineBuffer::insertAtPtr(line_t *_at_line, int _insert_flag, const char *_content)
{
    // _insert_flag is either INSERT_BEFORE or INSERT_AFTER

    line_t *new_line = create_line((char*)_content, strlen(_content));

    //
    if (m_head == NULL || _at_line == NULL)
        return;

    // before first line
    if ((_at_line == m_head) && (_insert_flag & INSERT_BEFORE))
    {
        push_front(new_line);

    }

    // after tail
    else if ((_at_line == m_tail) && (_insert_flag & INSERT_AFTER))
    {
        push_back(new_line);

    }

    // in the middle, after head or before tail
    else
    {
        switch (_insert_flag)
        {
            case INSERT_BEFORE:
                new_line->prev = _at_line->prev;
                new_line->next = _at_line;
                _at_line->prev->next = new_line;
                _at_line->prev = new_line;
                break;

            case INSERT_AFTER:
                new_line->prev = _at_line;
                new_line->next = _at_line->next;
                _at_line->next->prev = new_line;
                _at_line->next = new_line;
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
    #ifdef DEBUG
    if (_line) _line->__debug_print(true, __func__);
    #endif
    
    // empty
    if (m_head == NULL || _line == NULL)
        return;

    // first line
    else if (_line == m_head)
    {
        // basically deletes the list
        if (m_head->next == NULL)
        {
            free(_line);
            m_head = NULL;
            m_tail = NULL;
        }
        // delete and update m_head
        else
        {
            m_head = m_head->next;
            free(_line);
        }

    }

    // last line
    else if (_line == m_tail)
    {
        m_tail->prev->next = NULL;
        m_tail = m_tail->prev;
        free(_line);
    }

    // otherwise, somewhere in the middle (i.e. line count > 2)
    else
    {
        _line->prev->next = _line->next;
        _line->next->prev = _line->prev;
        free(_line);
    }

    m_lineCount--;

}

//---------------------------------------------------------------------------------------
line_t *LineBuffer::ptrFromIdx(int _index)
{
    /* TODO (?):
     * Could chunk this every eg 500 LineBuffer, so that we start the search either forwards 
     * backwards from the nearest chunk boundary. Problem: we need to update the chunks
     * periodically, which I guess could be done threaded.
     * 
     * For now, let's start from the back if that is closer.
     * 
     */

    // trivial cases
    if (m_lineCount == 0) 
        return NULL;
    else if (m_lineCount == 1)
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
        LOG_INFO("[%2zu] %p: %s %s", n, p, p->content, ht);
        LOG_INFO("        next: %s", p->next == NULL ? "NULL" : p->next->content);
        LOG_INFO("        prev: %s", p->prev == NULL ? "NULL" : p->prev->content);
        
        p = p->next;
        n++;

    }

}

//---------------------------------------------------------------------------------------
void LineBuffer::__debug_print()
{
    line_t *p = m_head;
    while (p != NULL)
    {
        printf("%s\n", p->content);
        p = p->next;

    }
    
    printf("\n");

}