
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
    assert(m_stack.size() != 0);
    
    undo_item_t item = m_stack.top();
    m_stack.pop();

    switch (item.type)
    {
        case UndoItemType::LINES:
        {
            switch (item.action)
            {
                case UndoAction::LINES_ADD:
                    // remove lines
                    m_window->__delete_mline_block(item.mline_block);
                    break;
                case UndoAction::LINES_DEL:
                    // add lines back in
                    //pasteLines();
                    break;
                default: LOG_WARNING("how?!"); break;
            }
            break;
        }

        case UndoItemType::STRING:
        {
            break;
        }

        case UndoItemType::TABS:
        {
            break;
        }
    }
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

