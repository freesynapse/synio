
#include "command.h"

// static decls
std::unordered_map<int, command_t> Command::s_commandMap =
{
    { CTRL('s'), { CommandID::SAVE_BUFFER, "save_buffer", "Save (current buffer)", "" } },
    { CTRL('o'), { CommandID::OPEN_BUFFER, "open_buffer", "Open buffer", "File:" } },
    { CTRL('f'), { CommandID::BUFFER_SEARCH, "buffer_find", "Find (current buffer)", "" } },
    { CTRL('h'), { CommandID::BUFFER_REPLACE, "buffer_replace", "Replace (current buffer)", "Find:" } },
    // { CTRL('s'), { CommandID::SAVE_BUFFER, "Save current buffer as", "Filename:" } },
    { CTRL('c'), { CommandID::YN_DIAG_SAVE_BEFORE_EXIT, "save_and_exit", "Exit Synio, save?", "Save buffer before exit (y/n)?" } },
    { CTRL('e'), { CommandID::YN_DIAG_EXIT, "exit_ask", "Exit Synio (without save)", "Exit (y/n)?" } },
    { CTRL('q'), { CommandID::JUST_EXIT, "just_exit", "Exit Synio now (without save)", "" } },
    { CTRL('p'), { CommandID::SWITCH_TO_BUFFER, "switch_to_buffer", "Switch to open buffer", "File:" } },
};
