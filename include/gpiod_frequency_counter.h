#ifndef GPIOD_FREQUENCY_COUNTER_H_INCLUDED
#define GPIOD_FREQUENCY_COUNTER_H_INCLUDED

#include <time.h>
#include <gpiod.h>

typedef struct gpiod_frequency_counter {
	struct gpiod_line *line;
	double *period;
	int period_buf_size;
	int period_buf_offset;
} gpiod_frequency_counter;

int gpiod_frequency_counter_init(
	gpiod_frequency_counter *self,
	struct gpiod_line *line,
	int buf_size
);
void gpiod_frequency_counter_destroy(gpiod_frequency_counter *self);
void gpiod_frequency_counter_reset(gpiod_frequency_counter *self);

int gpiod_frequency_counter_count(
	gpiod_frequency_counter *self,
	int waves,
	struct timespec *max_time
);

double gpiod_frequency_counter_get_period(gpiod_frequency_counter *self);
double gpiod_frequency_counter_get_frequency(gpiod_frequency_counter *self);

#endif
