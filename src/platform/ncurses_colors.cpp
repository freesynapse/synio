
#include "ncurses_colors.h"

#include <assert.h>

#include "../types.h"
#include "../utils/log.h"


//
void ncurses_init_colors(API_WINDOW_PTR _w)
{
    if (!has_colors())
        LOG_CRITICAL_ERROR("No terminal color support. Please use nano instead.");

    start_color();
    
    // terminal default colors, background better be black...
    // Update: no, it doesn't have to be, index -1 signifies terminal background color
    //         (as seen in ncurses_color.h; #define SYN_COLOR_BKGD -1).
    use_default_colors();

    LOG_INFO("max COLORS=%d", COLORS);
    LOG_INFO("max PAIRS=%d", COLOR_PAIRS);

    // TODO : get values from Config::readConfigFile (if any), otherwise use defaults 
    //        below.
    
    // background colors
    init_color(SYN_COLOR_BKGD,             0,    0,    0);
    init_color(SYN_COLOR_SEL_BKGD,  300,  300,  300);
    init_color(SYN_COLOR_STATUS_BKGD,    900,  900,  900);

    // foreground colors
    init_color(SYN_COLOR_TEXT_FGD,             1000, 1000, 1000);
    init_color(SYN_COLOR_KEYWORD_FGD,          1000, 1000,    0);
    init_color(SYN_COLOR_STRING_FGD,            100,  800,  100);
    init_color(SYN_COLOR_NUMBER_FGD,           1000,  360,    0);
    init_color(SYN_COLOR_LITERAL_STRUCT_FGD,    800,  310, 1000);
    init_color(SYN_COLOR_LITERAL_OP_FGD,        700,  700,  100);
    init_color(SYN_COLOR_LITERAL_DELIM_FGD,     260,  575,  960);
    init_color(SYN_COLOR_COMMENT_FGD,           800,  550,  235);
    init_color(SYN_COLOR_PREPROC_FGD,          1000, 1000,    0);
    init_color(SYN_COLOR_BLACK,                   0,    0,    0);
    
    // regular color pairs
    init_pair(SYN_COLOR_TEXT,           SYN_COLOR_TEXT_FGD,           SYN_COLOR_BKGD       ); // 'standard' color scheme
    init_pair(SYN_COLOR_KEYWORD,        SYN_COLOR_KEYWORD_FGD,        SYN_COLOR_BKGD       );
    init_pair(SYN_COLOR_STRING,         SYN_COLOR_STRING_FGD,         SYN_COLOR_BKGD       );
    init_pair(SYN_COLOR_MSTRING,        SYN_COLOR_STRING_FGD,         SYN_COLOR_BKGD       );
    init_pair(SYN_COLOR_NUMBER,         SYN_COLOR_NUMBER_FGD,         SYN_COLOR_BKGD       );
    init_pair(SYN_COLOR_LITERAL_STRUCT, SYN_COLOR_LITERAL_STRUCT_FGD, SYN_COLOR_BKGD       );
    init_pair(SYN_COLOR_LITERAL_OP,     SYN_COLOR_LITERAL_OP_FGD,     SYN_COLOR_BKGD       );
    init_pair(SYN_COLOR_LITERAL_DELIM,  SYN_COLOR_LITERAL_DELIM_FGD,  SYN_COLOR_BKGD       );
    init_pair(SYN_COLOR_COMMENT,        SYN_COLOR_COMMENT_FGD,        SYN_COLOR_BKGD       );
    init_pair(SYN_COLOR_MCOMMENT,       SYN_COLOR_COMMENT_FGD,        SYN_COLOR_BKGD       );
    init_pair(SYN_COLOR_PREPROC,        SYN_COLOR_PREPROC_FGD,        SYN_COLOR_BKGD       );
    init_pair(SYN_COLOR_STATUS,         SYN_COLOR_BLACK,              SYN_COLOR_STATUS_BKGD);

    // selection color pairs
    init_pair(SYN_COLOR_SEL_TEXT,           SYN_COLOR_TEXT_FGD,           SYN_COLOR_SEL_BKGD);
    init_pair(SYN_COLOR_SEL_KEYWORD,        SYN_COLOR_KEYWORD_FGD,        SYN_COLOR_SEL_BKGD);
    init_pair(SYN_COLOR_SEL_STRING,         SYN_COLOR_STRING_FGD,         SYN_COLOR_SEL_BKGD);
    init_pair(SYN_COLOR_SEL_MSTRING,        SYN_COLOR_STRING_FGD,         SYN_COLOR_SEL_BKGD);
    init_pair(SYN_COLOR_SEL_NUMBER,         SYN_COLOR_NUMBER_FGD,         SYN_COLOR_SEL_BKGD);
    init_pair(SYN_COLOR_SEL_LITERAL_STRUCT, SYN_COLOR_LITERAL_STRUCT_FGD, SYN_COLOR_SEL_BKGD);
    init_pair(SYN_COLOR_SEL_LITERAL_OP,     SYN_COLOR_LITERAL_OP_FGD,     SYN_COLOR_SEL_BKGD);
    init_pair(SYN_COLOR_SEL_LITERAL_DELIM,  SYN_COLOR_LITERAL_DELIM_FGD,  SYN_COLOR_SEL_BKGD);
    init_pair(SYN_COLOR_SEL_COMMENT,        SYN_COLOR_COMMENT_FGD,        SYN_COLOR_SEL_BKGD);
    init_pair(SYN_COLOR_SEL_MCOMMENT,       SYN_COLOR_COMMENT_FGD,        SYN_COLOR_SEL_BKGD);
    init_pair(SYN_COLOR_SEL_PREPROC,        SYN_COLOR_PREPROC_FGD,        SYN_COLOR_SEL_BKGD);
    init_pair(SYN_COLOR_SEL_STATUS,         SYN_COLOR_BLACK,              SYN_COLOR_SEL_BKGD);

    bkgd(COLOR_PAIR(SYN_COLOR_TEXT));   // set background color of window (stdscr?)
    
}

//---------------------------------------------------------------------------------------
void ncurses_find_selected_offsets(line_t *_line, size_t *start, size_t *end)
{
    *start = _line->len;
    *end = _line->len;

    size_t i = 0;
    bool in_selection = false;
    while (i <= _line->len)
    {
        if (CHECK_BIT(_line->content[i], CHTYPE_SELECTION_BIT) && !in_selection)
        {
            *start = i;
            in_selection = true;
        }
        if ((!CHECK_BIT(_line->content[i], CHTYPE_SELECTION_BIT) || i == _line->len) 
            && in_selection)
        {
            *end = i;
            break;
        }
        
        i++;
    }
}

//---------------------------------------------------------------------------------------
void ncurses_deselect_substr(line_t *_line, size_t _start, size_t _end)
{
    for (size_t i = _start; i < _end; i++)
    {
        // A_COLOR : 0x0000ff00 (masking bit 9..16)
        int16_t cp_idx = (_line->content[i] & CHTYPE_COLOR_MASK) >> 8;
        if (CHECK_BIT(_line->content[i], CHTYPE_SELECTION_BIT))
        {
            // need to clear the CHTYPE_COLOR_MASK first, since OR is applied
            _line->content[i] = ((_line->content[i] & ~CHTYPE_COLOR_MASK) | COLOR_PAIR(cp_idx - SELECTION_OFFSET));
            ZERO_BIT(_line->content[i], CHTYPE_SELECTION_BIT);
        }
        
    }

}

//---------------------------------------------------------------------------------------
void ncurses_toggle_selection_substr(line_t *_line, size_t _start, size_t _end)
{
    for (size_t i = _start; i < _end; i++)
    {
        // A_COLOR : 0x0000ff00 (masking bit 9..16)
        int16_t cp_idx = (_line->content[i] & CHTYPE_COLOR_MASK) >> 8;
        if (!CHECK_BIT(_line->content[i], CHTYPE_SELECTION_BIT))
        {
            // need to clear the CHTYPE_COLOR_MASK first, since OR is applied
            _line->content[i] = ((_line->content[i] & ~CHTYPE_COLOR_MASK) | COLOR_PAIR(cp_idx + SELECTION_OFFSET));
            SET_BIT(_line->content[i], CHTYPE_SELECTION_BIT);
        }
        else
        {
            // need to clear the CHTYPE_COLOR_MASK first, since OR is applied
            _line->content[i] = ((_line->content[i] & ~CHTYPE_COLOR_MASK) | COLOR_PAIR(cp_idx - SELECTION_OFFSET));
            ZERO_BIT(_line->content[i], CHTYPE_SELECTION_BIT);
        }
        
    }
}

//---------------------------------------------------------------------------------------
void ncurses_color_substr(line_t *_line, size_t _start, size_t _end, short _pair_index)
{
    // assert(_start >= 0 && _end <= _line->len);
    for (size_t i = _start; i < _end; i++)
      _line->content[i] = ((_line->content[i] & ~CHTYPE_COLOR_MASK) | COLOR_PAIR(_pair_index));

}

//---------------------------------------------------------------------------------------
int16_t ncurses_get_CHTYPE_color(CHTYPE _c)
{
    return (_c & CHTYPE_COLOR_MASK) >> 8;
    
}
