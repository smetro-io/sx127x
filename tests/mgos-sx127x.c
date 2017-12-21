/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#include "mgos.h"
#include "mgos_gpio.h"
#include "mgos_spi.h"
#include "mgos_sx127x.h"

static void timer_cb(void *arg) {
    sx127x_t *sx127x = (sx127x_t *)arg;
    uint8_t data[] = "123456";
    LOG(LL_INFO, ("sx127x sending data ..."));
    mgos_sx127x_send(sx127x, (void *)data, sizeof(data));
}

static void event_callback(void *param, int event) {
    LOG(LL_INFO, ("event %d", event));
}

enum mgos_app_init_result mgos_app_init(void) {
    sx127x_t *sx127x = mgos_sx127x_create(mgos_spi_get_global(), event_callback);
    LOG(LL_INFO, ("sx127x mgos test"));
    mgos_set_timer(5000, true, timer_cb, sx127x);
    return MGOS_APP_INIT_SUCCESS;
}
