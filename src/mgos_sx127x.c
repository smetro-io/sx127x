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
    	return NULL;
  	}
  	sx127x->dev.params.spi = spi;
  	sx127x_init(&sx127x->dev);

  	return sx127x;
}

void mgos_sx127x_close(struct mgos_sx127x *sx127x) {
	free(sx127x);
}

bool mgos_sx127x_init(void) {
	return true;
}