/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#ifndef SLRM_H
#define SLRM_H

#include <stddef.h>

#include "sx127x.h"
#include "sx127x_drv.h"

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

typedef enum {
	SLRM_SEND_TIMEOUT,
	SLRM_RECEIVE_NULL,
} slrm_error_type;

typedef void (*slrm_node_callback)(uint8_t status, uint8_t *data, size_t len);
typedef bool (*slrm_gateway_callback)(uint8_t *data, size_t *len, sx127x_lora_packet_info_t *info);

typedef struct {
	slrm_mode mode;
	uint8_t id[4];
    slrm_node_callback node_cb;
    slrm_gateway_callback gateway_cb;
    sx127x_t *dev;
} slrm_t;

typedef struct {
	uint8_t seq;
	uint8_t id[4];
} __attribute__((__packed__)) slrm_header_t;

void slrm_event_callback(void *param, int event);
bool slrm_send(uint8_t* data, uint8_t len);
void slrm_init(slrm_t *mac);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SLRM_H */
