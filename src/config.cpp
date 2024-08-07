
#include "config.h"

// static decls
bool            Config::SHOW_LINE_NUMBERS               = true;
int             Config::LINE_NUMBERS_MIN_WIDTH          = 5;

int             Config::COMMAND_WINDOW_HEIGHT           = 1;

float           Config::FILE_DIALOG_SIZE_PERCENT        = 0.75f;
int             Config::FILE_DIALOG_LISTING_SPACING     = 2;
int             Config::OPTIONS_DIALOG_OPT_HPAD         = 7;
int             Config::OPTIONS_DIALOG_TEXT_HPAD        = 2;
int             Config::OPTIONS_DIALOG_DEFAULT_WIDTH    = 50;

int             Config::PAGE_SIZE                       = 20;
int             Config::TAB_SIZE                        = 4;
TabsOrSpaces    Config::USE_TABS_OR_SPACES              = TabsOrSpaces::SPACES;

const char     *Config::COL_DELIMITERS                  = "\"\'.:;()<>$~&,\t ";
const char     *Config::FILE_PATH_DELIMITERS            = ".-/_";
const char     *Config::STRUCTURAL_LITERALS             = "{}()[]";

std::unordered_set<char> Config::ALLOWED_CHAR_SET = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U',
    'V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p',
    'q','r','s','t','u','v','w','x','y','z','1','2','3','4','5','6','7','8','9','0','!',
    '#','%','&','/','(',')','=','?','@','$','{','}','[',']','\\','^','~','*','-','_','.',
    ':',',',';','<','>','|','+','\'', '\"', ' '
};

//---------------------------------------------------------------------------------------
void Config::readConfigFile(const std::string &_filename)
{
    std::string fn = (_filename == "" ? ".syniocfg" : _filename);

    // TODO : implement me!
    
}


