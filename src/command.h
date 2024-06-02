#ifndef __COMMAND_H
#define __COMMAND_H

#include <unordered_map>
#include <string>

#include "utils/log.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#define NCURSES_IMPL
#pragma clang diagnostic pop

#include "platform/platform.h"


//
enum CommandID
{
    // file managment
    SAVE_BUFFER = 0,                // impl
    SAVE_TEMP_BUFFER,
    SAVE_BUFFER_AS,                 // impl
    SAVE_ALL,
    OPEN_BUFFER,
    CLOSE_BUFFER,
    NEW_BUFFER,

    // workflow
    SWITCH_TO_BUFFER,
    NEXT_BUFFER,
    PREV_BUFFER,

    // buffer operations
    BUFFER_SEARCH,
    BUFFER_REPLACE,

    // yes/no dialogs
    EXIT_SAVE_YN,
    EXIT_NO_SAVE_YN,                // impl

    // misc
    JUST_EXIT,  // debug -- remove later
    SHOW_SHORTCUTS,
    SHOW_OPEN_BUFFERS,
    SHOW_BUFFER_STATISTICS,

    INVALID_COMMAND,
    NONE
};

// enums for all command codes not identified by key codes
enum CommandAuxCodes
{
    AUX_SHOW_SHORTCUTS = 10000,
    AUX_SHOW_OPEN_BUFFERS,
    AUX_SHOW_BUFFER_STATISTICS,
    
};

//
struct command_t
{
    // int keycode; // key in std::unordered_map<int, Command>
    CommandID id                = CommandID::NONE;
    std::string id_str          = "";
    std::string command_str     = "";
    std::string command_prompt  = "";

    command_t() {}
    command_t(CommandID _id, 
              const std::string &_str, 
              const std::string &_command_str,
              const std::string &_prompt) :
        id(_id), id_str(_str), command_str(_command_str), command_prompt(_prompt)
    {}

    #ifdef DEBUG
    void __debug_print()
    {
        LOG_INFO("DEBUG command_t: id=%d, id_str='%s', command_str='%s', command_prompt='%s'",
                 id, id_str.c_str(), command_str.c_str(), command_prompt.c_str());
    }
    #endif

};

class Command
{
public:
    static void initialize();
    static const char *cmd2Str(CommandID _cmd);

    //
    static bool is_cmd_code(int _keycode) { return s_commandKeyCodes.find(_keycode) != s_commandKeyCodes.end(); }
    static command_t cmd(CommandID _cmd)
    {
        if (s_commandMap.find(_cmd) != s_commandMap.end())
            return s_commandMap[_cmd];
        else 
            return command_t(); // CommandID::NONE
    }
    static bool is_cmd(CommandID _cmd) { return s_commandMap.find(_cmd) != s_commandMap.end(); }
    static command_t cmd_from_key_code(int _keycode) { return s_commandMap[s_commandKeyCodes[_keycode]]; }

    // Sorted by command code (int) (e.g. CTRL(c) for shortcuts or arbitrary int for 
    // other commands).
    static std::unordered_map<CommandID, command_t> s_commandMap;

    // set of all command codes
    static std::unordered_map<int, CommandID> s_commandKeyCodes;

};


#endif // __COMMAND_H
