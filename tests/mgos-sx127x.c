/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#include "mgos.h"
#include "mgos_gpio.h"
#include "mgos_spi.h"
#include "mgos_slrm.h"

#define LED 13
#define KEY 21

#define NODE 1
#define BUTTON 1

enum tx_states {
    SENDING,
    SUCCESS,
    ERROR
};

static enum tx_states tx_state = SUCCESS;

static void timer_cb(void *arg) {
    if (tx_state == ERROR) {
        mgos_gpio_toggle(LED);
    }
}

static void timer2_cb(void *arg) {
    uint8_t data[] = "123456";
    LOG(LL_INFO, ("sx127x sending data ..."));
    slrm_send(data, sizeof(data));
}

static void key_cb(int pin, void *arg) {
    LOG(LL_INFO, ("Key pressed"));
	if (tx_state > 0) {
        uint8_t data[] = "123456";
        LOG(LL_INFO, ("sx127x sending data ..."));
        if (slrm_send(data, sizeof(data))) {
            tx_state = SENDING;
            mgos_gpio_write(LED, 1);
        } else {
            tx_state = ERROR;
        }
    }
}

static void node_cb(slrm_send_status status, uint8_t *data, size_t len) {
    if (status == SLRM_SEND_SUCCESS) {
        fprintf(stderr, "Received ack from gateway\n");
        tx_state = SUCCESS;
        mgos_gpio_write(LED, 0);
    } else {
        fprintf(stderr, "Error sending data\n");
        tx_state = ERROR;
    }
}

static void gateway_cb(uint8_t *data, size_t *len) {
    fprintf(stderr, "Received: %s\n", data + sizeof(slrm_header_t));
    *len = sizeof(slrm_header_t);
}

static void init_lora(void) {
#ifdef NODE
    uint8_t uid[6] = { 0x01, 0xaa, 0x02, 0xbb, 0x03, 0xcc };
    uint8_t gid[6] = { 0xcc, 0xcc, 0xcc, 0xaa, 0xaa, 0xaa };
    mgos_slrm_create(mgos_spi_get_global(), uid, gid, SLRM_NODE, node_cb, NULL); 
#else
    uint8_t gid[6] = { 0xcc, 0xcc, 0xcc, 0xaa, 0xaa, 0xaa };
    uint8_t uid[6] = { 0xcc, 0xcc, 0xcc, 0xaa, 0xaa, 0xaa };
    mgos_slrm_create(mgos_spi_get_global(), uid, gid, SLRM_GATEWAY, NULL, gateway_cb); 
#endif
}

static void init_gpio(void) {
    mgos_gpio_set_mode(LED, MGOS_GPIO_MODE_OUTPUT);
    mgos_gpio_set_pull(LED, MGOS_GPIO_PULL_UP);
    mgos_gpio_write(LED, 0);
    mgos_gpio_set_button_handler(KEY, MGOS_GPIO_PULL_NONE, MGOS_GPIO_INT_EDGE_NEG,
        250, key_cb, NULL);
}

enum mgos_app_init_result mgos_app_init(void) {
    LOG(LL_INFO, ("mgos-sx127x test"));
    init_lora();

#ifdef NODE
#ifdef BUTTON
    init_gpio();
    mgos_set_timer(500, true, timer_cb, NULL);
#else
    timer2_cb(NULL);
    mgos_set_timer(30000, true, timer2_cb, NULL);
#endif
#endif
    return MGOS_APP_INIT_SUCCESS;
}
