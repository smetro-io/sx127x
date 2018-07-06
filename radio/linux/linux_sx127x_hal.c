/*
 * Copyright (c) 2017 Smetro Technologies
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "sx127x_hal.h"
#include "timer.h"
#include "spi.h"
#include "gpio.h"
#include "log.h"

void sx127x_log_print(sx127x_log_t type, const char *fmt, ...)
{
	va_list args;
    char dest[1024];
	va_start(args, fmt);
    vsprintf(dest, fmt, args);
    va_end(args);
    LOG(type, (dest));
}

void sx127x_timer_set(sx127x_timer_t* timer, int timeout, void *cb_arg)
{
	timer->id = linux_timer_set(timeout, timer->callback, cb_arg);
}

void sx127x_timer_disable(sx127x_timer_t* timer)
{
	linux_timer_disable(timer->id);
}

void sx127x_timer_usleep(uint32_t usecs)
{
    usleep(usecs);
}

void sx127x_timer_msleep(uint32_t msecs)
{
    usleep(msecs * 1000);
}

void sx127x_spi_write(void *spi, uint8_t addr, uint8_t *buffer, uint8_t size)
{
	uint8_t tx_data[size + 1];
    tx_data[0] = addr;
    memcpy(tx_data + 1, buffer, size);

    struct spi_ioc_transfer spi_xfer[1];

    memset(spi_xfer, 0, sizeof(struct spi_ioc_transfer));
    spi_xfer[0].tx_buf = (unsigned long)tx_data;
    spi_xfer[0].len = size + 1;

    if (linux_spi_transfer(spi_xfer, 1) < 0) {
        sx127x_log_print(SX127X_ERROR, "Error during spi write\n");
    }
}

void sx127x_spi_read(void *spi, uint8_t addr, uint8_t *buffer, uint8_t size)
{
    struct spi_ioc_transfer spi_xfer[2];
    memset(spi_xfer, 0, 2 * sizeof(struct spi_ioc_transfer));

    spi_xfer[0].tx_buf = (unsigned long)&addr;
    spi_xfer[0].len = 1;

    spi_xfer[1].rx_buf = (unsigned long)buffer;
    spi_xfer[1].len = size;

    if (linux_spi_transfer(spi_xfer, 2) < 0) {
        sx127x_log_print(SX127X_ERROR, "Error during spi read\n");
    }
}

bool sx127x_gpio_init(int pin, sx127x_gpio_mode_t mode, sx127x_gpio_pull_t pull)
{
    switch (mode) {
    case GPIO_IN:
        if (!linux_gpio_set_mode(pin, LINUX_GPIO_MODE_INPUT)) return false;
        break;
    case GPIO_OUT:
        if (!linux_gpio_set_mode(pin, LINUX_GPIO_MODE_OUTPUT)) return false;
        break;
    }

    switch (pull) {
    case GPIO_PULL_NONE:
        if (!linux_gpio_set_pull(pin, LINUX_GPIO_PULL_NONE)) return false;
        break;
    case GPIO_PULL_UP:
        if (!linux_gpio_set_pull(pin, LINUX_GPIO_PULL_UP)) return false;
        break;
    case GPIO_PULL_DOWN:
        if (!linux_gpio_set_pull(pin, LINUX_GPIO_PULL_DOWN)) return false;
        break;
    }
    return true;
}

bool sx127x_gpio_init_int(int pin, sx127x_gpio_mode_t mode,
    sx127x_gpio_pull_t pull, sx127x_gpio_int_mode_t int_mode,
    sx127x_gpio_handler cb, void *arg)
{
    if (!sx127x_gpio_init(pin, mode, pull)) return false;
    switch(int_mode) {
    case GPIO_RISING:
        if (!linux_gpio_set_int_handler(pin, LINUX_GPIO_INT_EDGE_POS, cb, arg)) {
            return false;
        }
        break;
    }
    return true;
}

void sx127x_gpio_clear(int pin)
{
    linux_gpio_write(pin, 0);
}
