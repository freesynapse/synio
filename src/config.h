#ifndef __CONFIG_H
#define __CONFIG_H

#include <string>
#include <unordered_set>
#include <unordered_map>

// Enumerations
enum class TabsOrSpaces
{
    TABS = 0,
    SPACES,
};

// constexpr shenanigans
constexpr std::size_t strlen_constexpr(const char *_s) 
{
    return std::char_traits<char>::length(_s);
}

//
class Config
{
public:
    static bool SHOW_LINE_NUMBERS;
    static int LINE_NUMBERS_MIN_WIDTH;

    static int COMMAND_WINDOW_HEIGHT;           // height of (unexapnded) command window
                                                // (in lines)

    static float FILE_DIALOG_SIZE_PERCENT;      // proportion of the screen (both width 
                                                // and height) occupied by file popup
                                                // dialogs
    static int FILE_DIALOG_LISTING_SPACING;     // spacing between filenames (and left 
                                                // frame margin) in dir listings

    static int OPTIONS_DIALOG_OPT_HPAD;         // 'button' horizontal padding from window
                                                // edges
    static int OPTIONS_DIALOG_TEXT_HPAD;        // as above but for text
    static int OPTIONS_DIALOG_DEFAULT_WIDTH;
    
    static int PAGE_SIZE;                       // size of jump for PgUp/PgDn
    static int TAB_SIZE;
    static TabsOrSpaces USE_TABS_OR_SPACES;     // TODO : now unused

    static const char *COL_DELIMITERS;
    static const char *FILE_PATH_DELIMITERS;
    static const char *STRUCTURAL_LITERALS;

    static std::unordered_set<char> ALLOWED_CHAR_SET;
    static constexpr const char *ALLOWED_CHARS = "]\\^~*-_.:,;<>|+\'\" "
                                                 "AaBbCcDdEeFfGgHhIiJjKkLlMm"
                                                 "NnOoPpQqRrSsTtUuVvWwXxYyZz"
                                                 "1234567890!#%&/()=?@${}[";
    static constexpr size_t ALLOWED_CHARS_SZ = strlen_constexpr(ALLOWED_CHARS);


    //
    static void readConfigFile(const std::string &_filename="");

};


#endif //__CONFIG_H
