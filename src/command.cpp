
#include "command.h"

// static decls
std::unordered_map<CommandID, command_t> Command::s_commandMap;
std::unordered_map<int, CommandID> Command::s_commandKeyCodes;
std::set<CommandID> Command::s_fileIOCommands;
prefix_node_t *Command::s_cmdPTree;

//---------------------------------------------------------------------------------------
void Command::initialize()
{
    // all commands
    s_commandMap = std::unordered_map<CommandID, command_t>
    {
        // { CTRL('s'), { CommandID::SAVE_BUFFER, "Save current buffer as", "Save As (filename):" } },
        { CommandID::SAVE_BUFFER,               { CommandID::SAVE_BUFFER, "save-buffer", "Save (current buffer)", "" } },
        { CommandID::SAVE_TEMP_BUFFER,          { CommandID::SAVE_TEMP_BUFFER, "save-temp-buffer", "Save as (temporary buffer)", "Save as (temp buffer) (filename): " } },
        { CommandID::SAVE_BUFFER_AS,            { CommandID::SAVE_BUFFER_AS, "save-buffer-as", "Save as (current buffer)", "Save as (filename): " } },
        { CommandID::SAVE_ALL,                  { CommandID::SAVE_ALL, "save-all-buffers", "Save all buffers", "" } },
        { CommandID::OPEN_BUFFER,               { CommandID::OPEN_BUFFER, "open-buffer", "Open buffer", "Open (filename): " } },
        { CommandID::CLOSE_THIS_BUFFER,         { CommandID::CLOSE_THIS_BUFFER, "close-this-buffer", "Closes current buffer", "Save buffer before closing (Y/n)?" } },
        { CommandID::CLOSE_BUFFER,              { CommandID::CLOSE_BUFFER, "close-buffer", "Closes chosen buffer", "Buffer (filename)?" } },
        { CommandID::NEW_BUFFER,                { CommandID::NEW_BUFFER, "new-buffer", "Creates a new buffer", "New buffer (filename)?" } },

        { CommandID::SWITCH_TO_BUFFER,          { CommandID::SWITCH_TO_BUFFER, "switch-to-buffer", "Switch to open buffer", "Buffer (filename)?" } },

        { CommandID::BUFFER_SEARCH,             { CommandID::BUFFER_SEARCH, "buffer-find", "Find (current buffer)", "query:" } },
        { CommandID::BUFFER_REPLACE,            { CommandID::BUFFER_REPLACE, "buffer-replace", "Replace (current buffer)", "replace:" } },

        { CommandID::EXIT_SAVE_YN,              { CommandID::EXIT_SAVE_YN, "save-and-exit", "Save before exit?", "Save modified buffers (Y/n)?" } },
        { CommandID::EXIT_NO_SAVE_YN,           { CommandID::EXIT_NO_SAVE_YN, "exit-ask", "Exit Synio (without save)", "Exit (no save) (Y/n)?" } },

        { CommandID::JUST_EXIT,                 { CommandID::JUST_EXIT, "just-exit", "Exit Synio now (without save)", "" } },
        { CommandID::SHOW_SHORTCUTS,            { CommandID::SHOW_SHORTCUTS, "show-shortcuts", "Shows all keyboard shortcuts", "" } },
        { CommandID::SHOW_OPEN_BUFFERS,         { CommandID::SHOW_OPEN_BUFFERS, "show-open_buffers", "List all open buffers", "" } },
        { CommandID::SHOW_BUFFER_STATISTICS,    { CommandID::SHOW_BUFFER_STATISTICS, "show-buffer-stats", "Show statistics for the current buffer", "" } },

        #ifdef DEBUG
        { CommandID::DEBUG_COMMAND,             { CommandID::DEBUG_COMMAND, "debug-command", "", "" } },
        #endif
        { CommandID::INVALID_COMMAND,           { CommandID::INVALID_COMMAND, "", "", "" } },
    };

    //
    s_commandKeyCodes = std::unordered_map<int, CommandID>
    {
        { CTRL('s'), CommandID::SAVE_BUFFER },
        { CTRL('o'), CommandID::OPEN_BUFFER },
        { CTRL('w'), CommandID::CLOSE_THIS_BUFFER },
        { CTRL('k'), CommandID::CLOSE_BUFFER },
        { CTRL('f'), CommandID::BUFFER_SEARCH },
        { CTRL('r'), CommandID::BUFFER_REPLACE },   // ctrl+h confilcts with ctrl+backspace (key 8)
        //{ CTRL('c'), CommandID::YN_DIAG_SAVE_BEFORE_EXIT },
        //{ CTRL('e'), CommandID::YN_DIAG_EXIT },
        { CTRL('q'), CommandID::JUST_EXIT },
        { CTRL('p'), CommandID::SWITCH_TO_BUFFER },
        { CTRL('n'), CommandID::NEW_BUFFER },
        #ifdef DEBUG
        { CTRL('d'), CommandID::DEBUG_COMMAND },
        #endif
    };

    //
    s_fileIOCommands = std::set<CommandID>
    {
        CommandID::SAVE_BUFFER,
        CommandID::SAVE_TEMP_BUFFER,
        CommandID::SAVE_BUFFER_AS,
        CommandID::SAVE_ALL,
        CommandID::OPEN_BUFFER,
        CommandID::CLOSE_BUFFER,
        CommandID::NEW_BUFFER,
    };

    // insert all commands into prefix tree (for autocompletion)
    s_cmdPTree = new prefix_node_t();
    for (auto &it : s_commandMap)
    {
        if (it.second.id_str != "")
            PrefixTree::insert_string(s_cmdPTree, it.second.id_str);
    }

    LOG_INFO("done.");

}

//---------------------------------------------------------------------------------------
const char *Command::cmd2Str(CommandID _cmd)
{
    switch (_cmd)
    {
        case CommandID::SAVE_BUFFER:            return "CommandID::SAVE_BUFFER";
        case CommandID::SAVE_TEMP_BUFFER:       return "CommandID::SAVE_TEMP_BUFFER";
        case CommandID::SAVE_BUFFER_AS:         return "CommandID::SAVE_BUFFER_AS";
        case CommandID::SAVE_ALL:               return "CommandID::SAVE_ALL";
        case CommandID::OPEN_BUFFER:            return "CommandID::OPEN_BUFFER";
        case CommandID::CLOSE_BUFFER:           return "CommandID::CLOSE_BUFFER";
        case CommandID::CLOSE_THIS_BUFFER:      return "CommandID::CLOSE_THIS_BUFFER";
        case CommandID::NEW_BUFFER:             return "CommandID::NEW_BUFFER";
        case CommandID::SWITCH_TO_BUFFER:       return "CommandID::SWITCH_TO_BUFFER";
        case CommandID::BUFFER_SEARCH:          return "CommandID::BUFFER_SEARCH";
        case CommandID::BUFFER_REPLACE:         return "CommandID::BUFFER_REPLACE";
        case CommandID::EXIT_SAVE_YN:           return "CommandID::EXIT_SAVE_YN";
        case CommandID::EXIT_NO_SAVE_YN:        return "CommandID::EXIT_NO_SAVE_YN";
        case CommandID::JUST_EXIT:              return "CommandID::JUST_EXIT";
        case CommandID::SHOW_SHORTCUTS:         return "CommandID::SHOW_SHORTCUTS";
        case CommandID::SHOW_OPEN_BUFFERS:      return "CommandID::SHOW_OPEN_BUFFERS";
        case CommandID::SHOW_BUFFER_STATISTICS: return "CommandID::SHOW_BUFFER_STATISTICS";
        case CommandID::DEBUG_COMMAND:          return "CommandID::DEBUG_COMMAND";
        case CommandID::INVALID_COMMAND:        return "CommandID::INVALID_COMMAND";
        case CommandID::NONE:                   return "CommandID::NONE";
    }
}
