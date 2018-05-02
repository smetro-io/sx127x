/*
 * Copyright (c) 2017 Smetro Technologies
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include <stdarg.h>
#include <stdio.h>

#include "mgos.h"
#include "mgos_timers.h"
#include "mgos_gpio.h"
#include "mgos_hal.h"
#include "mgos_spi.h"

#include "sx127x_hal.h"

#define MGOS_SPI_SPEED    10000000
#define MGOS_SPI_MODE     0

void sx127x_log(sx127x_log_t type, const char *fmt, ...)
{
	va_list args;
    char dest[1024];
	va_start(args, fmt);
    vsprintf(dest, fmt, args);
    va_end(args);
	switch (type) {
	case SX127X_ERROR:
		LOG(LL_ERROR, (dest));
		break;
    case SX127X_WARNING:
        LOG(LL_WARN, (dest));
        break;
	case SX127X_INFO:
		LOG(LL_INFO, (dest));
		break;
	case SX127X_DEBUG:
		LOG(LL_DEBUG, (dest));
		break;
	}
}

void sx127x_timer_set(sx127x_timer_t* timer, int timeout, void *cb_arg)
{
	timer->id = mgos_set_timer(timeout, false, timer->callback, cb_arg);
}

void sx127x_timer_disable(sx127x_timer_t* timer)
{
	mgos_clear_timer(timer->id);
}

void sx127x_timer_usleep(uint32_t usecs)
{
    mgos_usleep(usecs);
}

void sx127x_timer_msleep(uint32_t msecs)
{
    mgos_msleep(msecs);
}

void sx127x_spi_write(void *spi, uint8_t addr, uint8_t *buffer, uint8_t size)
{
	uint8_t tx_data[size + 1];
    tx_data[0] = addr;
    memcpy(tx_data + 1, buffer, size);

    struct mgos_spi_txn txn = {
        .cs = 0, /* Use CS0 line as configured by cs0_gpio */
        .mode = MGOS_SPI_MODE,
        .freq = MGOS_SPI_SPEED,
    };
    /* Half-duplex, command/response transaction setup */
    /* Transmit 1 byte from tx_data */
    txn.hd.tx_len = size + 1;
    txn.hd.tx_data = tx_data;
    /* No dummy bytes necessary */
    txn.hd.dummy_len = 0;
    /* No rx_data */
    txn.hd.rx_len = 0;
    txn.hd.rx_data = NULL;
    mgos_ints_disable();
    if (!mgos_spi_run_txn((struct mgos_spi *)spi, false /* full_duplex */, &txn)) {
        sx127x_log(SX127X_ERROR, "Error during spi write\n");
    }
    mgos_ints_enable();
}

void sx127x_spi_read(void *spi, uint8_t addr, uint8_t *buffer, uint8_t size)
{
    struct mgos_spi_txn txn = {
        .cs = 0, /* Use CS0 line as configured by cs0_gpio */
        .mode = MGOS_SPI_MODE,
        .freq = MGOS_SPI_SPEED,
    };
    /* Half-duplex, command/response transaction setup */
    /* Transmit 1 byte from tx_data */
    txn.hd.tx_len = 1;
    txn.hd.tx_data = &addr;
    /* No dummy bytes necessary */
    txn.hd.dummy_len = 0;
    /* No rx_data */
    txn.hd.rx_len = size;
    txn.hd.rx_data = buffer;
    mgos_ints_disable();
    if (!mgos_spi_run_txn((struct mgos_spi *)spi, false /* full_duplex */, &txn)) {
        sx127x_log(SX127X_ERROR, "Error during spi read\n");
    }
    mgos_ints_enable();
}

bool sx127x_gpio_init(int pin, sx127x_gpio_mode_t mode, sx127x_gpio_pull_t pull)
{
    switch (mode) {
    case GPIO_IN:
        if (!mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_INPUT)) return false;
        break;
    case GPIO_OUT:
        if (!mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_OUTPUT)) return false;
        break;
    }

    switch (pull) {
    case GPIO_PULL_NONE:
        if (!mgos_gpio_set_pull(pin, MGOS_GPIO_PULL_NONE)) return false;
        break;
    case GPIO_PULL_UP:
        if (!mgos_gpio_set_pull(pin, MGOS_GPIO_PULL_UP)) return false;
        break;
    case GPIO_PULL_DOWN:
        if (!mgos_gpio_set_pull(pin, MGOS_GPIO_PULL_DOWN)) return false;
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
        if (!mgos_gpio_set_int_handler(pin, MGOS_GPIO_INT_EDGE_POS, cb, arg)) {
            return false;
        }
        if (!mgos_gpio_enable_int(pin)) return false;
        break;
    case GPIO_FALLING:
        break;
    case GPIO_RISING_FALLING:
        break;         
    }
    return true;
}

void sx127x_gpio_clear(int pin)
{
    mgos_gpio_write(pin, 0);
}
