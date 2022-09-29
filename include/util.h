#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include "config.h"

#include <stdio.h>

#if DEBUG == 0
#define dbg(...)
#else
#define dbg(...) fprintf(stderr, __VA_ARGS__)
#endif

#define dbg_timespec(msg, tv) dbg(msg ": %.04lfs\n", (double)(tv).tv_sec + (1e-9 * (tv).tv_nsec))

#endif
