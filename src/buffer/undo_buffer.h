#ifndef __UNDO_BUFFER_H
#define __UNDO_BUFFER_H

#include <stack>
#include <vector>
#include <unordered_map>

#include "../types.h"


//
enum class UndoAction
{
    // for multiple cursors; probably just adding an undo item per edit
    CHAR_ADD,
    CHAR_DEL,       // delete
    CHAR_ERASE,     // backspace
    STRING_ADD,
    STRING_DEL,
    STRING_ERASE,
    LITERAL_MADD,
    MTABS_ADD,
    MTABS_DEL,
    LINE_NEW,
    LINE_COLLAPSE_NEXT,
    LINE_COLLAPSE_PREV,
    LINES_ADD,
    LINES_DEL,
};

//
struct undo_item_t
{
    undo_item_t() {}
    ~undo_item_t() = default;
    //
    undo_item_t(UndoAction _action, const mline_block_t &_mline_block) :
        action(_action), mline_block(_mline_block)
    {}
    //    
    undo_item_t(UndoAction _action,
                const ivec2_t &_start_pos,
                const ivec2_t &_end_pos) :
        action(_action)
    {
        mline_block.setStart(_start_pos);
        mline_block.setEnd(_end_pos);
    }
    //
    undo_item_t(UndoAction _action,
                const ivec2_t &_start_pos,
                const ivec2_t &_end_pos,
                const std::vector<copy_line_t> &_lines) :
        action(_action)
    {
        mline_block.setStart(_start_pos);
        mline_block.setEnd(_end_pos);
        mline_block.copy_lines = _lines;
    }
    //
    undo_item_t(UndoAction _action,
                const line_chars_t &_sline) :
        action(_action), sline(_sline)
    {}

    //
    UndoAction action;
    mline_block_t mline_block;  // multi-line actions
    line_chars_t sline;         // single line actions
    char ch;                    // single char actions
    bool move_cursor = false;   // to diff between STRING_DEL prev and next
    int sel_start_y = 0;
    bool deleted_selection = false;

};

//
class FileBufferWindow;
class UndoBuffer
{
public:
    UndoBuffer() {}
    UndoBuffer(FileBufferWindow *_window) : 
        m_window(_window) 
    {}
    ~UndoBuffer() = default;

    void push(const undo_item_t &_item);
    void undo();

    //
    void __debug_print_stack(std::stack<undo_item_t> &_stack, size_t _n=0);
    void __debug_print_stack()
    {
        if (m_stack.size() == 0)
        {
            LOG_INFO("undo stack is empty.");
            return;
        }
        LOG_INFO("printing undo stack");
        __debug_print_stack(m_stack, m_stack.size());
    }
    void __debug_print_item(undo_item_t &_item, size_t _n=0);

private:
    // action functions
    void deleteCharFromLine(const undo_item_t &_item);
    void addCharToLine(const undo_item_t &_item, bool _erase);

    void deleteStrFromLine(const undo_item_t &_item);
    void addStrToLine(const undo_item_t &_item, bool _erase);

    void deleteMLiteral(const undo_item_t &_item);

    void deleteTabs(const undo_item_t &_item);
    void addTabs(const undo_item_t &_item);

    void revertNewLine(const undo_item_t &_item);
    void addNewLineAfter(const undo_item_t &_item);
    void addNewLineBefore(const undo_item_t &_item);

    void deleteLines(const undo_item_t &_item);
    void addLines(const undo_item_t &_item);

    //
    __always_inline void next_LINES_DEL_()
    {
        if (m_stack.size() == 0)
            return;
            
        undo_item_t next_item = m_stack.top();
        if (next_item.action == UndoAction::LINES_DEL)
        {
            addLines(next_item);
            m_stack.pop();
        }
    }

private:
    std::stack<undo_item_t> m_stack;
    FileBufferWindow *m_window = NULL;

};

// debug function
extern const char *__debug_undoAction2Str(UndoAction _a);




#endif // __UNDO_BUFFER_H
