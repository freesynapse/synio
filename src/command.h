#ifndef __COMMAND_H
#define __COMMAND_H

#include <unordered_map>
#include <string>

#include "utils/log.h"


enum class CommandID
{
    
};

//
class Command
{
public:
    Command() {}
    ~Command() = default;

    void execute() {}   // use this structure?

private:
    std::string m_displayName = "";     // would be e.g. "ctrl-x"

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



#endif // __COMMAND_H
