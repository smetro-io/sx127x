/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#include <stdio.h>
#include <string.h>

#include "stm_slrm.h"
#include "conf.h"

static slrm_t s_mac;
static sx127x_t s_dev;
static sx127x_radio_settings_t s_radio;

slrm_t *stm_slrm_create(uint8_t* id,
    slrm_mode mode, slrm_node_callback node_cb,
    slrm_gateway_callback gateway_cb) {

    slrm_t *mac = &s_mac;

    mac->mode = mode;
    memcpy(mac->id, id, sizeof(mac->id));
    mac->node_cb = node_cb;
    mac->gateway_cb = gateway_cb;

    mac->dev = &s_dev;

    mac->dev->params.spi = NULL;
    mac->dev->event_callback = slrm_event_callback;

    mac->dev->params.reset_pin = RESET_PIN;
    mac->dev->params.dio0_pin = DIO0_PIN;
    mac->dev->params.dio1_pin = DIO1_PIN;
    mac->dev->params.dio2_pin = DIO2_PIN;
    mac->dev->params.dio3_pin = DIO3_PIN;

    sx127x_radio_settings_t *radio = &s_radio;
    radio->channel = s_lr_cfg.lr_channel;
    radio->lora.bandwidth = s_lr_cfg.lr_bandwidth;
    radio->lora.datarate = s_lr_cfg.lr_sf;
    radio->lora.coderate = s_lr_cfg.lr_cr;    

    if(sx127x_setup(mac->dev, radio) < 0) {
        sx127x_log(SX127X_ERROR, "stm_slrm_create: slrm exiting...\n");
        return NULL;
    }

    slrm_init(mac);

  	return mac;
}

void stm_slrm_close(slrm_t *mac) {

}

bool stm_sx127x_init(void) {
    return true;
}
