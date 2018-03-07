/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#ifndef LINUX_SLRM_H_
#define LINUX_SLRM_H_

#include "slrm.h"

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
	slrm_mode mode;
	uint8_t uid[6];
	uint8_t gid[6];
    slrm_node_callback node_cb;
    slrm_gateway_callback gateway_cb;
} linux_slrm_cfg_t;

slrm_t* linux_slrm_init(linux_slrm_cfg_t *cfg);
void linux_slrm_close(slrm_t* mac);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LINUX_SLRM_H_ */
