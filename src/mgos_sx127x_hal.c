/*
 * Copyright (c) 2017 Smetro Technologies
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include <stdarg.h>

#include "mgos_timers.h"
#include "mgos_hal.h"
#include "mgos_spi.h"

#include "sx127x_hal.h"

#define MGOS_SPI_SPEED    10000000
#define MGOS_SPI_MODE     0

void sx127x_log(sx127x_log_t type, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	switch (type) {
	case SX127X_ERROR:
		LOG(LL_ERROR, (fmt, args));
		break;
	case SX127X_INFO:
		LOG(LL_INFO, (fmt, args));
		break;
	case SX127X_DEBUG:
		LOG(LL_DEBUG, (fmt, args));
		break;
	}
	va_end(args);
}

void sx127x_timer_set(sx127x_timer_t* timer, int timeout, void *cb_arg)
{
	timer->id = mgos_set_timer(timeout, false, timer->cb, cb_arg);
}

void sx127x_timer_disable(sx127x_timer_t* timer)
{
	mgos_clear_timer(timer->id);
}

void sx127x_spi_write(void *spi, uint8_t addr, uint8_t *buffer, uint8_t size)
{
	uint8_t tx_data[size + 1];
    tx_data[0] = addr;
    memcpy(tx_data + 1, buffer, size);

    struct mgos_spi_txn txn = {
        .cs = 0, /* Use CS0 line as configured by cs0_gpio */
        .mode = MGOS_SPI_MODE,
        .freq = MGOS_SPI_SPEED,
    };
    /* Half-duplex, command/response transaction setup */
    /* Transmit 1 byte from tx_data */
    txn.hd.tx_len = size + 1;
    txn.hd.tx_data = tx_data;
    /* No dummy bytes necessary */
    txn.hd.dummy_len = 0;
    /* No rx_data */
    txn.hd.rx_len = 0;
    txn.hd.rx_data = NULL;
    mgos_ints_disable();
    mgos_spi_run_txn((struct mgos_spi *)spi, false /* full_duplex */, &txn);
    mgos_ints_enable();
}

void sx127x_spi_read(void *spi, uint8_t addr, uint8_t *buffer, uint8_t size)
{
    struct mgos_spi_txn txn = {
        .cs = 0, /* Use CS0 line as configured by cs0_gpio */
        .mode = MGOS_SPI_MODE,
        .freq = MGOS_SPI_SPEED,
    };
    /* Half-duplex, command/response transaction setup */
    /* Transmit 1 byte from tx_data */
    txn.hd.tx_len = 1;
    txn.hd.tx_data = &addr;
    /* No dummy bytes necessary */
    txn.hd.dummy_len = 0;
    /* No rx_data */
    txn.hd.rx_len = size;
    txn.hd.rx_data = buffer;
    mgos_ints_disable();
    mgos_spi_run_txn((struct mgos_spi *)spi, false /* full_duplex */, &txn);
    mgos_ints_enable();
}
