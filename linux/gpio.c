/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "gpio.h"
#include "log.h"

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 32
#define MAX_PIN 4

static linux_sx127x_gpio_t gpios[MAX_PIN];
static int gpio_len = 0;
static int stop = 1;

static int gpio_export(unsigned int gpio)
{
    int fd, len, ret = 0;
    char buf[MAX_BUF];
 
    fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
    if (fd < 0) {
        LOG(ERROR, ("Error: gpio/export\n"));
        return fd;
    }
 
    len = snprintf(buf, sizeof(buf), "%d", gpio);
    if (write(fd, buf, len) < 0) {
        LOG(WARNING, ("Cannot write export pin\n"));
        ret = -1;
    }
    close(fd);
 
    return ret;
}

static int gpio_unexport(unsigned int gpio)
{
    int fd, len;
    char buf[MAX_BUF];
 
    fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
    if (fd < 0) {
        LOG(ERROR, ("Error: gpio/export\n"));
        return fd;
    }
 
    len = snprintf(buf, sizeof(buf), "%d", gpio);
    if (write(fd, buf, len) < 0) {
        LOG(WARNING, ("Cannot write unexport pin\n"));
    }
    close(fd);
    return 0;
}

static int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
    int fd, ret = 0;
    char buf[MAX_BUF];
 
    snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
 
    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        LOG(ERROR, ("Error: gpio/direction\n"));
        return fd;
    }
 
    if (out_flag) {
        if (write(fd, "out", 4) < 0) {
            LOG(WARNING, ("Cannot write set dir\n"));
            ret = -1;
        }
    } else {
        if (write(fd, "in", 3) < 0) {
            LOG(WARNING, ("Cannot write set dir\n"));
            ret = -1;
        }
    }
 
    close(fd);
    return ret;
}

static int gpio_set_edge(unsigned int gpio, char *edge)
{
    int fd;
    char buf[MAX_BUF];

    snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);
 
    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        LOG(ERROR, ("Error: gpio/set-edge\n"));
        return fd;
    }
 
    if (write(fd, edge, strlen(edge) + 1) < 0) {
        LOG(WARNING, ("Cannot write set edge\n"));
    }
    close(fd);
    return 0;
}

static int gpio_fd_open(unsigned int gpio)
{
    int fd;
    char buf[MAX_BUF];

    snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
    fd = open(buf, O_RDONLY | O_NONBLOCK );
    if (fd < 0) {
        LOG(ERROR, ("Error: gpio/fd_open\n"));
    }
    return fd;
}

static int gpio_set_value(unsigned int gpio, unsigned int value)
{
    int fd;
    char buf[MAX_BUF];
 
    snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        LOG(ERROR, ("Error: gpio/set-value\n"));
        return fd;
    }
 
    if (value) {
        if (write(fd, "1", 2) < 0) {
            LOG(WARNING, ("Cannot write set value\n"));
        }
    } else {
        if (write(fd, "0", 2) < 0) {
            LOG(WARNING, ("Cannot write set value\n"));
        }
    }
 
    close(fd);
    return 0;
}

static void* gpio_handle(void *arg)
{
	struct pollfd fdset[1];
    int fd, rc;
    char *buf[MAX_BUF];
    linux_sx127x_gpio_t *gpio = (linux_sx127x_gpio_t *)arg;

    fd = gpio_fd_open(gpio->pin);
    while (stop) {
        memset((void*)fdset, 0, sizeof(fdset));
      
        fdset[0].fd = fd;
        fdset[0].events = POLLPRI;

        rc = poll(fdset, 1, POLL_TIMEOUT);      

        if (rc < 0) {
            LOG(ERROR, ("Error: poll failed\n"));
            break;
        }
            
        if (fdset[0].revents & POLLPRI) {
            if (read(fdset[0].fd, buf, MAX_BUF) < 0) {
                LOG(WARNING, ("Cannot read POLLPRI event"));
                continue;
            }
            if (gpio->cb) {
                gpio->cb(gpio->pin, gpio->arg);
            }
        }
    }
    close(fd);
    return NULL;
}

bool linux_gpio_set_mode(int pin, linux_gpio_mode_t mode)
{
    gpio_unexport(pin);
	if (gpio_export(pin) < 0) return false;
	if (gpio_set_dir(pin, mode) < 0) return false;
	return true;
}

bool linux_gpio_set_pull(int pin, linux_gpio_pull_type_t pull)
{
	/* Not supported via sysfs */
	return true;
}

bool linux_gpio_set_int_handler(int pin, linux_gpio_int_mode_t mode,
                               sx127x_gpio_handler cb, void *arg)
{
	int ret;

	switch (mode) {
	case LINUX_GPIO_INT_EDGE_POS:
		gpio_set_edge(pin, "rising");
		break;
	case LINUX_GPIO_INT_EDGE_NEG:
		gpio_set_edge(pin, "falling");
		break;
	case LINUX_GPIO_INT_EDGE_ANY:
		gpio_set_edge(pin, "both");
		break;
	default:
		/* Other modes not supported */
		LOG(ERROR, ("Interrupt mode not supported\n"));
		return false;
	}

	gpios[gpio_len].pin = pin;
	gpios[gpio_len].cb = cb;
	gpios[gpio_len].arg = arg;

	ret = pthread_create(&gpios[gpio_len].pid, NULL, &gpio_handle, &gpios[gpio_len]);
    if (ret != 0) {
    	LOG(ERROR, ("Cannot create the gpio handle\n"));
        return false;
    }
    gpio_len++;
    return true;		
}

void linux_gpio_write(int pin, int level)
{
	gpio_set_value(pin, level);
}

void linux_gpio_init(void)
{
	gpio_len = 0;
}

void linux_gpio_term(void)
{
	int i;
	stop = 0;
	for (i = 0; i < MAX_PIN; i++) {
		if (gpios[i].pid) {
			pthread_join(gpios[i].pid, NULL);
            gpio_unexport(gpios[i].pin);
		}
	}
}