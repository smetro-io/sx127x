/*
 * Copyright (c) 2017 Smetro Technologies
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "clock.h"
#include "ext_int.h"
#include "gpio.h"
#include "log.h"
#include "power.h"
#include "rtc.h"
#include "spi.h"
#include "timer.h"
#include "uart.h"

#include "sx127x_hal.h"

void sx127x_log(sx127x_log_t type, const char *fmt, ...) {
	va_list args;
    char dest[256];
	va_start(args, fmt);
    vsprintf(dest, fmt, args);
    va_end(args);
	switch (type) {
	case SX127X_ERROR:
		LOG("LOG_ERROR: %s\r\n", (dest));
		break;
    case SX127X_WARNING:
        LOG("LOG_WARN: %s\r\n", (dest));
        break;
	case SX127X_INFO:
		LOG("LOG_INFO: %s\r\n", (dest));
		break;
	case SX127X_DEBUG:
		LOG("LOG_DEBUG: %s\r\n", (dest));
		break;
	}
}

void sx127x_timer_set(sx127x_timer_t* timer, int timeout, void *cb_arg) {
    timer_set(TIMER21, timeout, true, timer->callback, cb_arg);
    timer->id = TIMER21;
}

void sx127x_timer_disable(sx127x_timer_t* timer) {
    timer_stop_int(timer->id);
}

void sx127x_timer_msleep(uint32_t msecs) {
    delay_ms(msecs);
}

void sx127x_spi_write(void *spi, uint8_t addr, uint8_t *buffer, uint8_t size) {
    uint8_t i;

    spi_cs_low();
    spi_transmit(addr | 0x80);
    for( i = 0; i < size; i++ ) {
        spi_transmit(buffer[i]);
    }
    spi_cs_high();
}

void sx127x_spi_read(void *spi, uint8_t addr, uint8_t *buffer, uint8_t size) {
    uint8_t i;

    spi_cs_low();
    spi_transmit(addr & 0x7F);
    for( i = 0; i < size; i++ ) {
        buffer[i] = spi_transmit(0);
    }
    spi_cs_high();
}

bool sx127x_gpio_init(int pin, sx127x_gpio_mode_t mode, sx127x_gpio_pull_t pull) {
    switch (mode) {
    case GPIO_IN:
        gpio_init(pin, INPUT, pull);
        break;
    case GPIO_OUT:
        gpio_init(pin, OUTPUT, pull);
        break;
    }
    return true;
}

bool sx127x_gpio_init_int(int pin, sx127x_gpio_mode_t mode, sx127x_gpio_pull_t pull,
    sx127x_gpio_int_mode_t int_mode, sx127x_gpio_handler cb, void *arg) {

    switch (int_mode) {
    case GPIO_RISING:
        ext_interrupt_init(pin, INT_RISING, pull, cb, arg);
        break;
    case GPIO_FALLING:
        ext_interrupt_init(pin, INT_FALLING, pull, cb, arg);
        break;
    case GPIO_RISING_FALLING:
        ext_interrupt_init(pin, INT_RISING_FALLING, pull, cb, arg);
        break;        
    }
    return true;
}

void sx127x_gpio_clear(int pin) {
    gpio_write(pin, 0);
}
