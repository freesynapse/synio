#ifndef __NCURSES_COLORS_H
#define __NCURSES_COLORS_H

#include <stddef.h>
#include <ncurses.h>

#include "../core.h"
#include "../types.h"

// individual color ids
#define SYN_COLOR_BKGD                  -1  // assumes use_default_colors() is called
#define SYN_COLOR_SEL_BKGD               0
// #define SYN_COLOR_MENU_BKGD           1  // TODO : implement me (?)
#define SYN_COLOR_STATUS_BKGD            1
#define SYN_COLOR_TEXT_FGD               2
#define SYN_COLOR_KEYWORD_FGD            3
#define SYN_COLOR_STRING_FGD             4
#define SYN_COLOR_NUMBER_FGD             5
#define SYN_COLOR_LITERAL_STRUCT_FGD     6
#define SYN_COLOR_LITERAL_OP_FGD         7
#define SYN_COLOR_LITERAL_DELIM_FGD      8
#define SYN_COLOR_COMMENT_FGD            9
#define SYN_COLOR_PREPROC_FGD           10
#define SYN_COLOR_BLACK                 11
#define SYN_COLOR_GREY                  12
#define SYN_COLOR_DARK_GREY             13

#define SYN_COLOR_COUNT                 14

// color pair ids
#define SYN_COLOR_TEXT                   0
#define SYN_COLOR_KEYWORD                1
#define SYN_COLOR_STRING                 2
#define SYN_COLOR_MSTRING                3
#define SYN_COLOR_NUMBER                 4
#define SYN_COLOR_LITERAL_STRUCT         5
#define SYN_COLOR_LITERAL_OP             6
#define SYN_COLOR_LITERAL_DELIM          7
#define SYN_COLOR_COMMENT                8
#define SYN_COLOR_MCOMMENT               9
#define SYN_COLOR_PREPROC               10
#define SYN_COLOR_STATUS                11
#define SYN_COLOR_INACTIVE              12
#define SYN_COLOR_LINENO_INACTIVE       13

#define COLOR_PAIR_COUNT                14

// selection color pair ids
#define SELECTION_OFFSET                (255 - COLOR_PAIR_COUNT)
#define SYN_COLOR_SEL_TEXT              (SYN_COLOR_TEXT           + SELECTION_OFFSET)
#define SYN_COLOR_SEL_KEYWORD           (SYN_COLOR_KEYWORD        + SELECTION_OFFSET)
#define SYN_COLOR_SEL_STRING            (SYN_COLOR_STRING         + SELECTION_OFFSET)
#define SYN_COLOR_SEL_MSTRING           (SYN_COLOR_MSTRING        + SELECTION_OFFSET)
#define SYN_COLOR_SEL_NUMBER            (SYN_COLOR_NUMBER         + SELECTION_OFFSET)
#define SYN_COLOR_SEL_LITERAL_STRUCT    (SYN_COLOR_LITERAL_STRUCT + SELECTION_OFFSET)
#define SYN_COLOR_SEL_LITERAL_OP        (SYN_COLOR_LITERAL_OP     + SELECTION_OFFSET)
#define SYN_COLOR_SEL_LITERAL_DELIM     (SYN_COLOR_LITERAL_DELIM  + SELECTION_OFFSET)
#define SYN_COLOR_SEL_COMMENT           (SYN_COLOR_COMMENT        + SELECTION_OFFSET)
#define SYN_COLOR_SEL_MCOMMENT          (SYN_COLOR_MCOMMENT       + SELECTION_OFFSET)
#define SYN_COLOR_SEL_PREPROC           (SYN_COLOR_PREPROC        + SELECTION_OFFSET)
#define SYN_COLOR_SEL_STATUS            (SYN_COLOR_STATUS         + SELECTION_OFFSET)


//
extern void ncurses_init_colors(API_WINDOW_PTR _w);

// selection
extern void ncurses_find_selected_offsets(line_t *_line, size_t *start, size_t *end);
extern void ncurses_deselect_substr(line_t *_line, size_t _start, size_t _end);
extern void ncurses_toggle_selection_substr(line_t *_line, size_t _start, size_t _end);

// keeps background for selection 
extern void ncurses_color_substr(line_t *_line, size_t _start, size_t _end, short _pair_index);
//
extern int16_t ncurses_get_CHTYPE_color(CHTYPE _c);


#endif // __NCURSES_COLORS_H
