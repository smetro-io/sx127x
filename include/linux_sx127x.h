/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#ifndef LINUX_SX127X_H_
#define LINUX_SX127X_H_

#include <sx127x.h>
#include <sx127x_drv.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int rst;
	int dio0;
	int dio1;
	int dio2;
	int dio3;
	char* spi;
} linux_sx127x_cfg_t;

int linux_sx127x_send(sx127x_t *dev, void *buf, unsigned len);

void linux_sx127x_close(sx127x_t *dev);

sx127x_t *linux_sx127x_init(sx127x_event_callback event_callback, linux_sx127x_cfg_t *cfg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LINUX_SX127X_H_ */
