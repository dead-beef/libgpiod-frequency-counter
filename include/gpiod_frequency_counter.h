#ifndef GPIOD_FREQUENCY_COUNTER_H_INCLUDED
#define GPIOD_FREQUENCY_COUNTER_H_INCLUDED

#include <time.h>
#include <gpiod.h>

typedef struct gpiod_frequency_counter {
	struct gpiod_line *line;
	size_t period_buf_size;
	double *period_buf[2];
	size_t period_buf_offset[2];
	double period[2];
} gpiod_frequency_counter;

int gpiod_frequency_counter_init(
	gpiod_frequency_counter *self,
	struct gpiod_line *line,
	size_t buf_size
);
void gpiod_frequency_counter_destroy(gpiod_frequency_counter *self);
void gpiod_frequency_counter_reset(gpiod_frequency_counter *self);

int gpiod_frequency_counter_count(
	gpiod_frequency_counter *self,
	int waves,
	const struct timespec *timeout
);

double gpiod_frequency_counter_get_period(gpiod_frequency_counter *self);
double gpiod_frequency_counter_get_frequency(gpiod_frequency_counter *self);

double gpiod_frequency_counter_get_high_period(gpiod_frequency_counter *self);
double gpiod_frequency_counter_get_low_period(gpiod_frequency_counter *self);
double gpiod_frequency_counter_get_duty_cycle(gpiod_frequency_counter *self);

const char *gpiod_frequency_counter_version_string();

#endif
