#ifndef __NCURSES_COLORS_H
#define __NCURSES_COLORS_H

#include <stddef.h>
#include <ncurses.h>

#include "../core.h"
#include "../types.h"

// individual color ids
#define SYNIO_COLOR_BKGD                -1  // assumes use_default_colors() is called
#define SYNIO_COLOR_SELECTED_BKGD        1
#define SYNIO_COLOR_TEXT_FGD             2
#define SYNIO_COLOR_KEYWORD_FGD          3
#define SYNIO_COLOR_STRING_FGD           4
#define SYNIO_COLOR_NUMBER_FGD           5
#define SYNIO_COLOR_DELIM_FGD            6
#define SYNIO_COLOR_COMMENT_FGD          7

#define SYNIO_COLOR_COUNT                8

// color pair ids
#define SYNIO_COLOR_TEXT                 0
#define SYNIO_COLOR_KEYWORD              1
#define SYNIO_COLOR_STRING               2
#define SYNIO_COLOR_NUMBER               3
#define SYNIO_COLOR_DELIM                4
#define SYNIO_COLOR_COMMENT              5

#define SYNIO_COLOR_PAIR_COUNT           6

// selection color pair ids
#define SELECTION_OFFSET                (256 - SYNIO_COLOR_PAIR_COUNT)
#define SYNIO_COLOR_SELECTED_TEXT       (SYNIO_COLOR_TEXT    + SELECTION_OFFSET)
#define SYNIO_COLOR_SELECTED_KEYWORD    (SYNIO_COLOR_KEYWORD + SELECTION_OFFSET)
#define SYNIO_COLOR_SELECTED_STRING     (SYNIO_COLOR_STRING  + SELECTION_OFFSET)
#define SYNIO_COLOR_SELECTED_NUMBER     (SYNIO_COLOR_NUMBER  + SELECTION_OFFSET)
#define SYNIO_COLOR_SELECTED_DELIM      (SYNIO_COLOR_DELIM   + SELECTION_OFFSET)
#define SYNIO_COLOR_SELECTED_COMMENT    (SYNIO_COLOR_COMMENT + SELECTION_OFFSET)


//
extern void init_colors(API_WINDOW_PTR _w);
extern void select_substr(line_t *_line, size_t _start, size_t _end);
extern void color_substr(line_t *_line, size_t _start, size_t _end, short _pair_index);

//#ifdef DEBUG
//#include <assert.h>
//void __debug_set_substr_color(line_t *_line, size_t _start, size_t _end, short _color_pair_index)
//{
//    assert(_start > 0 && _end <= _line->len);
//    for (size_t i = _start; i < _end; i++)
//        _line->content[i] = (_line->content[i] | COLOR_PAIR(_color_pair_index));
//
//}
//#endif

#endif // __NCURSES_COLORS_H
