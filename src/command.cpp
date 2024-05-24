
#include "command.h"

// static decls
std::unordered_map<int, command_t> Command::s_commandMap =
{
    { CTRL('s'), { CommandID::SAVE_BUFFER, "(C-x C-s) save_buffer", "Save (current buffer)", "" } },
    { CTRL('f'), { CommandID::BUFFER_SEARCH, "(C-x C-f) buffer_find", "Find (current buffer)", "" } },
    { CTRL('h'), { CommandID::BUFFER_REPLACE, "(C-x C-h) buffer_replace", "Replace (current buffer)", "Find:" } },
    // { CTRL('s'), { CommandID::SAVE_BUFFER, "Save current buffer as", "Filename:" } },
    { CTRL('c'), { CommandID::YN_DIAG_SAVE_BEFORE_EXIT, "(C-x C-c) save_and_exit", "Exit Synio, save?", "Save buffer before exit (y/n)?" } },
    { CTRL('e'), { CommandID::YN_DIAG_EXIT, "(C-x C-e) exit_ask", "Exit Synio (without save)", "Exit (y/n)?" } },
    { CTRL('q'), { CommandID::JUST_EXIT, "(C-x C-q) exit_dont_ask", "Exit Synio now (without save)", "" } },
};
