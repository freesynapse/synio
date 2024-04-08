
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
    LOG_RAW("=== [undo_item_t] %p ===", &item);
    // LOG_RAW("type   = %s", UndoItemType2Str(_item.type));
    LOG_RAW("action = %s", UndoAction2Str(item.action));
    if (item.mline_block.size() ||
        item.mline_block.start_pos != ivec2_t(0) ||
        item.mline_block.end_pos != ivec2_t(0))
    {
        LOG_RAW("mblock:");
        LOG_RAW("\tstart_pos = %d, %d", item.mline_block.start_pos.x, item.mline_block.start_pos.y);
        LOG_RAW("\tend_pos   = %d, %d", item.mline_block.end_pos.x, item.mline_block.end_pos.y);
        LOG_RAW("\tmlines:")
        for (auto &it : item.mline_block.copy_lines)
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
        LOG_RAW("\tstart_pos = %d, %d", item.sline.start_pos.x, item.sline.start_pos.y);
        LOG_RAW("\tchars = %s (%zu)", item.sline.chars, item.sline.len);
    }
    #endif

    // prevent new commands of being recorded as undoable
    m_window->m_storeActions = false;

    //
    switch (item.action)
    {
        case UndoAction::LINES_ADD:     deleteLines(item);          break;
        case UndoAction::LINES_DEL:     addLines(item);             break;
        case UndoAction::STRING_ADD:    deleteStrFromLine(item);    break;
        case UndoAction::LITERAL_MADD:     deleteMLiteral(item);          break;
        case UndoAction::MTABS_ADD:     deleteTabs(item);           break;
        case UndoAction::MTABS_DEL:     addTabs(item);              break;
        case UndoAction::NEWLINE:       revertNewLine(item);        break;
        default: LOG_WARNING("how?!"); break;
    }

    // go back to recording actions as undoable
    m_window->m_storeActions = true;

    m_window->update_lines_after_y_(m_window->m_cursor.cy());
    m_window->refresh_next_frame_();
    m_window->buffer_changed_();
    m_window->syntax_highlight_buffer_();

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
    bool merge_after = (lines[0].offset0 != 0 && nlines != 0);
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
    if (merge_after)
        w->m_lineBuffer.appendNextToThis(w->m_currentLine);


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
    FileBufferWindow *w = m_window;
    w->gotoBufferCursorPos(_item.sline.start_pos);
    // w->insertCharAtPos()
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
    while (i < lines.size() && line != NULL)
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
    while (i < lines.size() && line != NULL)
    {
        if (lines[i].len > 0)
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
const char *UndoAction2Str(UndoAction _a)
{
    #ifdef DEBUG
    switch (_a)
    {
        case UndoAction::STRING_ADD:    return "STRING_ADD";
        case UndoAction::STRING_DEL:    return "STRING_DEL";
        case UndoAction::LITERAL_MADD:  return "LITERAL_MADD";
        case UndoAction::LINES_ADD:     return "LINES_ADD";
        case UndoAction::LINES_DEL:     return "LINES_DEL";
        case UndoAction::MTABS_ADD:     return "MTABS_ADD";
        case UndoAction::MTABS_DEL:     return "MTABS_DEL";
        case UndoAction::NEWLINE:       return "NEWLINE";
        default: return "UNKNOWN";
    }
    #endif    

}

////---------------------------------------------------------------------------------------
//void update_regret_map(line_t *_line_ptr)
//{
//    if (!_line_ptr)
//    {
//        LOG_WARNING("line_t * nullptr");
//        return;
//    }
//
//    // if (s_replayMap.find(_line_no) == s_replayMap.end())
//    s_lineToPtrMap[_line_ptr->regret_line_no] = _line_ptr;
//    LOG_INFO("added entry [%d -> %p]",
//             _line_ptr->regret_line_no,
//             s_lineToPtrMap[_line_ptr->regret_line_no]);
//
//}
//
////---------------------------------------------------------------------------------------
//size_t get_next_regret_line_no()
//{
//    size_t line_no = s_nextRegretLineNo;
//    s_nextRegretLineNo++;
//    return line_no;
//
//}

