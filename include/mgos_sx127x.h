/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#ifndef CS_MOS_LIBS_SX127x_SRC_MGOS_H_
#define CS_MOS_LIBS_SX127x_SRC_MGOS_H_

#include "mgos_spi.h"
#include "sx127x/sx127x.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mgos_sx127x {
	sx127x_t dev;
};

struct mgos_sx127x *mgos_sx127x_create(struct mgos_spi* spi);

void mgos_sx127x_close(struct mgos_sx127x *sx127x);

bool mgos_sx127x_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CS_MOS_LIBS_SX127x_SRC_MGOS_H_ */
