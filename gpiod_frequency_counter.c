#include "gpiod_frequency_counter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


static void timespec_diff(
    const struct timespec *start,
	const struct timespec *stop,
	struct timespec *res
) {
	if(start->tv_nsec > stop->tv_nsec) {
		res->tv_sec = stop->tv_sec - start->tv_sec - 1;
		res->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
	}
	else {
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


static void* gpiod_frequency_counter_thread(void *counter) {
	gpiod_frequency_counter *self = counter;
	struct timespec timeout = self->interval;
	struct timespec prev;
	struct timespec time;
	unsigned long count = 0;

	clock_gettime(CLOCK_MONOTONIC, &prev);

	while(1) {
		int rc = gpiod_line_event_wait(self->line, &timeout);
		if(rc < 0) {
			fprintf(stderr, "gpiod_line_event_wait: %s\n", strerror(errno));
		}
		else if(rc > 0) {
			++count;
			struct gpiod_line_event ev;
			if(gpiod_line_event_read(self->line, &ev)) {
				fprintf(stderr, "gpiod_line_event_read: %s\n", strerror(errno));
			}
		}

		clock_gettime(CLOCK_MONOTONIC, &time);
		struct timespec tmp;
		timespec_diff(&prev, &time, &tmp);

		pthread_mutex_lock(&self->mutex);

		if(timespec_ge(&tmp, &self->interval)) {
			prev = time;
			timeout = self->interval;
			self->period = 2.0 * (double)self->interval.tv_sec / count;
			self->period += 2e-9 * (double)self->interval.tv_nsec / count;
			self->frequency = 1.0 / self->period;
			count = 0;
		}
		else {
			timespec_diff(&tmp, &self->interval, &timeout);
		}

		pthread_mutex_unlock(&self->mutex);
	}
	return NULL;
}


int gpiod_frequency_counter_init(
	gpiod_frequency_counter *self,
	struct gpiod_line *line,
	struct timespec interval
) {
	pthread_attr_t attr;
	self->thread = 0;
	self->line = line;
	self->interval = interval;
	self->frequency = 0.0;
	self->period = 0.0;
	if(gpiod_line_request_both_edges_events(
		self->line,
		"gpiod_frequency_counter"
	)) {
		return -1;
	}
	if(pthread_mutex_init(&self->mutex, NULL)) {
		return -1;
	}
	if(pthread_attr_init(&attr)) {
		return -1;
	}
	if(pthread_create(&self->thread, &attr,
	                  &gpiod_frequency_counter_thread, self)) {
		return -1;
	}
	return 0;
}

void gpiod_frequency_counter_destroy(gpiod_frequency_counter *self) {
	gpiod_line_release(self->line);
	if(self->thread) {
		pthread_cancel(self->thread);
		pthread_join(self->thread, NULL);
		pthread_mutex_destroy(&self->mutex);
	}
}

double gpiod_frequency_counter_get_period(gpiod_frequency_counter *self) {
	double ret;
	pthread_mutex_lock(&self->mutex);
	ret = self->period;
	pthread_mutex_unlock(&self->mutex);
	return ret;
}

double gpiod_frequency_counter_get_frequency(gpiod_frequency_counter *self) {
	double ret;
	pthread_mutex_lock(&self->mutex);
	ret = self->frequency;
	pthread_mutex_unlock(&self->mutex);
	return ret;
}
