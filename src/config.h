#ifndef __CONFIG_H
#define __CONFIG_H

#include <string>
#include <unordered_set>

// Enumerations
enum class TabsOrSpaces
{
    TABS = 0,
    SPACES,
};

//
class Config
{
public:
    static bool SHOW_LINE_NUMBERS;
    static int LINE_NUMBERS_MIN_WIDTH;

    static int COMMAND_WINDOW_HEIGHT;

    static int PAGE_SIZE;
    static int TAB_SIZE;
    static TabsOrSpaces USE_TABS_OR_SPACES;

    static const char *COL_DELIMITERS;
    static const char *FILE_PATH_DELIMITERS;
    static const char *STRUCTURAL_LITERALS;

    static std::unordered_set<char> ALLOWED_CHARACTERS;

    //
    static void readConfigFile(const std::string &_filename="");

};


#endif //__CONFIG_H
