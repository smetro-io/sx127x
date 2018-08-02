/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#ifndef _STM_SLRM_H_
#define _STM_SLRM_H_

#include "slrm.h"
#include "sx127x.h"
#include "sx127x_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

slrm_t* stm_slrm_create(uint8_t* id,
  slrm_mode mode, slrm_node_callback node_cb,
  slrm_gateway_callback gateway_cb);

void stm_slrm_close(slrm_t *dev);

bool stm_sx127x_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _STM_SLRM_H_ */
