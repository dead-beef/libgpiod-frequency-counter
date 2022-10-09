#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include "config.h"

#include <stdio.h>
#include <time.h>
#include <gpiod.h>

#define EXPORT __attribute__((visibility("default")))

#if DEBUG == 0
#define dbg(...)
#else
#define dbg(...) fprintf(stderr, __VA_ARGS__)
#endif

#define timespec_to_double(ts) ((double)(ts).tv_sec + (1e-9 * (ts).tv_nsec))

#define dbg_timespec(msg, ts) dbg(msg ": %.04lfs\n", timespec_to_double(ts))
#define dbg_event(msg, ev) dbg(msg ": %d %.04lfs\n", (ev).event_type == GPIOD_LINE_EVENT_RISING_EDGE, timespec_to_double((ev).ts))

//double timespec_to_double(const struct timespec *tv);

void timespec_diff(
	const struct timespec *start,
	const struct timespec *stop,
	struct timespec *res
);
double timespec_diff_double(
	const struct timespec *start,
	const struct timespec *stop
);
int timespec_gt(
	const struct timespec *x,
	const struct timespec *y
);
int timespec_ge(
	const struct timespec *x,
	const struct timespec *y
);
int update_timeout(
	const struct timespec *start,
	const struct timespec *timeout,
	struct timespec *remaining
);

int read_event(
	struct gpiod_line *line,
	struct gpiod_line_event *ev,
	const struct timespec *timeout
);

double get_period(double *buf, int size);

#endif
