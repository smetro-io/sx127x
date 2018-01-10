/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <linux_slrm.h>

static bool stop = true;

static void node_cb(slrm_send_status status, uint8_t *data, size_t len) {
    if (status == SLRM_SEND_SUCCESS) {
        fprintf(stderr, "Received ack from gateway\n");
    } else {
        fprintf(stderr, "Error sending data\n");
    }
    stop = false;
}

static void gateway_cb(uint8_t *data, size_t *len) {
    fprintf(stderr, "Received: %s\n", data + sizeof(slrm_header_t));
    *len = sizeof(slrm_header_t);
}

static void handle_signal(int signo) {
	if (signo == SIGINT) {
    	stop = false;
	}
}

static void usage() {
    fprintf(stderr, "linux-sx127x usage:\n");
    fprintf(stderr, "\t-n <data>: transmit data in node mode\n");
    fprintf(stderr, "\t-g: gateway mode\n");
    fprintf(stderr, "\t-h: show help\n");
}

int main(int argc, char *argv[])
{
	int opt;
	bool node = false, gateway = false;
	char *data = NULL;

	slrm_t *mac;
	linux_slrm_cfg_t cfg = { 0 };

	struct sigaction sig_handler;
    sig_handler.sa_handler = handle_signal;
    sigemptyset(&sig_handler.sa_mask);
    sig_handler.sa_flags = 0;
    sigaction(SIGINT, &sig_handler, NULL);

	while ((opt = getopt(argc, argv, "hgn:")) != -1) {
        switch (opt) {
        case 'h':
            usage();
            return 0;
        case 'g':
            gateway = true;
            break;
        case 'n':
        	node = true;
            data = optarg;
            break;
        default:
            usage();
            return 1;
        }
    }

    fprintf(stderr, "linux-sx127x  test\n");

   	cfg.rst = 6;
	cfg.dio0 = 0;
	cfg.dio1 = 1;
	cfg.dio2 = 2;
	cfg.dio3 = 3;
	cfg.spi = "/dev/spidev1.0";

	if (gateway) {
        uint8_t uid[6] = { 0xcc, 0xcc, 0xcc, 0xaa, 0xaa, 0xaa };
        uint8_t gid[6] = { 0xcc, 0xcc, 0xcc, 0xaa, 0xaa, 0xaa };
        cfg.mode = SLRM_GATEWAY;
        cfg.gateway_cb = gateway_cb;
        memcpy(cfg.uid, uid, 6);
        memcpy(cfg.gid, gid, 6);
	} else if (node) {
        uint8_t uid[6] = { 0x01, 0xaa, 0x02, 0xbb, 0x03, 0xcc };
        uint8_t gid[6] = { 0xcc, 0xcc, 0xcc, 0xaa, 0xaa, 0xaa };
        cfg.mode = SLRM_NODE;
        cfg.node_cb = node_cb;
        memcpy(cfg.uid, uid, 6);
        memcpy(cfg.gid, gid, 6);
	}
    mac = linux_slrm_init(&cfg);
    if (!mac) {
        fprintf(stderr, "Cannot create mac!\n");
        return 1;
    }
    if (node) slrm_send((uint8_t *)data, strlen(data));
    while(stop) sleep(1);
	return 0;
}
