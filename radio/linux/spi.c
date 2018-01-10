/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/ioctl.h>
#include <linux/ioctl.h>

#include "spi.h"
#include "log.h"

#define LINUX_SPI_SPEED    1000000
#define LINUX_SPI_MODE     0

static int fd = -1;

int linux_spi_init(const char* path)
{
	uint8_t data8;
    /* Open device */
    if ((fd = open(path, O_RDWR)) < 0) {
        LOG(ERROR, ("Error: Opening SPI device \"%s\"", path));
        return -1;
    }

    /* Set mode, bit order, extra flags */
    data8 = LINUX_SPI_MODE | MSB_FIRST;
    if (ioctl(fd, SPI_IOC_WR_MODE, &data8) < 0) {
        close(fd);
        LOG(ERROR, ("Error: Setting SPI mode"));
        return -1;
    }

    /* Set max speed */
    uint32_t max_speed = LINUX_SPI_SPEED;
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &max_speed) < 0) {
        close(fd);
        LOG(ERROR, ("Error: Setting SPI max speed"));
        return -1;
    }

    /* Set bits per word */
    uint8_t bits_per_word = 8;
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0) {
        close(fd);
        LOG(ERROR, ("Error: Setting SPI bits per word"));
        return -1;
    }

    return 0;
}

int linux_spi_transfer(struct spi_ioc_transfer *spi_xfer, int len)
{
    if (ioctl(fd, SPI_IOC_MESSAGE(len), spi_xfer) < 1) return -1;
    return 0;
}

int linux_spi_term(void)
{
    if (fd < 0)return 0;

    if (close(fd) < 0) {
    	LOG(ERROR, ("Closing SPI device"));
    	return -1;
    }

    fd = -1;
    return 0;
}
