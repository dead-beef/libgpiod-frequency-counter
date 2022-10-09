#include <util.h>
#include <gpiod_frequency_counter.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gpiod.h>
#include <unistd.h>

enum {
	PRINT_FREQUENCY = 1,
	PRINT_PERIOD,
	PRINT_SPLIT_PERIOD,
	PRINT_DUTY_CYCLE,
	PRINT_ALL,
};

typedef struct arguments {
	const char *chip;
	const char *format;
	unsigned long line;
	int buf_size;
	int print;
	struct timespec *interval;
	struct timespec _interval;
} arguments;

void init_args(struct arguments *args) {
	args->chip = "0";
	args->format = "%.04lf";
	args->line = 0;
	args->buf_size = 32;
	args->print = PRINT_FREQUENCY;
	args->interval = NULL;
	args->_interval.tv_sec = 0;
	args->_interval.tv_nsec = 0;
}

void print_help(const char *name) {
	struct arguments args;
	init_args(&args);
	fprintf(
		stderr,
		"Usage: %s [-h] [-i <time>] [-b <size>] [-f <format>] [-p | -P | -d | -F | -a] <chip name/number> <offset>\n"
		"\n"
		"Options:\n"
		"    -h, --help               print this help text and exit\n"
		"    -i, --interval <time>    maximum time in seconds (default: none)\n"
		"    -b, --buf-size <size>    period buffer size (default: %d)\n"
		"    -f, --format <format>    output format string (defult: %s)\n"
		"    -p, --period             print period\n"
		"    -P, --split-period       print low and high periods\n"
		"    -d, --duty-cycle         print duty cycle\n"
		"    -F, --frequency          print frequency (default)\n"
		"    -a, --all                print all of the above\n",
		name,
		args.buf_size,
		args.format
	);
}

int parse_args(int argc, char **argv, struct arguments *args) {
	int i = 1;
	const char *arg;
	init_args(args);
	while (i < argc) {
		arg = argv[i];
		dbg("arg %d %s\n", i, arg);
		if (!strcmp(arg, "--")) {
			++i;
			break;
		} else if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
			print_help(argv[0]);
			return 1;
		} else if (!strcmp(arg, "-i") || !strcmp(arg, "--interval")) {
			++i;
			if (i >= argc) {
				goto missing_arg;
			}
			arg = argv[i++];
			double time = strtod(arg, NULL);
			if (time <= 0.0) {
				fprintf(
					stderr,
					"Interval must be greater than 0 (got %s)\n",
					arg
				);
				return 1;
			}
			unsigned long sec = time;
			args->_interval.tv_sec = sec,
			args->_interval.tv_nsec = (time - sec) * 1e9;
			args->interval = &args->_interval;
		} else if (!strcmp(arg, "-b") || !strcmp(arg, "--buf-size")) {
			++i;
			if (i >= argc) {
				goto missing_arg;
			}
			arg = argv[i++];
			int buf_size = atoi(arg);
			if (buf_size <= 0) {
				fprintf(
					stderr,
					"Buffer size must be greater than 0 (got %s)\n",
					arg
				);
				return 1;
			}
			args->buf_size = buf_size;
		} else if (!strcmp(arg, "-f") || !strcmp(arg, "--format")) {
			++i;
			if (i >= argc) {
				goto missing_arg;
			}
			args->format = argv[i++];
		} else if (!strcmp(arg, "-p") || !strcmp(arg, "--period")) {
			++i;
			args->print = PRINT_PERIOD;
		} else if (!strcmp(arg, "-P") || !strcmp(arg, "--split-period")) {
			++i;
			args->print = PRINT_SPLIT_PERIOD;
		} else if (!strcmp(arg, "-F") || !strcmp(arg, "--frequency")) {
			++i;
			args->print = PRINT_FREQUENCY;
		} else if (!strcmp(arg, "-d") || !strcmp(arg, "--duty-cycle")) {
			++i;
			args->print = PRINT_DUTY_CYCLE;
		} else if (!strcmp(arg, "-a") || !strcmp(arg, "--all")) {
			++i;
			args->print = PRINT_ALL;
		} else {
			break;
		}
	}
	if (argc - i != 2) {
		print_help(argv[0]);
		return 1;
	}
	args->chip = argv[i];
	args->line = strtoul(argv[i + 1], NULL, 0);
	return 0;
missing_arg:
	fprintf(stderr, "Option %s requires an argument\n", arg);
	return 1;
}

int main(int argc, char **argv) {
	struct gpiod_chip *chip = NULL;
	struct gpiod_line *line = NULL;
	gpiod_frequency_counter counter = {0};
	struct arguments args;

	if (parse_args(argc, argv, &args)) {
		goto error;
	}

	if (!(chip = gpiod_chip_open_lookup(args.chip))) {
		fprintf(
			stderr,
			"gpiod_chip_open(%s): %s\n",
			args.chip,
			strerror(errno)
		);
		goto error;
	}
	if (!(line = gpiod_chip_get_line(chip, args.line))) {
		fprintf(
			stderr,
			"gpiod_chip_get_line(%s, %lu): %s\n",
			args.chip,
			args.line,
			strerror(errno)
		);
		goto error;
	}

	if (gpiod_frequency_counter_init(&counter, line, args.buf_size)) {
		fprintf(stderr, "gpiod_frequency_counter_init: %s\n", strerror(errno));
		goto error;
	}
	if (gpiod_frequency_counter_count(&counter, 0, args.interval)) {
		fprintf(stderr, "gpiod_frequency_counter_count: %s\n", strerror(errno));
		goto error;
	}

	switch (args.print) {
		case PRINT_PERIOD:
			printf(
				args.format,
				gpiod_frequency_counter_get_period(&counter)
			);
			break;
		case PRINT_FREQUENCY:
			printf(
				args.format,
				gpiod_frequency_counter_get_frequency(&counter)
			);
			break;
		case PRINT_DUTY_CYCLE:
			printf(
				args.format,
				gpiod_frequency_counter_get_duty_cycle(&counter)
			);
			break;
		case PRINT_SPLIT_PERIOD:
			printf(
				args.format,
				gpiod_frequency_counter_get_low_period(&counter)
			);
			putchar(' ');
			printf(
				args.format,
				gpiod_frequency_counter_get_high_period(&counter)
			);
			break;
		case PRINT_ALL:
			fputs("frequency = ", stdout);
			printf(
				args.format,
				gpiod_frequency_counter_get_frequency(&counter)
			);
			fputs("\nperiod = ", stdout);
			printf(
				args.format,
				gpiod_frequency_counter_get_period(&counter)
			);
			fputs(" (low = ", stdout);
			printf(
				args.format,
				gpiod_frequency_counter_get_low_period(&counter)
			);
			fputs(" ; high = ", stdout);
			printf(
				args.format,
				gpiod_frequency_counter_get_high_period(&counter)
			);
			fputs(")\nduty cycle = ", stdout);
			printf(
				args.format,
				gpiod_frequency_counter_get_duty_cycle(&counter)
			);
	}

	putchar('\n');

	gpiod_frequency_counter_destroy(&counter);
	gpiod_chip_close(chip);
	return 0;

error:
	gpiod_frequency_counter_destroy(&counter);
	if(chip) {
		gpiod_chip_close(chip);
	}
	return 1;
}
