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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generic log types.
 */
typedef enum {
	SX127X_ERROR,
	SX127X_INFO,
	SX127X_DEBUG
} sx127x_log_t;

/**
 * Generic spi.
 */
typedef void sx127x_spi_t;

/**
 * Generic timer.
 */

typedef void (*sx127x_timer_callback)(void *param);

typedef struct {
	uintptr_t id;
	sx127x_timer_callback cb;
} sx127x_timer_t;

void sx127x_log(sx127x_log_t type, const char *fmt, ...);

void sx127x_timer_set(sx127x_timer_t* timer, int timeout, void *cb_arg);
void sx127x_timer_disable(sx127x_timer_t* timer);

void sx127x_spi_write(void *spi, uint8_t addr, uint8_t *buffer, uint8_t size);
void sx127x_spi_read(void *spi, uint8_t addr, uint8_t *buffer, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif /* SX127X_HAL_H */
