
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
    LOG_RAW("[undo_item_t] %p", &item);
    // LOG_RAW("type   = %s", UndoItemType2Str(_item.type));
    LOG_RAW("action = %s", UndoAction2Str(item.action));
    LOG_RAW("mblock:");
    LOG_RAW("\tstart_pos = %d, %d", item.mline_block.start_pos.x, item.mline_block.start_pos.y);
    LOG_RAW("\tend_pos   = %d, %d", item.mline_block.end_pos.x, item.mline_block.end_pos.y);
    LOG_RAW("\tmlines:")
    LOG_RAW("\tsline = '%s' (%zu) [%zu : %zu] (newline: %s)", 
            item.sline.line_chars,
            item.sline.len,
            item.sline.offset0,
            item.sline.offset1,
            item.sline.newline ? "true" : "false");
    for (auto &it : item.mline_block.copy_lines)
        LOG_RAW("\t\t'%s' (%zu) [%zu : %zu] (newline: %s)",
                it.line_chars,
                it.len,
                it.offset0,
                it.offset1,
                it.newline ? "true" : "false");
    #endif

    //
    switch (item.action)
    {
        case UndoAction::LINES_ADD:
            // remove lines
            deleteLines(item);
            break;
        case UndoAction::LINES_DEL:
            // add lines back in
            addLines(item);
            break;
        default: LOG_WARNING("how?!"); break;
    }

    m_window->update_lines_after_y_(m_window->m_cursor.cy());
    m_window->refresh_next_frame_();
    m_window->buffer_changed_();
    m_window->syntax_highlight_buffer_();

}

//---------------------------------------------------------------------------------------
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
/*
const char *UndoItemType2Str(UndoItemType _t)
{
    #ifdef DEBUG
    switch (_t)
    {
        case UndoItemType::LINES:   return "LINES";
        case UndoItemType::STRING:  return "STRING";
        case UndoItemType::TABS:    return "TABS";
        default: return "UNKNOWN";
    }
    #endif    

}
*/

//---------------------------------------------------------------------------------------
const char *UndoAction2Str(UndoAction _a)
{
    #ifdef DEBUG
    switch (_a)
    {
        case UndoAction::STRING_ADD:    return "STRING_ADD";
        case UndoAction::STRING_MADD:   return "STRING_MADD";
        case UndoAction::STRING_DEL:    return "STRING_DEL";
        case UndoAction::STRING_MDEL:   return "STRING_MDEL";
        case UndoAction::LINES_ADD:     return "LINES_ADD";
        case UndoAction::LINES_DEL:     return "LINES_DEL";
        case UndoAction::TAB_INSERT:    return "TAB_INSERT";
        case UndoAction::MTABS_ADD:     return "MTABS_ADD";
        case UndoAction::TAB_DEL:       return "TAB_DEL";
        case UndoAction::MTABS_DEL:     return "MTABS_DEL";
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

