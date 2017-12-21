/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#ifndef _TIMER_H_
#define _TIMER_H_ 

#include <pthread.h>

#include "sx127x_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int enable;
	uint32_t timeout;
	sx127x_timer_callback callback;
	void *cb_arg;
	pthread_t pid;
} linux_sx127x_timer_t;

uintptr_t linux_timer_set(uint32_t timeout, sx127x_timer_callback callback,
						void *cb_arg);

void linux_timer_disable(uintptr_t id);

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_H_ */