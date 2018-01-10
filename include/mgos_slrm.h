/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#ifndef CS_MOS_LIBS_SX127x_SRC_MGOS_H_
#define CS_MOS_LIBS_SX127x_SRC_MGOS_H_

#include "slrm.h"
#include "mgos_spi.h"
#include "sx127x.h"
#include "sx127x_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

slrm_t* mgos_slrm_create(struct mgos_spi* spi, uint8_t* uid, uint8_t* gid,
  slrm_mode mode, slrm_node_callback node_cb,
  slrm_gateway_callback gateway_cb);

void mgos_slrm_close(slrm_t *dev);

bool mgos_sx127x_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CS_MOS_LIBS_SX127x_SRC_MGOS_H_ */
