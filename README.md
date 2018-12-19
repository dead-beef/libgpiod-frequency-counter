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
tar czvf ../libgpiod-frequency-counter_1.0.orig.tar.gz .
debuild -us -uc
sudo dpkg -i ../libgpiod-frequency-counter_1.0*.deb
```

## Usage

```c
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <gpiod.h>
#include <gpiod_frequency_counter.h>

int main() {
	enum { INPUT_PIN = 4 };

	struct gpiod_chip *chip = NULL;
	struct gpiod_line *input_line = NULL;
	gpiod_frequency_counter counter;
	struct timespec interval;

	interval.tv_sec = 0;
	interval.tv_nsec = 500000000;

	chip = gpiod_chip_open("/dev/gpiochip0");
    input_line = gpiod_chip_get_line(chip, INPUT_PIN);

	gpiod_frequency_counter_init(&counter, input_line, interval);

	while(1) {
		for(int i = 0; i < 80; ++i, putchar('\b'));
		printf("period=%.06fs frequency=%.06fHz   ",
		       gpiod_frequency_counter_get_period(&counter),
		       gpiod_frequency_counter_get_frequency(&counter)
		);
		fflush(stdout);
		usleep(1000000);
	}

	gpiod_frequency_counter_destroy(&counter);
	return 0;
}
```

## Licenses

* [`libgpiod-frequency-counter`](https://github.com/dead-beef/libgpiod-frequency-counter/blob/master/LICENSE)
