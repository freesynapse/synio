#ifndef __UNDO_BUFFER_H
#define __UNDO_BUFFER_H

#include <stack>
#include <vector>
#include <unordered_map>

#include "../types.h"


//
enum class UndoAction
{
    STRING_ADD,
    STRING_MADD,    // for multiple cursors; TODO : implement
    STRING_DEL,
    STRING_MDEL,    // for multiple cursors; TODO : implement
    LINES_ADD,
    LINES_DEL,
    TAB_INSERT,
    MTABS_ADD,
    TAB_DEL,
    MTABS_DEL,
};

//
struct undo_item_t
{
    undo_item_t() {}
    ~undo_item_t() = default;
    undo_item_t(UndoAction _action, const mline_block_t &_mline_block) :
        action(_action), mline_block(_mline_block)
    {}    
    undo_item_t(UndoAction _action,
                const ivec2_t &_start_pos,
                const ivec2_t &_end_pos) :
        action(_action)
    {
        mline_block.setStart(_start_pos);
        mline_block.setEnd(_end_pos);
    }
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

    undo_item_t(UndoAction _action,
                const line_chars_t &_sline) :
        action(_action), sline(_sline)
    {}

    UndoAction action;
    mline_block_t mline_block;  // multi-line actions
    line_chars_t sline;         // single line actions

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

private:
    // action functions
    void deleteLines(const undo_item_t &_item);
    void addLines(const undo_item_t &_item);
    void deleteStrFromLine(const undo_item_t &_item);
    void addStrToLine(const undo_item_t &_item);

private:
    std::stack<undo_item_t> m_stack;
    FileBufferWindow *m_window = NULL;

};

//extern const char *UndoItemType2Str(UndoItemType _t);
extern const char *UndoAction2Str(UndoAction _a);

// TODO :   these should be moved to the ReplayBuffer class, since each file buffer  
//          window should have its own ReplayBuffer.
// map for mapping lines to line_t pointers (mostly for the replay buffer)
//static std::unordered_map<int, line_t *> s_lineToPtrMap;
// updating (and adding) line_t ptr to map
//extern void update_replay_map(line_t *_line_ptr);
// tracking current line number
//static int s_nextRegretLineNo = 0;
//extern size_t get_next_regret_line_no();




#endif // __UNDO_BUFFER_H
