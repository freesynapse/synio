
#include "replay_buffer.h"


//
void update_replay_map(line_t *_line_ptr)
{
    if (!_line_ptr)
    {
        LOG_WARNING("NULL line_t *");
        return;
    }

    // if (s_replayMap.find(_line_no) == s_replayMap.end())
    s_replayMap[_line_ptr->replay_line_no] = _line_ptr;
    LOG_INFO("added entry [%zu -> %p]", _line_ptr->replay_line_no, s_replayMap[_line_ptr->replay_line_no]);

}

//---------------------------------------------------------------------------------------
size_t get_next_replay_line_no()
{
    size_t line_no = s_nextLineNo;
    s_nextLineNo++;
    return line_no;

}

