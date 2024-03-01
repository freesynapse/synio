#ifndef __CONFIG_H
#define __CONFIG_H

#include <string>

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
    static int PAGE_SIZE;
    static int TAB_SIZE;
    static const char *COL_DELIMITERS;
    static const char *FILE_PATH_DELIMITERS;

    static TabsOrSpaces USE_TABS_OR_SPACES;
    
    static int LINE_NUMBERS_MIN_WIDTH;

    //
    static void readConfigFile(const std::string &_filename="");

};


#endif //__CONFIG_H
