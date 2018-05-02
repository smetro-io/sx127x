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

slrm_t *stm_slrm_create(uint8_t* uid, uint8_t* gid,
    slrm_mode mode, slrm_node_callback node_cb,
    slrm_gateway_callback gateway_cb) {

    slrm_t *mac = &s_mac;

    mac->mode = mode;
    memcpy(mac->uid, uid, 6);
    memcpy(mac->gid, gid, 6);
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

    if(sx127x_setup(mac->dev) < 0) {
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
