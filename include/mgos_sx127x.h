/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#ifndef CS_MOS_LIBS_SX127x_SRC_MGOS_H_
#define CS_MOS_LIBS_SX127x_SRC_MGOS_H_

#include "mgos_spi.h"
#include "sx127x.h"
#include "sx127x_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

int mgos_sx127x_send(sx127x_t *dev, void *buf, unsigned len);

sx127x_t *mgos_sx127x_create(struct mgos_spi* spi,
	sx127x_event_callback event_callback);

void mgos_sx127x_close(sx127x_t *dev);

bool mgos_sx127x_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CS_MOS_LIBS_SX127x_SRC_MGOS_H_ */
