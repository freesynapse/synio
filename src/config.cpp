
#include "config.h"

// static decls
bool Config::SHOW_LINE_NUMBERS = true;
int Config::PAGE_SIZE = 25;
int Config::TAB_SIZE = 4;
// for <CTRL> + left/right action
// const char *Config::COL_DELIMITERS = "\"\'.:;()+-*/=<>$~\t{} ";
const char *Config::COL_DELIMITERS = "\"\'.:;()<>$~\t{} ";
TabsOrSpaces Config::USE_TABS_OR_SPACES = TabsOrSpaces::SPACES;

//
void Config::readConfigFile(const std::string &_filename)
{
    std::string fn = (_filename == "" ? ".syniocfg" : _filename);

    // TODO : implement me!
    
}


