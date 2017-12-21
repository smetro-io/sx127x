/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#ifndef _SPI_H_
#define _SPI_H_

#include <linux/spi/spidev.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum spi_bit_order {
    MSB_FIRST,
    LSB_FIRST,
} spi_bit_order_t;

int linux_spi_init(const char* path);

int linux_spi_transfer(struct spi_ioc_transfer *spi_xfer, int len);

int linux_spi_term(void);

#ifdef __cplusplus
}
#endif

#endif /* _SPI_H_ */
