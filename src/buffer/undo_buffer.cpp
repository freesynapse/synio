
#include "undo_buffer.h"

#include <assert.h>

#include "../window/file_buffer_window.h"


//
void UndoBuffer::push(const undo_item_t &_item)
{
    m_stack.push(_item);

}

//---------------------------------------------------------------------------------------
void UndoBuffer::undo()
{
    assert(m_window != NULL && "no window to undo in..");
    
    if (!m_stack.size())
    {
        #ifdef DEBUG
        LOG_WARNING("undo buffer empty.");
        #endif
        m_window->moveCursor(0, 0);
        return;
    }

    undo_item_t item = m_stack.top();
    m_stack.pop();

    #ifdef DEBUG
    LOG_RAW("stack size = %zu", m_stack.size());
    __debug_print_item(item);
    #endif

    // prevent new commands of being recorded as undoable actions
    m_window->m_storeActions = false;

    //
    switch (item.action)
    {
        case UndoAction::CHAR_DEL:              addCharToLine(item);        break;
        case UndoAction::CHAR_ADD:              deleteCharFromLine(item);   break;
        case UndoAction::STRING_DEL:            addStrToLine(item);         break;
        case UndoAction::STRING_ADD:            deleteStrFromLine(item);    break;
        case UndoAction::LITERAL_MADD:          deleteMLiteral(item);       break;
        case UndoAction::MTABS_ADD:             deleteTabs(item);           break;
        case UndoAction::MTABS_DEL:             addTabs(item);              break;
        case UndoAction::LINE_NEW:              revertNewLine(item);        break;
        case UndoAction::LINE_COLLAPSE_NEXT:    addNewLineAfter(item);      break;
        case UndoAction::LINE_COLLAPSE_PREV:    addNewLineBefore(item);     break;
        case UndoAction::LINES_ADD:             deleteLines(item);          break;
        case UndoAction::LINES_DEL:             addLines(item);             break;
        default: LOG_WARNING("how?!"); break;
    }

    // recording undoable actions again
    m_window->m_storeActions = true;

    // m_window->update_lines_after_y_(m_window->m_cursor.cy());
    m_window->refresh_next_frame_();
    m_window->buffer_changed_();
    m_window->syntax_highlight_buffer_();

}

//---------------------------------------------------------------------------------------
void UndoBuffer::deleteCharFromLine(const undo_item_t &_item)
{
    const line_chars_t *sline = &_item.sline;
    FileBufferWindow *w = m_window;

    w->gotoBufferCursorPos(sline->start_pos);
    line_t *line = w->m_lineBuffer.ptrFromIdx(sline->start_pos.y);
    line->delete_at(sline->start_pos.x);
    // w->deleteCharAtCursor();

    w->m_windowLinesUpdateList.insert(sline->start_pos.y);

    if (_item.deleted_selection)
        next_LINES_DEL_();

}

//---------------------------------------------------------------------------------------
void UndoBuffer::addCharToLine(const undo_item_t &_item)
{
    const line_chars_t *sline = &_item.sline;
    FileBufferWindow *w = m_window;

    w->gotoBufferCursorPos(sline->start_pos);
    w->insertCharAtPos(sline->chars[0], sline->start_pos.x, false);

}

//---------------------------------------------------------------------------------------
void UndoBuffer::deleteStrFromLine(const undo_item_t &_item)
{
    const line_chars_t *sline = &_item.sline;
    FileBufferWindow *w = m_window;

    w->gotoBufferCursorPos(sline->start_pos);

    for (int i = 0; i < sline->len; i++)
        w->deleteCharAtCursor();
}

//---------------------------------------------------------------------------------------
void UndoBuffer::addStrToLine(const undo_item_t &_item)
{
    auto &sline = _item.sline;
    FileBufferWindow *w = m_window;
    
    w->gotoBufferCursorPos(sline.start_pos);
    w->insertStrAtCursor(sline.chars, sline.len, _item.move_cursor);
    
}

//---------------------------------------------------------------------------------------
void UndoBuffer::deleteMLiteral(const undo_item_t &_item)
{
    FileBufferWindow *w = m_window;
    mline_block_t mblock = _item.mline_block;

    w->gotoBufferCursorPos(mblock.start_pos);

    // two different lines
    if (mblock.start_pos.y != mblock.end_pos.y)
    {
        line_t *line0 = w->m_lineBuffer.ptrFromIdx(mblock.start_pos.y);
        line0->delete_at(mblock.start_pos.x);
        
        line_t *line1 = w->m_lineBuffer.ptrFromIdx(mblock.end_pos.y);
        line1->delete_at(mblock.end_pos.x);

        w->m_windowLinesUpdateList.insert(mblock.start_pos.y - w->m_scrollPos.y);
        w->m_windowLinesUpdateList.insert(mblock.end_pos.y - w->m_scrollPos.y);
    }
    // same line, two chars
    else
    {
        int x0 = mblock.start_pos.x;
        int x1 = mblock.end_pos.x;

        if (x0 > x1) std::swap(x0, x1);

        line_t *line = w->m_lineBuffer.ptrFromIdx(mblock.start_pos.y);
        line->delete_at(x1+1);
        line->delete_at(x0);

        w->m_windowLinesUpdateList.insert(mblock.start_pos.y - w->m_scrollPos.y);
    }

}

//---------------------------------------------------------------------------------------
void UndoBuffer::deleteTabs(const undo_item_t &_item)
{
    FileBufferWindow *w = m_window;
    mline_block_t mblock = _item.mline_block;
    auto &lines = mblock.copy_lines;
    
    w->gotoBufferCursorPos(mblock.start_pos);
    
    line_t *line = w->m_lineBuffer.ptrFromIdx(mblock.end_pos.y);
    size_t i = 0;
    while (i < mblock.end_pos.x && line != NULL)
    {
        int first_char = w->find_first_non_empty_char_(line);
        if (first_char && lines[i].len > 0)
        {
            int steps = first_char - w->find_prev_tab_stop_(first_char);
            line->delete_n_at(0, steps);
        }
        i++;
        line = line->next;
    }
    w->update_lines_after_y_(mblock.end_pos.y - w->m_scrollPos.y);

}

//---------------------------------------------------------------------------------------
void UndoBuffer::addTabs(const undo_item_t &_item)
{
    FileBufferWindow *w = m_window;
    mline_block_t mblock = _item.mline_block;
    auto &lines = mblock.copy_lines;

    w->gotoBufferCursorPos(mblock.start_pos);

    line_t *line = w->m_lineBuffer.ptrFromIdx(mblock.end_pos.y);
    size_t i = 0;
    LOG_INFO("");
    while (i < mblock.end_pos.x && line != NULL)
    {
        LOG_RAW("\t\t%s", line->__debug_str);
        if (line->len > 0 && line->content != NULL && lines[i].len != 0)
            w->insert_leading_tab_(line);

        i++;
        line = line->next;
    }
    w->update_lines_after_y_(mblock.end_pos.y - w->m_scrollPos.y);

}

//---------------------------------------------------------------------------------------
void UndoBuffer::revertNewLine(const undo_item_t &_item)
{
    FileBufferWindow *w = m_window;
    mline_block_t mblock = _item.mline_block;
    auto &lines = mblock.copy_lines;

    w->gotoBufferCursorPos(mblock.start_pos);
    
    line_t *lfirst = w->m_lineBuffer.ptrFromIdx(mblock.start_pos.y);
    line_t *llast = w->m_lineBuffer.ptrFromIdx(mblock.start_pos.y + lines.size() - 1);
    
    // a structural literal was split and another newline was inserted
    if (lines.size() == 3)
    {
        line_t *lmid = w->m_lineBuffer.ptrFromIdx(mblock.start_pos.y + 1);
        w->m_lineBuffer.deleteAtPtr(lmid);
    }
    
    // remove inserted whitespaces from last line
    llast->delete_n_at(0, mblock.end_pos.x);
    // coalesce first and last lines
    lfirst->insert_str(llast->content, llast->len, lfirst->len);
    // delete last line
    w->m_lineBuffer.deleteAtPtr(llast);

    w->update_lines_after_y_(mblock.start_pos.y - w->m_scrollPos.y);    

}

//---------------------------------------------------------------------------------------
void UndoBuffer::addNewLineAfter(const undo_item_t &_item)
{
    auto &start = _item.mline_block.start_pos;
    auto &end   = _item.mline_block.end_pos;
    // auto &lines = _item.mline_block.copy_lines;
    FileBufferWindow *w = m_window;

    w->gotoBufferCursorPos(start);

    line_t *line0 = w->m_lineBuffer.ptrFromIdx(start.y);
    line_t *line1 = line0->split_at_pos(end.x);
    w->m_lineBuffer.insertAtPtr(line0, INSERT_AFTER, line1);

    w->update_lines_after_y_(start.y - w->m_scrollPos.y);
}

//---------------------------------------------------------------------------------------
void UndoBuffer::addNewLineBefore(const undo_item_t &_item)
{
    auto &start = _item.mline_block.start_pos;
    auto &end   = _item.mline_block.end_pos;
    auto &lines = _item.mline_block.copy_lines;
    FileBufferWindow *w = m_window;

    w->gotoBufferCursorPos(start);

    line_t *line0 = w->m_lineBuffer.ptrFromIdx(start.y - 1);
    line_t *line1 = line0->split_at_pos(end.x);
    w->m_lineBuffer.insertAtPtr(line0, INSERT_AFTER, line1);
    w->m_currentLine = line1;
 
    w->update_lines_after_y_(start.y - 1 - w->m_scrollPos.y);

}

//---------------------------------------------------------------------------------------
// TODO : essentially a copy of FileBufferWindow::deleteSelection(). Merge!
void UndoBuffer::deleteLines(const undo_item_t &_item)
{
    auto &start = _item.mline_block.start_pos;
    auto &end   = _item.mline_block.end_pos;
    auto &lines = _item.mline_block.copy_lines;
    FileBufferWindow *w = m_window;

    w->gotoBufferCursorPos(start);

    line_t *line_ptr = w->m_lineBuffer.ptrFromIdx(start.y);
    int nlines = end.y - start.y;
    // bool merge_after = (lines[0].offset0 != 0 && nlines != 0);

    int i = 0;
    while (i <= nlines && line_ptr != NULL)
    {
        line_t *lnext = line_ptr->next;
        if (lines[i].len == line_ptr->len && i != nlines)
            w->m_lineBuffer.deleteAtPtr(line_ptr);
        else
            line_ptr->delete_n_at(lines[i].offset0, lines[i].len);

        i++;
        line_ptr = lnext;
    }

    w->m_currentLine = w->m_lineBuffer.ptrFromIdx(start.y);
    w->m_pageFirstLine = w->m_lineBuffer.ptrFromIdx(w->m_scrollPos.y);

    //
    //if (merge_after)
    //    w->m_lineBuffer.appendNextToThis(w->m_currentLine);

    w->update_lines_after_y_(start.y);
    
}

//---------------------------------------------------------------------------------------
// TODO : essentially a copy of FileBufferWindow::paste(). Merge!
void UndoBuffer::addLines(const undo_item_t &_item)
{
    auto &start = _item.mline_block.start_pos;
    auto &end   = _item.mline_block.end_pos;
    auto &lines = _item.mline_block.copy_lines;
    FileBufferWindow *w = m_window;

    w->gotoBufferCursorPos(start);

    //
    copy_line_t cline = lines[0];
    w->insertStrAtCursor(cline.line_chars, cline.len);
    if (cline.newline)
        w->insertNewLine(false);

    if (lines.size() > 1)
    {
        for (size_t i = 1; i < lines.size() - 1; i++)
        {
            copy_line_t cline1 = lines[i];
            // paste lines as a new lines (omitting last line)
            w->insertLineAtCursor(cline1.line_chars, cline1.len);
        }

        // last line
        cline = lines[lines.size() - 1];
        w->insertStrAtCursor(cline.line_chars, cline.len);
        if (cline.newline)
            w->insertNewLine(false);
    }

    //
    w->m_currentLine = w->m_lineBuffer.ptrFromIdx(w->m_cursor.cy() + w->m_scrollPos.y);
    w->m_pageFirstLine = w->m_lineBuffer.ptrFromIdx(w->m_scrollPos.y);

}

//---------------------------------------------------------------------------------------
void UndoBuffer::__debug_print_stack(std::stack<undo_item_t> &_stack, size_t _n)
{
    if (_stack.empty() || _n == 0)
        return;
    
    undo_item_t item = _stack.top();
    _stack.pop();
    __debug_print_stack(_stack, _n-1);
    _stack.push(item);
    __debug_print_item(item, _n);

}

//---------------------------------------------------------------------------------------
void UndoBuffer::__debug_print_item(undo_item_t &_item, size_t _n)
{
    LOG_RAW("=== [undo_item_t] %p (%d) ===", &_item, _n);
    LOG_RAW("action = %s", __debug_undoAction2Str(_item.action));
    if (_item.mline_block.size() ||
        _item.mline_block.start_pos != ivec2_t(0) ||
        _item.mline_block.end_pos != ivec2_t(0))
    {
        LOG_RAW("mblock:");
        LOG_RAW("\tstart_pos = %d, %d", _item.mline_block.start_pos.x, _item.mline_block.start_pos.y);
        LOG_RAW("\tend_pos   = %d, %d", _item.mline_block.end_pos.x, _item.mline_block.end_pos.y);
        LOG_RAW("\tmlines:")
        for (auto &it : _item.mline_block.copy_lines)
            LOG_RAW("\t\t'%s' (%zu) [%zu : %zu] (newline: %s)",
                    it.line_chars,
                    it.len,
                    it.offset0,
                    it.offset1,
                    it.newline ? "true" : "false");
    }
    else    // single line
    {
        LOG_RAW("sline:");
        LOG_RAW("\tstart_pos = %d, %d", _item.sline.start_pos.x, _item.sline.start_pos.y);
        LOG_RAW("\tchars = %s (%zu)", _item.sline.chars, _item.sline.len);
    }
    
}

//---------------------------------------------------------------------------------------
const char *__debug_undoAction2Str(UndoAction _a)
{
    #ifdef DEBUG
    switch (_a)
    {
        case UndoAction::CHAR_ADD:              return "CHAR_ADD";
        case UndoAction::CHAR_DEL:              return "CHAR_DEL";
        case UndoAction::STRING_ADD:            return "STRING_ADD";
        case UndoAction::STRING_DEL:            return "STRING_DEL";
        case UndoAction::LITERAL_MADD:          return "LITERAL_MADD";
        case UndoAction::MTABS_ADD:             return "MTABS_ADD";
        case UndoAction::MTABS_DEL:             return "MTABS_DEL";
        case UndoAction::LINE_NEW:              return "LINE_NEW";
        case UndoAction::LINE_COLLAPSE_NEXT:    return "LINE_COLLAPSE_NEXT";
        case UndoAction::LINE_COLLAPSE_PREV:    return "LINE_COLLAPSE_PREV";
        case UndoAction::LINES_ADD:             return "LINES_ADD";
        case UndoAction::LINES_DEL:             return "LINES_DEL";
        default: return "UNKNOWN";
    }
    #endif    

}

