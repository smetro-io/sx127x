/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#include <stdio.h>
#include <unistd.h>
#include <linux_sx127x.h>

static void send_data(sx127x_t *sx127x) {
	uint8_t data[] = "123456";
    fprintf(stderr, "sx127x sending data ...\n");
    linux_sx127x_send(sx127x, (void *)data, sizeof(data));
}

static void event_callback(void *param, int event) {
	printf("event %d\n", event);
}

int main(int argc, const char * argv[])
{
	sx127x_t *sx127x;
	linux_sx127x_cfg_t cfg;
	cfg.rst = 6;
	cfg.dio0 = 0;
	cfg.dio1 = 1;
	cfg.dio2 = 2;
	cfg.dio3 = 3;
	cfg.spi = "/dev/spidev1.0";

	fprintf(stderr, "sx127x linux test\n");
	
	sx127x = linux_sx127x_init(event_callback, &cfg);
	if (!sx127x) {
		fprintf(stderr, "Cannot create sx127x device!\n");
		return 1;
	}

	while (1) {
		send_data(sx127x);
		sleep(5);
	}
	return 0;
}
