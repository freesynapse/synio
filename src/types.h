#ifndef __TYPES_H
#define __TYPES_H


//
typedef struct ivec2_t
{
    int x = 0;
    int y = 0;
    
} ivec2_t;

//
typedef struct irect_t
{
    ivec2_t v0; // top-left
    ivec2_t v1; // bottom-right

} irect_t;


#endif // __TYPES_H