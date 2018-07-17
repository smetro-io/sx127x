/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#include "mgos.h"
#include "mgos_slrm.h"

slrm_t *mgos_slrm_create(struct mgos_spi* spi, uint8_t* uid, uint8_t* gid,
    slrm_mode mode, slrm_node_callback node_cb,
    slrm_gateway_callback gateway_cb) {
    slrm_t *mac = calloc(1, sizeof(slrm_t));
    if (mac == NULL) return NULL;	

    mac->mode = mode;
    memcpy(mac->uid, uid, sizeof(mac->uid));
    memcpy(mac->gid, gid, sizeof(mac->gid));
    mac->node_cb = node_cb;
    mac->gateway_cb = gateway_cb;

    mac->dev = calloc(1, sizeof(sx127x_t));
   	if (mac->dev == NULL) {
        free(mac);
        return NULL;
    }
   	if (spi == NULL) {
        LOG(LL_ERROR, ("SPI is not configured, make sure spi.enable is true"));
        free(mac->dev);
        free(mac);
        return NULL;
    }
    mac->dev->params.spi = spi;
    mac->dev->event_callback = slrm_event_callback;

    const struct mgos_config_sx127x *cfg = mgos_sys_config_get_sx127x();
    if (cfg->rst < 0 || cfg->rst > 34 || cfg->dio0 < 0 || cfg->dio1 > 34 ||
    	cfg->dio2 < 0 || cfg->dio2 > 34 || cfg->dio3 < 0 || cfg->dio3 > 34) {
    	LOG(LL_ERROR, ("Invalid SX127X settings"));
        free(mac->dev);
    	free(mac);
    	return NULL;
    }

    mac->dev->params.reset_pin = cfg->rst;
    mac->dev->params.dio0_pin = cfg->dio0;
    mac->dev->params.dio1_pin = cfg->dio1;
    mac->dev->params.dio2_pin = cfg->dio2;
    mac->dev->params.dio3_pin = cfg->dio3;

    sx127x_setup(mac->dev);

    slrm_init(mac);

  	return mac;
}

void mgos_slrm_close(slrm_t *mac) {
    free(mac->dev);
    free(mac);
}

bool mgos_sx127x_init(void) {
    return true;
}
