#ifndef __PLATFORM_H
#define __PLATFORM_H

#if defined NCURSES_IMPL
#include "ncurses_impl.h"
#define CTRL(x) ((x) & 0x1f)
#elif defined(GLFW_IMPL)
#include "glfw_impl.h"
#endif



#endif // __PLATFORM_H
