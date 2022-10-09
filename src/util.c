#include <util.h>

#include <math.h>
#include <string.h>
#include <errno.h>

/*double timespec_to_double(const struct timespec *tv) {
	return (double)tv->tv_sec + 1e-9 * tv->tv_nsec;
}*/

void timespec_diff(
	const struct timespec *start,
	const struct timespec *stop,
	struct timespec *res
) {
	if (start->tv_nsec > stop->tv_nsec) {
		res->tv_sec = stop->tv_sec - start->tv_sec - 1;
		res->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
	} else {
		res->tv_sec = stop->tv_sec - start->tv_sec;
		res->tv_nsec = stop->tv_nsec - start->tv_nsec;
	}
}

double timespec_diff_double(
	const struct timespec *start,
	const struct timespec *stop
) {
	struct timespec tmp;
	timespec_diff(start, stop, &tmp);
	return timespec_to_double(tmp);
}

int timespec_gt(
	const struct timespec *x,
	const struct timespec *y
) {
	return (x->tv_sec > y->tv_sec
	        || (x->tv_sec == y->tv_sec && x->tv_nsec > y->tv_nsec));
}

int timespec_ge(
	const struct timespec *x,
	const struct timespec *y
) {
	return (x->tv_sec > y->tv_sec
	        || (x->tv_sec == y->tv_sec && x->tv_nsec >= y->tv_nsec));
}

int update_timeout(
	const struct timespec *start,
	const struct timespec *timeout,
	struct timespec *remaining
) {
	if (!timeout) {
		return 0;
	}
	struct timespec time, elapsed;
	clock_gettime(CLOCK_MONOTONIC, &time);
	timespec_diff(start, &time, &elapsed);
	dbg_timespec("elapsed", elapsed);
	if(timespec_ge(&elapsed, timeout)) {
		remaining->tv_sec = 0;
		remaining->tv_nsec = 0;
		return 1;
	}
	timespec_diff(&elapsed, timeout, remaining);
	return 0;
}

int read_event(
	struct gpiod_line *line,
	struct gpiod_line_event *ev,
	const struct timespec *timeout
) {
	int rc = gpiod_line_event_wait(line, timeout);
	if (rc < 0) {
		dbg("gpiod_line_event_wait: %s\n", strerror(errno));
		return -1;
	}
	if (rc > 0) {
		int rc_ = gpiod_line_event_read(line, ev);
		if (rc_ < 0) {
			dbg("gpiod_line_event_read: %s\n", strerror(errno));
		}
	}
	return rc;
}

double get_period(double *buf, int size) {
	double sum = 0.0;
	int count = 0;
	dbg("[ ");
	for (int i = 0; i < size; ++i) {
		dbg("%.04lf, ", buf[i]);
		if (buf[i] > 0.0) {
			++count;
			sum += buf[i];
		}
	}
	dbg("]\n");
	if (count == 0) {
		return INFINITY;
	}
	return sum / count;
}
