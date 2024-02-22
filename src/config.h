#ifndef __CONFIG_H
#define __CONFIG_H

#include <string>

//
class Config
{
public:
    static bool SHOW_LINE_NUMBERS;
    static int PAGE_SIZE;
    static int TAB_SIZE;
    static const char *COL_DELIMITERS;
    
    static void readConfigFile(const std::string &_filename="");

};


#endif //__CONFIG_H
