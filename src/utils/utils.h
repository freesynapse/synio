#ifndef __UTILS_H
#define __UTILS_H

//
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) MIN((hi), MAX((lo), (x)))

//
#ifndef RETURN_SUCCESS
#define RETURN_SUCCESS  0
#define RETURN_FAILURE  1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS    0
#define EXIT_FAILURE    1
#endif

#endif // __UTILS_H
