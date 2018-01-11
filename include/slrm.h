/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#ifndef SLRM_H
#define SLRM_H

#include <stddef.h>

#include "sx127x.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SLRM_NODE,
	SLRM_GATEWAY,
} slrm_mode;

typedef enum {
	SLRM_SEND_FAIL,
	SLRM_SEND_SUCCESS,
} slrm_send_status;

typedef void (*slrm_node_callback)(slrm_send_status status, uint8_t *data, size_t len);
typedef void (*slrm_gateway_callback)(uint8_t *data, size_t *len);

typedef struct {
	slrm_mode mode;
	uint8_t uid[6];
	uint8_t gid[6];
    slrm_node_callback node_cb;
    slrm_gateway_callback gateway_cb;
    sx127x_t *dev;
} slrm_t;

typedef struct {
	uint8_t seq;
	uint8_t uid[6];
	uint8_t nid;
} slrm_header_t;

void slrm_event_callback(void *param, int event);
bool slrm_send(uint8_t* data, uint8_t len);
void slrm_init(slrm_t *mac);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SLRM_H */
