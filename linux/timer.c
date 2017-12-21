/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#include <stdlib.h>
#include <unistd.h>

#include "timer.h"
#include "log.h"

void* linux_timer_handle(void *arg) {
	linux_sx127x_timer_t *timer = (linux_sx127x_timer_t *)arg;
	if (!timer) {
		LOG(WARNING, ("Invalid timer argument\n"));
        return NULL;
	}
	usleep(timer->timeout);
	if (timer->enable) {
		timer->callback(timer->cb_arg);
	}
	free(timer);
	return NULL;
}

uintptr_t linux_timer_set(uint32_t timeout, sx127x_timer_callback callback,
						void *cb_arg)
{
	int ret;
	linux_sx127x_timer_t *timer = calloc(1, sizeof(*timer)); 
	timer->enable = 1;
	timer->callback = callback;
	timer->cb_arg = cb_arg;
	timer->timeout = timeout;

	ret = pthread_create(&timer->pid, NULL, &linux_timer_handle, timer);
    if (ret != 0) {
    	LOG(ERROR, ("Cannot set the timer\n"));
        return (uintptr_t)0;
    }
    return (uintptr_t)(void*)timer;
}

void linux_timer_disable(uintptr_t id)
{
	linux_sx127x_timer_t *timer = (linux_sx127x_timer_t *)id;
	if (timer) {
		timer->enable = 0;
	}
}
