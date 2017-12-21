/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#include <linux_sx127x.h>

#include "spi.h"
#include "log.h"

int linux_sx127x_send(sx127x_t *dev, void *buf, unsigned len) {
    return sx127x_send(dev, buf, len);
}

void linux_sx127x_close(sx127x_t *dev) {
    linux_spi_term();
	free(dev);
}

sx127x_t * linux_sx127x_init(sx127x_event_callback event_callback, linux_sx127x_cfg_t *cfg) {
	sx127x_t *dev = calloc(1, sizeof(*dev));
    if (dev == NULL) return NULL;
    if (linux_spi_init(cfg->spi) < 0) {
        LOG(ERROR, ("SPI is not configured, make sure is the right one"));
        free(dev);
        return NULL;
    }
    /* Not used in Linux implementation */
    dev->params.spi = NULL;
    if (event_callback == NULL) {
        LOG(ERROR, ("Invalid event callback function"));
        free(dev);
        return NULL;
    }
    dev->event_callback = event_callback;

    dev->params.reset_pin = cfg->rst;
    dev->params.dio0_pin = cfg->dio0;
    dev->params.dio1_pin = cfg->dio1;
    dev->params.dio2_pin = cfg->dio2;
    dev->params.dio3_pin = cfg->dio3;

    if (sx127x_setup(dev) < 0) {
        free(dev);
        return NULL;
    }

    return dev;
}
