#include <util.h>
#include <gpiod_frequency_counter.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gpiod.h>
#include <unistd.h>

typedef struct arguments {
	const char *chip;
	unsigned long line;
	int buf_size;
	struct timespec *interval;
	struct timespec _interval;
} arguments;

void init_args(struct arguments *args) {
	args->chip = "0";
	args->line = 0;
	args->buf_size = 32;
	args->interval = NULL;
	args->_interval.tv_sec = 0;
	args->_interval.tv_nsec = 0;
}

void print_help(const char *name) {
	struct arguments args;
	init_args(&args);
	fprintf(
		stderr,
		"Usage: %s [-h] [-i <time>] [-b <size>] <chip name/number> <offset>\n"
		"\n"
		"Options:\n"
		"    -h, --help               print this help text and exit\n"
		"    -i, --interval <time>    maximum time in seconds (default: none)\n"
		"    -b, --buf-size <size>    period buffer size (default: %d)\n",
		name,
		args.buf_size
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
	double frequency = gpiod_frequency_counter_get_frequency(&counter);
	printf("%.04lf\n", frequency);

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
