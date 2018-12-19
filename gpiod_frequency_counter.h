#ifndef GPIOD_FREQUENCY_COUNTER_H_INCLUDED
#define GPIOD_FREQUENCY_COUNTER_H_INCLUDED

#include <time.h>
#include <pthread.h>
#include <gpiod.h>

typedef struct gpiod_frequency_counter {
	struct gpiod_line *line;
	pthread_t thread;
	pthread_mutex_t mutex;
	struct timespec interval;
	double period;
	double frequency;
} gpiod_frequency_counter;

int gpiod_frequency_counter_init(
	gpiod_frequency_counter *self,
	struct gpiod_line *line,
	struct timespec interval
);
void gpiod_frequency_counter_destroy(gpiod_frequency_counter *self);

double gpiod_frequency_counter_get_period(gpiod_frequency_counter *self);
double gpiod_frequency_counter_get_frequency(gpiod_frequency_counter *self);

#endif
