/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "linux_slrm.h"
#include "sx127x_drv.h"
#include "linux/spi.h"
#include "linux/log.h"

slrm_t* linux_slrm_init(linux_slrm_cfg_t *cfg) {
	slrm_t *mac = calloc(1, sizeof(slrm_t));
    if (mac == NULL) return NULL;

    mac->mode = cfg->mode;
    memcpy(mac->uid, cfg->uid, 6);
    memcpy(mac->gid, cfg->gid, 6);
    mac->node_cb = cfg->node_cb;
    mac->gateway_cb = cfg->gateway_cb;

    mac->dev = calloc(1, sizeof(sx127x_t));
    if (mac->dev == NULL) {
        free(mac);
        return NULL;
    }
    if (linux_spi_init(cfg->spi) < 0) {
        LOG(ERROR, ("SPI is not configured, make sure is the right one"));
        free(mac->dev);
        free(mac);
        return NULL;
    }
    /* Not used in Linux implementation */
    mac->dev->params.spi = NULL;
    mac->dev->event_callback = slrm_event_callback;

    mac->dev->params.reset_pin = cfg->rst;
    mac->dev->params.dio0_pin = cfg->dio0;
    mac->dev->params.dio1_pin = cfg->dio1;
    mac->dev->params.dio2_pin = cfg->dio2;
    mac->dev->params.dio3_pin = cfg->dio3;

    if (sx127x_setup(mac->dev) < 0) {
        free(mac->dev);
        free(mac);
        return NULL;
    }
    slrm_init(mac);

    return mac;
}

void linux_slrm_close(slrm_t *mac) {
    linux_spi_term();
    free(mac->dev);
    free(mac);
}