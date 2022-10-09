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
sudo apt-get install debhelper build-essential libgpiod-dev python3-dev
git clone https://github.com/dead-beef/libgpiod-frequency-counter.git
cd libgpiod-frequency-counter
make pkg
sudo dpkg -i ../libgpiod-frequency-counter_0.3.0-1_*.deb ../libgpiod-frequency-counter-dev_0.3.0-1_*.deb ../gpiod-frequency_0.3.0-1_*.deb ../python3-libgpiod-frequency-counter_0.3.0-1_*.deb
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

### C

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
        double duty_cycle = gpiod_frequency_counter_get_duty_cycle(&counter);

        fputs("\x1b[2K\r", stdout);
        printf(
            "period=%.06lfs frequency=%.06lfHz duty_cycle=%.02lf ",
            period,
            frequency,
            duty_cycle
        );
        fflush(stdout);
    }

    gpiod_frequency_counter_destroy(&counter);
    gpiod_chip_close(chip);
    return 0;
}
```

### Python

```python
#!/usr/bin/env python3

import sys

from gpiod import Chip
from gpiod_frequency_counter import FrequencyCounter

chip = 'gpiochip0'
line = 4

with Chip(chip, Chip.OPEN_BY_NAME) as chip:
    line = chip.get_line(line)
    buf_size = 32
    timeout_sec = 1
    timeout_nsec = 0
    counter = FrequencyCounter(line, buf_size)
    try:
        while True:
            counter.count(buf_size, timeout_sec, timeout_nsec)
            print(
                '\x1b[2K\r'
                f'period={counter.period:.04f}s '
                f'frequency={counter.frequency:.04f}Hz '
                f'duty_cycle={counter.duty_cycle:.02f} ',
                end=''
            )
            sys.stdout.flush()
    except KeyboardInterrupt:
        pass
```

## Licenses

* [`libgpiod-frequency-counter`](https://github.com/dead-beef/libgpiod-frequency-counter/blob/master/LICENSE)
