#include <util.h>
#include <gpiod_frequency_counter.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static double timespec_to_double(const struct timespec *tv) {
	return (double)tv->tv_sec + 1e-9 * tv->tv_nsec;
}

static void timespec_diff(
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

static int timespec_ge(
	const struct timespec *x,
	const struct timespec *y
) {
	return (x->tv_sec > y->tv_sec
	        || (x->tv_sec == y->tv_sec && x->tv_nsec >= y->tv_nsec));
}


int gpiod_frequency_counter_init(
	gpiod_frequency_counter *self,
	struct gpiod_line *line,
	int buf_size
) {
	void *buf = calloc(buf_size, sizeof(*self->period));
	if (!buf) {
		return -1;
	}
	self->line = line;
	self->period_buf_size = buf_size;
	self->period_buf_offset = 0;
	self->period = buf;
	return 0;
}

void gpiod_frequency_counter_destroy(gpiod_frequency_counter *self) {
	if (self->line) {
		gpiod_line_release(self->line);
		self->line = NULL;
	}
	if (self->period) {
		free(self->period);
		self->period = NULL;
	}
}

void gpiod_frequency_counter_reset(gpiod_frequency_counter *self) {
	memset(self->period, 0, self->period_buf_size * sizeof(*self->period));
	self->period_buf_offset = 0;
	gpiod_line_release(self->line);
}

int gpiod_frequency_counter_count(
	gpiod_frequency_counter *self,
	int waves,
	struct timespec *max_time
) {
	int rc = 0;

	rc = gpiod_line_request_rising_edge_events(
		self->line,
		"gpiod_frequency_counter"
	);
	if (rc) {
		dbg("gpiod_line_request_rising_edge_events: %s\n", strerror(errno));
		return -1;
	}

	struct timespec timeout;
	struct timespec start;
	struct timespec last_event_time = {0};
	int first_event = 1;
	int skip_events = 1;
	int wave = 0;

	if (waves == 0) {
		waves = self->period_buf_size;
	}
	if (max_time) {
		timeout = *max_time;
	} else {
		timeout.tv_sec = 1;
		timeout.tv_nsec = 0;
	}

	clock_gettime(CLOCK_MONOTONIC, &start);

	while (1) {
		rc = gpiod_line_event_wait(self->line, &timeout);

		if (rc < 0) {
			dbg("gpiod_line_event_wait: %s\n", strerror(errno));
			rc = -1;
			break;
		}
		if (rc > 0) {
			struct gpiod_line_event ev;
			if ((rc = gpiod_line_event_read(self->line, &ev))) {
				dbg("gpiod_line_event_read: %s\n", strerror(errno));
				rc = -1;
				break;
			}
			if (skip_events) {
				--skip_events;
			} else {
				if (first_event) {
					first_event = 0;
				} else {
					struct timespec period;
					timespec_diff(&last_event_time, &ev.ts, &period);
					double p = timespec_to_double(&period);
					self->period[self->period_buf_offset] = p;
					++self->period_buf_offset;
					self->period_buf_offset %= self->period_buf_size;
					dbg("period: %.04lfs\n", p);
					if (++wave == waves) {
						rc = 0;
						break;
					}
				}
				last_event_time = ev.ts;
			}
			dbg_timespec("event", ev.ts);
		}

		if (max_time) {
			struct timespec time, elapsed;
			clock_gettime(CLOCK_MONOTONIC, &time);
			timespec_diff(&start, &time, &elapsed);
			dbg_timespec("elapsed", elapsed);
			if(timespec_ge(&elapsed, max_time)) {
				dbg_timespec("last event", last_event_time);
				rc = 0;
				break;
			}
			else {
				timespec_diff(&elapsed, max_time, &timeout);
			}
		}
	}

	gpiod_line_release(self->line);
	return rc;
}

double gpiod_frequency_counter_get_period(gpiod_frequency_counter *self) {
	int count = 0;
	double sum = 0.0;
	dbg("[ ");
	for (int i = 0; i < self->period_buf_size; ++i) {
		dbg("%.04lf, ", self->period[i]);
		if (self->period[i] > 0.0) {
			++count;
			sum += self->period[i];
		}
	}
	dbg("]\n");
	if (count == 0) {
		return INFINITY;
	}
	return sum / count;
}

double gpiod_frequency_counter_get_frequency(gpiod_frequency_counter *self) {
	double period = gpiod_frequency_counter_get_period(self);
	if (period == INFINITY) {
		return 0.0;
	}
	return 1.0 / period;
}
