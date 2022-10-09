#include <util.h>
#include <gpiod_frequency_counter.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

EXPORT int gpiod_frequency_counter_init(
	gpiod_frequency_counter *self,
	struct gpiod_line *line,
	size_t buf_size
) {
	self->line = line;
	self->period_buf_size = buf_size;
	memset(self->period_buf, 0, sizeof(self->period_buf));
	memset(self->period_buf_offset, 0, sizeof(self->period_buf_offset));
	for (int i = 0; i < 2; ++i) {
		self->period[i] = INFINITY;
		self->period_buf[i] = calloc(buf_size, sizeof(**self->period_buf));
		if (!self->period_buf[i]) {
			gpiod_frequency_counter_destroy(self);
			return -1;
		}
	}
	return 0;
}

EXPORT void gpiod_frequency_counter_destroy(gpiod_frequency_counter *self) {
	if (self->line) {
		//gpiod_line_release(self->line);
		self->line = NULL;
	}
	for (int i = 0; i < 2; ++i) {
		if (self->period_buf[i]) {
			free(self->period_buf[i]);
			self->period_buf[i] = NULL;
		}
	}
}

EXPORT void gpiod_frequency_counter_reset(gpiod_frequency_counter *self) {
	size_t buf_size = self->period_buf_size * sizeof(**self->period_buf);
	for (int i = 0; i < 2; ++i) {
		self->period[i] = INFINITY;
		memset(self->period_buf[i], 0, buf_size);
	}
	memset(self->period_buf_offset, 0, sizeof(self->period_buf_offset));
	//gpiod_line_release(self->line);
}

EXPORT int gpiod_frequency_counter_count(
	gpiod_frequency_counter *self,
	int waves,
	const struct timespec *timeout
) {
	int rc = 0;

	rc = gpiod_line_request_both_edges_events(
		self->line,
		"gpiod_frequency_counter"
	);
	if (rc) {
		dbg("gpiod_line_request_both_edges_events: %s\n", strerror(errno));
		return -1;
	}

	struct timespec start;
	struct timespec remaining_timeout;
	struct timespec *remaining_timeout_ptr = NULL;
	clock_gettime(CLOCK_MONOTONIC, &start);
	dbg_timespec("start", start);
	if (timeout) {
		remaining_timeout = *timeout;
		remaining_timeout_ptr = &remaining_timeout;
	}

	if (waves == 0) {
		waves = self->period_buf_size;
	}
	int events = waves * 2;

	struct gpiod_line_event prev;

	while (1) {
		rc = read_event(self->line, &prev, remaining_timeout_ptr);
		if (rc <= 0) {
			goto end;
		}
		dbg_event("first event", prev);
		if (timespec_gt(&prev.ts, &start)) {
			break;
		}
		update_timeout(&start, timeout, remaining_timeout_ptr);
	}

	while (1) {
		struct gpiod_line_event ev;
		rc = read_event(self->line, &ev, remaining_timeout_ptr);
		if (rc <= 0) {
			goto end;
		}
		dbg_event("event", ev);

		double period = timespec_diff_double(&prev.ts, &ev.ts);
		int value = prev.event_type == GPIOD_LINE_EVENT_RISING_EDGE;
		prev = ev;
		dbg("period: %d %.04lfs\n", value, period);

		self->period_buf[value][self->period_buf_offset[value]] = period;
		++self->period_buf_offset[value];
		self->period_buf_offset[value] %= self->period_buf_size;

		if (--events == 0) {
			rc = 0;
			break;
		}
		update_timeout(&start, timeout, remaining_timeout_ptr);
	}

end:
	for (int i = 0; i < 2; ++i) {
		self->period[i] = get_period(
			self->period_buf[i],
			self->period_buf_size
		);
	}

	gpiod_line_release(self->line);
	return rc;
}

EXPORT double gpiod_frequency_counter_get_period(
	gpiod_frequency_counter *self
) {
	return self->period[0] + self->period[1];
}

EXPORT double gpiod_frequency_counter_get_frequency(
	gpiod_frequency_counter *self
) {
	double period = gpiod_frequency_counter_get_period(self);
	if (period == INFINITY) {
		return 0.0;
	}
	if (period == 0.0) {
		return INFINITY;
	}
	return 1.0 / period;
}

EXPORT double gpiod_frequency_counter_get_high_period(
	gpiod_frequency_counter *self
) {
	return self->period[1];
}

EXPORT double gpiod_frequency_counter_get_low_period(
	gpiod_frequency_counter *self
) {
	return self->period[0];
}

EXPORT double gpiod_frequency_counter_get_duty_cycle(
	gpiod_frequency_counter *self
) {
	double period = gpiod_frequency_counter_get_period(self);
	if (period == 0.0 || period == INFINITY) {
		return 1.0;
	}
	double high = gpiod_frequency_counter_get_high_period(self);
	return high / period;
}

EXPORT const char* gpiod_frequency_counter_version_string() {
	return VERSION_STR;
}
