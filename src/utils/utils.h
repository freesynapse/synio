#ifndef __UTILS_H
#define __UTILS_H

//
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) MIN((hi), MAX((lo), (x)))

//
#define RETURN_SUCCESS  0
#define RETURN_FAILURE  1

#define EXIT_SUCCESS    0
#define EXIT_FAILURE    1


#endif // __UTILS_H
