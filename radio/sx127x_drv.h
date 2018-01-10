/*
 * Copyright (c) 2017 Smetro Technologies
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#ifndef SX127X_DRV_H
#define SX127X_DRV_H

#include <stdlib.h>
#include "sx127x.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Received LoRa packet status information
 */
typedef struct {
    uint8_t rssi;           /**< RSSI of a received packet */
    uint8_t lqi;            /**< LQI of a received packet */
    int8_t snr;             /**< S/N ratio */
    uint32_t time_on_air;   /**< Time on air of a received packet (ms) */
} sx127x_lora_packet_info_t;

/**
 * List of configuration options available
 */
typedef enum {
	SX127X_OPT_STATE,
	SX127X_OPT_DEVICE_MODE,
	SX127X_OPT_CHANNEL,
	SX127X_OPT_BANDWIDTH,
	SX127X_OPT_SPREADING_FACTOR,
	SX127X_OPT_CODING_RATE,
	SX127X_OPT_MAX_PACKET_SIZE,
	SX127X_OPT_INTEGRITY_CHECK,
	SX127X_OPT_CHANNEL_HOP,
	SX127X_OPT_CHANNEL_HOP_PERIOD,
	SX127X_OPT_SINGLE_RECEIVE,
	SX127X_OPT_RX_TIMEOUT,
	SX127X_OPT_TX_TIMEOUT,
	SX127X_OPT_TX_POWER,
	SX127X_OPT_FIXED_HEADER,
	SX127X_OPT_PREAMBLE_LENGTH,
	SX127X_OPT_IQ_INVERT
} sx127x_opt_t;

/**
 * Option parameter to be used with SX127X_OPT_STATE to set or get
 */
typedef enum {
    SX127X_OPT_STATE_OFF = 0,
    SX127X_OPT_STATE_SLEEP,
    SX127X_OPT_STATE_IDLE,
    SX127X_OPT_STATE_RX,
    SX127X_OPT_STATE_TX,
    SX127X_OPT_STATE_RESET,
    SX127X_OPT_STATE_STANDBY,
} sx127x_opt_state_t;

/**
 * Binary parameter for enabling and disabling options
 */
typedef enum {
    SX127X_OPT_DISABLE = 0,
    SX127X_OPT_ENABLE = 1,
} sx127x_opt_enable_t;

/**
 * Driver interface
 */

/**
 * Send frame
 */
int sx127x_send(sx127x_t *dev, void *buf, unsigned len);

/**
 * Get a received frame
 */
int sx127x_recv(sx127x_t *dev, void *buf, size_t len,
			sx127x_lora_packet_info_t *info);

/**
 * Driver's setup function
 */
int sx127x_setup(sx127x_t *dev);

/**
 * Get an option value from device
 */
int sx127x_get(sx127x_t *dev, sx127x_opt_t opt,
           void *value, size_t max_len);

/**
 * Set an option value for device
 */
int sx127x_set(sx127x_t *dev, sx127x_opt_t opt,
           const void *value, size_t value_len);

#ifdef __cplusplus
}
#endif

#endif /* SX127X_DRV_H */
