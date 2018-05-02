/*
 * Copyright (c) 2017 Smetro Technologies
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef SX127X_HAL_H
#define SX127X_HAL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Log types.
 */
typedef enum {
	SX127X_ERROR,
	SX127X_WARNING,
	SX127X_INFO,
	SX127X_DEBUG
} sx127x_log_t;

/**
 * Spi types.
 */
typedef void sx127x_spi_t;

/**
 * Timer types.
 */
typedef void (*sx127x_timer_callback)(void *param);

typedef struct {
	uintptr_t id;
	sx127x_timer_callback callback;
} sx127x_timer_t;

/**
 * Gpio types.
 */
typedef enum {
	GPIO_IN,
	GPIO_OUT
} sx127x_gpio_mode_t;

typedef enum {
	GPIO_PULL_NONE,
	GPIO_PULL_UP,
	GPIO_PULL_DOWN
} sx127x_gpio_pull_t;

typedef enum {
	GPIO_RISING,
	GPIO_FALLING,
	GPIO_RISING_FALLING,	
} sx127x_gpio_int_mode_t;

typedef void (*sx127x_gpio_handler)(int pin, void *arg);

void sx127x_log(sx127x_log_t type, const char *fmt, ...);

void sx127x_timer_set(sx127x_timer_t* timer, int timeout, void *cb_arg);
void sx127x_timer_disable(sx127x_timer_t* timer);
void sx127x_timer_usleep(uint32_t usecs);
void sx127x_timer_msleep(uint32_t msecs);

void sx127x_spi_write(void *spi, uint8_t addr, uint8_t *buffer, uint8_t size);
void sx127x_spi_read(void *spi, uint8_t addr, uint8_t *buffer, uint8_t size);

bool sx127x_gpio_init(int pin, sx127x_gpio_mode_t mode, sx127x_gpio_pull_t pull);
bool sx127x_gpio_init_int(int pin, sx127x_gpio_mode_t mode,
	sx127x_gpio_pull_t pull, sx127x_gpio_int_mode_t int_mode,
	sx127x_gpio_handler cb, void *arg);
void sx127x_gpio_clear(int pin);

#ifdef __cplusplus
}
#endif

#endif /* SX127X_HAL_H */
