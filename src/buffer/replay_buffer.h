#ifndef __REPLAY_BUFFER_H
#define __REPLAY_BUFFER_H

#include <stack>
#include <unordered_map>

#include "../types.h"


//
struct replay_item_t
{

};

//
class ReplayBuffer
{
public:


// private:


};

// TODO :   these should be moved to the ReplayBuffer class, since each file buffer  
//          window should have its own ReplayBuffer.
// map for mapping lines to line_t pointers (mostly for the replay buffer)
static std::unordered_map<size_t, line_t *> s_replayMap;
// updating (and adding) line_t ptr to map
extern void update_replay_map(line_t *_line_ptr);
// tracking current line number
static size_t s_nextLineNo = 0;
extern size_t get_next_replay_line_no();




#endif // __REPLAY_BUFFER_H
