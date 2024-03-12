#ifndef __NCURSES_COLORS_H
#define __NCURSES_COLORS_H

#include <stddef.h>
#include <ncurses.h>

#include "../core.h"
#include "../types.h"

// individual color ids
#define SYNIO_COLOR_BKGD                -1  // assumes use_default_colors() is called
#define SYNIO_COLOR_SELECTED_BKGD        0
// #define SYNIO_COLOR_MENU_BKGD            1  // TODO : implement me...
#define SYNIO_COLOR_STATUS_BKGD          1
#define SYNIO_COLOR_TEXT_FGD             2
#define SYNIO_COLOR_KEYWORD_FGD          3
#define SYNIO_COLOR_STRING_FGD           4
#define SYNIO_COLOR_NUMBER_FGD           5
#define SYNIO_COLOR_DELIM_FGD            6
#define SYNIO_COLOR_COMMENT_FGD          7
#define SYNIO_COLOR_BLACK                8

#define SYNIO_COLOR_COUNT                9

// color pair ids
#define SYNIO_COLOR_TEXT                 0
#define SYNIO_COLOR_KEYWORD              1
#define SYNIO_COLOR_STRING               2
#define SYNIO_COLOR_NUMBER               3
#define SYNIO_COLOR_DELIM                4
#define SYNIO_COLOR_COMMENT              5
#define SYNIO_COLOR_STATUS               6

#define SYNIO_COLOR_PAIR_COUNT           7

// selection color pair ids
#define SELECTION_OFFSET                (255 - SYNIO_COLOR_PAIR_COUNT)
#define SYNIO_COLOR_SELECTED_TEXT       (SYNIO_COLOR_TEXT    + SELECTION_OFFSET)
#define SYNIO_COLOR_SELECTED_KEYWORD    (SYNIO_COLOR_KEYWORD + SELECTION_OFFSET)
#define SYNIO_COLOR_SELECTED_STRING     (SYNIO_COLOR_STRING  + SELECTION_OFFSET)
#define SYNIO_COLOR_SELECTED_NUMBER     (SYNIO_COLOR_NUMBER  + SELECTION_OFFSET)
#define SYNIO_COLOR_SELECTED_DELIM      (SYNIO_COLOR_DELIM   + SELECTION_OFFSET)
#define SYNIO_COLOR_SELECTED_COMMENT    (SYNIO_COLOR_COMMENT + SELECTION_OFFSET)

//
extern void ncurses_init_colors(API_WINDOW_PTR _w);

extern void ncurses_find_selected_offsets(line_t *_line, size_t *start, size_t *end);
extern void ncurses_deselect_substr(line_t *_line, size_t _start, size_t _end);
extern void ncurses_toggle_selection_substr(line_t *_line, size_t _start, size_t _end);

extern void ncurses_color_substr(line_t *_line, size_t _start, size_t _end, short _pair_index);


#endif // __NCURSES_COLORS_H
