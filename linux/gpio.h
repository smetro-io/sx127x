/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#ifndef _GPIO_H_
#define _GPIO_H_

#include <pthread.h>

#include <sx127x_hal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LINUX_GPIO_MODE_INPUT = 0, /* input mode */
	LINUX_GPIO_MODE_OUTPUT = 1 /* output mode */
} linux_gpio_mode_t;

typedef enum {
	LINUX_GPIO_INT_NONE = 0,
	LINUX_GPIO_INT_EDGE_POS = 1, /* positive edge */
	LINUX_GPIO_INT_EDGE_NEG = 2, /* negative edge */
	LINUX_GPIO_INT_EDGE_ANY = 3, /* any edge - positive or negative */
	LINUX_GPIO_INT_LEVEL_HI = 4, /* high voltage level */
	LINUX_GPIO_INT_LEVEL_LO = 5  /* low voltage level */
} linux_gpio_int_mode_t;

typedef enum {
	LINUX_GPIO_PULL_NONE = 0,
	LINUX_GPIO_PULL_UP = 1,  /* pin is pilled to the high voltage */
	LINUX_GPIO_PULL_DOWN = 2 /* pin is pulled to the low voltage */
} linux_gpio_pull_type_t;

typedef void (*sx127x_gpio_handler)(int pin, void *arg);

typedef struct {
	int pin;
	sx127x_gpio_handler cb;
	void *arg;
	pthread_t pid;
} linux_sx127x_gpio_t;

bool linux_gpio_set_mode(int pin, linux_gpio_mode_t mode);

bool linux_gpio_set_pull(int pin, linux_gpio_pull_type_t pull);

bool linux_gpio_set_int_handler(int pin, linux_gpio_int_mode_t mode,
                               sx127x_gpio_handler cb, void *arg);

void linux_gpio_write(int pin, int level);

#ifdef __cplusplus
}
#endif

#endif /* _GPIO_H_ */
