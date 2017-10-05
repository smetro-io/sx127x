/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#include "mgos_sx127x.h"

struct mgos_sx127x *mgos_sx127x_create(struct mgos_spi* spi) {
	struct mgos_sx127x *sx127x = calloc(1, sizeof(*sx127x));
 	if (sx127x == NULL) return NULL;
 	if (spi == NULL) {
    	LOG(LL_ERROR, ("SPI is not configured, make sure spi.enable is true"));
    	free(sx127x);
    	return NULL;
  	}
  	sx127x->dev.params.spi = spi;

  	const struct sys_config_sx127x *cfg = &get_cfg()->sx127x;
  	if (cfg->rst < 0 || cfg->rst > 34 || cfg->dio0 < 0 || cfg->dio1 > 34 ||
  		cfg->dio2 < 0 || cfg->dio2 > 34 || cfg->dio3 < 0 || cfg->dio3 > 34) {
  		LOG(LL_ERROR, ("Invalid SX127X settings"));
  		free(sx127x);
  		return NULL;
  	}
  	sx127x->dev.params.reset_pin = cfg->rst;
  	sx127x->dev.params.dio0_pin = cfg->dio0;
  	sx127x->dev.params.dio1_pin = cfg->dio1;
  	sx127x->dev.params.dio2_pin = cfg->dio2;
  	sx127x->dev.params.dio3_pin = cfg->dio3;

  	sx127x->dev.irq = 0;
  	sx127x_radio_settings_t settings;
    settings.channel = SX127X_CHANNEL_DEFAULT;
    settings.modem = SX127X_MODEM_DEFAULT;
    settings.state = SX127X_RF_IDLE;

    sx127x->dev.settings = settings;

  	sx127x_init(&sx127x->dev);

  	sx127x_init_radio_settings(&sx127x->dev);
    /* Put chip into sleep */
    sx127x_set_sleep(&sx127x->dev);

    LOG(LL_DEBUG, ("init_radio: sx127x initialization done\n"));

  	return sx127x;
}

void mgos_sx127x_close(struct mgos_sx127x *sx127x) {
	free(sx127x);
}

bool mgos_sx127x_init(void) {
	return true;
}