/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#include "mgos_sx127x.h"

int mgos_sx127x_send(sx127x_t *dev, void *buf, unsigned len) {
  return sx127x_send(dev, buf, len);
}

sx127x_t *mgos_sx127x_create(struct mgos_spi* spi,
  sx127x_event_callback event_callback) {
	sx127x_t *dev = calloc(1, sizeof(*dev));
 	if (dev == NULL) return NULL;
 	if (spi == NULL) {
    LOG(LL_ERROR, ("SPI is not configured, make sure spi.enable is true"));
    free(dev);
    return NULL;
  }
  dev->params.spi = spi;
  if (event_callback == NULL) {
    LOG(LL_ERROR, ("Invalid event callback function"));
    free(dev);
    return NULL;
  }
  dev->event_callback = event_callback;

  const struct sys_config_sx127x *cfg = &get_cfg()->sx127x;
  if (cfg->rst < 0 || cfg->rst > 34 || cfg->dio0 < 0 || cfg->dio1 > 34 ||
  	cfg->dio2 < 0 || cfg->dio2 > 34 || cfg->dio3 < 0 || cfg->dio3 > 34) {
  	LOG(LL_ERROR, ("Invalid SX127X settings"));
  	free(dev);
  	return NULL;
  }
  dev->params.reset_pin = cfg->rst;
  dev->params.dio0_pin = cfg->dio0;
  dev->params.dio1_pin = cfg->dio1;
  dev->params.dio2_pin = cfg->dio2;
  dev->params.dio3_pin = cfg->dio3;

  sx127x_setup(dev);

	return dev;
}

void mgos_sx127x_close(sx127x_t *dev) {
	free(dev);
}

bool mgos_sx127x_init(void) {
	return true;
}