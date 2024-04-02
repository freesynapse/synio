#ifndef __UNDO_BUFFER_H
#define __UNDO_BUFFER_H

#include <stack>
#include <vector>
#include <unordered_map>

#include "../types.h"

enum class UndoItemType
{
    LINES,
    STRING,
    TABS,
};

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
    undo_item_t(UndoItemType _type, UndoAction _action, const mline_block_t &_mline_block) :
        type(_type), action(_action), mline_block(_mline_block)
    {}

    UndoItemType type;
    UndoAction action;
    mline_block_t mline_block;
    //std::vector<copy_line_t> m_lines;
    //ivec2_t m_startPos;
    //ivec2_t m_endPos;   // only used for block insert/delete

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

// private:
    std::stack<undo_item_t> m_stack;
    FileBufferWindow *m_window = NULL;

};

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
