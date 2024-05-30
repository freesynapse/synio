#ifndef __COMMAND_H
#define __COMMAND_H

#include <unordered_map>
#include <string>

#include "utils/log.h"
#define NCURSES_IMPL
#include "platform/platform.h"


//
enum class CommandID
{
    // file managment
    SAVE_BUFFER,
    SAVE_BUFFER_AS,
    OPEN_BUFFER,

    // workflow
    SWITCH_TO_BUFFER,

    // buffer operations
    BUFFER_SEARCH,
    BUFFER_REPLACE,

    // yes/no dialogs
    YN_DIAG_SAVE_OVERWRITE_FILE,
    YN_DIAG_SAVE_BEFORE_EXIT,
    YN_DIAG_EXIT,

    JUST_EXIT,

};

//
struct command_t
{
    // int keycode; // key in std::unordered_map<int, Command>
    CommandID id;
    std::string id_str;
    std::string command_str;
    std::string command_prompt;

    command_t() {}
    command_t(CommandID _id, 
              const std::string &_str, 
              const std::string &_command_str,
              const std::string &_prompt) :
        id(_id), id_str(_str), command_str(_command_str), command_prompt(_prompt)
    {}

};

class Command
{
public:
    // Sorted by keycode (int)
    static std::unordered_map<int, command_t> s_commandMap;

};


//
/*
class Command
{
public:
    Command() {}
    ~Command() = default;

    void execute() {}   // use this structure?

private:
    std::string m_displayName = "";     // would be e.g. "C-x"

};

//
class CommandHandler
{
public:
    CommandHandler() {}
    ~CommandHandler() = default;

    void registerCommand(const Command &_command)
    {

    }

    void executeCommand(int _keycode)
    {
        if (m_commands.find(_keycode) == m_commands.end())
        {
            LOG_WARNING("command with keycode %d not found.", _keycode);
            return;
        }

        

    }


private:
    std::unordered_map<int, Command> m_commands;

};
*/


#endif // __COMMAND_H
