/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#include <stdio.h>
#include <string.h>

#include "sx127x_internal.h"
#include "sx127x_drv.h"
#include "sx127x_hal.h"
#include "slrm.h"

#define MAX_SEND_ATTEMPTS	2

/* An 8-bit CRC is used to validate the readings */
#define CRC8_POLYNOMIAL 0x131
#define CRC8_INITIAL_CRC 0x00

typedef struct {
	uint8_t data[32];
	uint8_t len;
	uint8_t retries;
} slrm_frame_t;

static slrm_frame_t frame;
static uint8_t message[32] = { 0 };

slrm_t *mac = NULL;

static uint8_t slrm_crc8(uint8_t crc, uint8_t byte) {
	uint8_t i;

	crc ^= byte;

	for (i = 0; i < 8; i++) {
		if ((crc & 0x80) == 0x80) {
	  		crc = (crc << 1) ^ CRC8_POLYNOMIAL;
		} else {
	  		crc <<= 1;
		}
	}

	return crc;
}

static uint8_t slrm_crc(uint8_t *data, uint8_t len) {
	int i;
	uint8_t crc;

	crc = CRC8_INITIAL_CRC;

	for (i = 0; i < len; i++) {
		crc = slrm_crc8(crc, data[i]);
	}
  	return crc;
}

static bool slrm_collision_avoidance(uint8_t *data, uint8_t len) {
	uint8_t i;
	for (i = 0; i < 3; i++) {
		if (!sx127x_is_channel_free(mac->dev, SX127X_CHANNEL_DEFAULT, -100)) {
			uint8_t timeout = (sx127x_random(mac->dev) % 5) + 3;
			sx127x_timer_msleep(100 * timeout);
			continue;
		}
		sx127x_send(mac->dev, data, len);
		return true;
	}
	sx127x_log(SX127X_DEBUG, "Cannot send because channel always busy\n");
	return false;
}

static void slrm_retry(void) {
	if (frame.retries < MAX_SEND_ATTEMPTS) {
		sx127x_log(SX127X_DEBUG, "Try send again %d\n", frame.retries);
		frame.retries++;
		slrm_collision_avoidance(frame.data, frame.len);
	} else {
		sx127x_log(SX127X_DEBUG, "Give up don't try again %d\n", frame.retries);
		slrm_error_type err = SLRM_RECEIVE_NULL;
		mac->node_cb(SLRM_SEND_FAIL, (uint8_t*)&err, 1);
		sx127x_set_sleep(mac->dev);
    }
}

static bool slrm_check_frame(uint8_t* data, uint8_t len) {
	for(uint8_t i = 0; i < sizeof(slrm_header_t); i++) {
		if(frame.data[i] != data[i]) {
			return false;
		}
	}
	return true;
}

static void slrm_rx_listen(sx127x_t *dev) {
	/* Switch to continuous listen mode */
	sx127x_set_rx_single(dev, false);
	sx127x_set_rx_timeout(dev, 0);
	sx127x_set_rx(dev);
}

static void slrm_rx_single(sx127x_t *dev, int timeout) {
	sx127x_set_rx_single(dev, true);
	sx127x_set_rx_timeout(dev, timeout);
	sx127x_set_rx(dev);
}

static void slrm_node_recv(void) {
	size_t len;
	sx127x_lora_packet_info_t packet_info;

    len = sx127x_recv(mac->dev, NULL, 0, 0);
    memset(message, 0, 32);
    sx127x_recv(mac->dev, message, len, &packet_info);
    sx127x_log(SX127X_DEBUG, "{Payload: (%d bytes), RSSI: %i, SNR: %i, TOA: %i}\n",
        (int)len, packet_info.rssi, (int)packet_info.snr,
        (int)packet_info.time_on_air);

    if (slrm_check_frame(message, len)) {
    	sx127x_log(SX127X_DEBUG, "Received ack from last frame\n", frame.retries);
    	mac->node_cb(SLRM_SEND_SUCCESS, message + sizeof(slrm_header_t), len - sizeof(slrm_header_t));
    	sx127x_set_sleep(mac->dev);
    } else {
    	sx127x_log(SX127X_DEBUG, "Received other frame\n", frame.retries);
    	slrm_retry();
    }
}

static void slrm_gateway_recv(void) {
	size_t len;
	sx127x_lora_packet_info_t packet_info;

    len = sx127x_recv(mac->dev, NULL, 0, 0);
    if (len < 8 || len > 32) {
    	sx127x_log(SX127X_DEBUG, "Received incorrect frame\n");
    	return;
    }

    memset(message, 0, 32);
    sx127x_recv(mac->dev, message, len, &packet_info);
    sx127x_log(SX127X_DEBUG, "{Payload: (%d bytes), RSSI: %i, SNR: %i, TOA: %i}\n",
        (int)len, packet_info.rssi, (int)packet_info.snr,
        (int)packet_info.time_on_air);

    if(mac->gateway_cb(message, &len)) {
    	sx127x_timer_msleep(20);
    	sx127x_send(mac->dev, message, len);
	} else {
		sx127x_log(SX127X_DEBUG, "Received frame from other network\n");
	}
}

void slrm_event_callback(void *param, int event) {
    sx127x_t *dev = (sx127x_t *)param;

    if (mac->mode == SLRM_NODE) {
		switch (event) {
	    case SX127X_RX_DONE:
	    	slrm_node_recv();
	        break;
	    case SX127X_TX_DONE:
	        sx127x_log(SX127X_DEBUG, "Transmission completed\n");
	        slrm_rx_single(dev, (1000U * 4UL));
	        break;
	    case SX127X_CAD_DONE:
	        break;
	    case SX127X_RX_TIMEOUT:
			slrm_retry();
	    	break;
	    case SX127X_TX_TIMEOUT:
	    	sx127x_log(SX127X_WARNING, "Transmission timeout\n");
	    	slrm_error_type err = SLRM_SEND_TIMEOUT;
	    	mac->node_cb(SLRM_SEND_FAIL, (uint8_t*)&err, 1);
	    	sx127x_set_sleep(dev);
	        break;
	    default:
	        sx127x_log(SX127X_WARNING, "Unexpected event received: %d\n", event);
	        break;
	    }
    } else if (mac->mode == SLRM_GATEWAY) {
		switch (event) {
	    case SX127X_RX_DONE:
	    	slrm_gateway_recv();
	        break;
	    case SX127X_TX_DONE:
	        sx127x_log(SX127X_DEBUG, "Transmission completed\n");
	        slrm_rx_listen(dev);
	        break;
	    case SX127X_CAD_DONE:
	        break;
	    case SX127X_TX_TIMEOUT:
	    	sx127x_log(SX127X_DEBUG, "Transmission timeout\n");
	        break;
	    default:
	        sx127x_log(SX127X_WARNING, "Unexpected event received: %d\n", event);
	        break;
	    }
    }
}

bool slrm_send(uint8_t* data, uint8_t len) {
	slrm_header_t* header;

	frame.len = sizeof(slrm_header_t) + len;
	memcpy((frame.data + sizeof(slrm_header_t)), data, len);

	/* Create header */
	header = (slrm_header_t*)frame.data;
	memcpy(&header->id, mac->id, sizeof(mac->id));
	header->seq = slrm_crc(frame.data + 1, frame.len - 1);
	
	frame.retries = 0;
	return slrm_collision_avoidance(frame.data, frame.len);
}

void slrm_init(slrm_t *data) {
	mac = data;
	if (mac->mode == SLRM_GATEWAY) {
		slrm_rx_listen(mac->dev);
	}
}
