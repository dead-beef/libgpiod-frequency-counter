# libgpiod-frequency-counter

[![license](https://img.shields.io/github/license/dead-beef/libgpiod-frequency-counter.svg)](
    https://github.com/dead-beef/libgpiod-frequency-counter/blob/master/LICENSE
)

## Overview

GPIO input frequency counter.

## Requirements

* [`libgpiod`](https://github.com/brgl/libgpiod)

## Installation

```
sudo apt-get install debhelper build-essential libgpiod-dev
git clone https://github.com/dead-beef/libgpiod-frequency-counter.git
cd libgpiod-frequency-counter
make pkg
sudo dpkg -i ../libgpiod-frequency-counter_*.deb
```

## Usage

### CLI

```
> gpio-get-frequency -h
Usage: gpio-get-frequency [-h] [-i <time>] [-b <size>] <chip name/number> <offset>

Options:
    -h, --help               print this help text and exit
    -i, --interval <time>    maximum time in seconds (default: none)
    -b, --buf-size <size>    period buffer size (default: 32)
```

### Library

```c
#include <stdio.h>
#include <time.h>
#include <gpiod.h>
#include <gpiod_frequency_counter.h>

int main() {
	enum {
		INPUT_PIN = 4,
		BUF_SIZE = 32
	};

	struct gpiod_chip *chip = NULL;
	struct gpiod_line *input_line = NULL;
	gpiod_frequency_counter counter;
	struct timespec interval;

	interval.tv_sec = 0;
	interval.tv_nsec = 500000000;

	chip = gpiod_chip_open("/dev/gpiochip0");
	input_line = gpiod_chip_get_line(chip, INPUT_PIN);

	gpiod_frequency_counter_init(&counter, input_line, BUF_SIZE);

	while (1) {
		gpiod_frequency_counter_count(&counter, BUF_SIZE, &interval);
		double period = gpiod_frequency_counter_get_period(&counter);
		double frequency = gpiod_frequency_counter_get_frequency(&counter);

		for(int i = 0; i < 80; ++i, putchar('\b'));
		printf("period=%.06fs frequency=%.06fHz   ", period, frequency);
		fflush(stdout);
	}

	gpiod_frequency_counter_destroy(&counter);
	gpiod_chip_close(chip);
	return 0;
}
```

## Licenses

* [`libgpiod-frequency-counter`](https://github.com/dead-beef/libgpiod-frequency-counter/blob/master/LICENSE)
