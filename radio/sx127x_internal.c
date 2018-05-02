/*
 * Copyright (c) 2017 Smetro Technologies
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#include "sx127x_hal.h"

#include "sx127x_internal.h"
#include "sx127x_registers.h"

bool sx127x_test(const sx127x_t *dev)
{
    /* Read version number and compare with sx127x assigned revision */
    uint8_t version = sx127x_reg_read(dev, SX127X_REG_VERSION);
#if defined(MODULE_SX1272)
    if (version != VERSION_SX1272) {
        sx127x_log(SX127X_ERROR, "[Error] sx1272 test failed, invalid version number: %d\n",
              version);
        return false;
    }
    sx127x_log(SX127X_DEBUG, "SX1272 transceiver detected.\n");
#else /* MODULE_SX1276) */
    if (version != VERSION_SX1276) {
        sx127x_log(SX127X_ERROR, "[Error] sx1276 test failed, invalid version number: %d\n",
              version);
        return false;
    }
    sx127x_log(SX127X_DEBUG, "SX1276 transceiver detected.\n");
#endif

    return true;
}

void sx127x_reg_write(const sx127x_t *dev, uint8_t addr, uint8_t data)
{
    sx127x_reg_write_burst(dev, addr, &data, 1);
}

uint8_t sx127x_reg_read(const sx127x_t *dev, uint8_t addr)
{
    uint8_t data = 0;

    sx127x_reg_read_burst(dev, addr, &data, 1);

    return data;
}

void sx127x_reg_write_burst(const sx127x_t *dev, uint8_t addr, uint8_t *buffer,
                            uint8_t size)
{
    sx127x_spi_write(dev->params.spi, addr | 0x80, buffer, size);
}

void sx127x_reg_read_burst(const sx127x_t *dev, uint8_t addr, uint8_t *buffer,
                           uint8_t size)
{
    sx127x_spi_read(dev->params.spi, addr & 0x7F, buffer, size);
}

void sx127x_write_fifo(const sx127x_t *dev, uint8_t *buffer, uint8_t size)
{
    sx127x_reg_write_burst(dev, 0, buffer, size);
}

void sx127x_read_fifo(const sx127x_t *dev, uint8_t *buffer, uint8_t size)
{
    sx127x_reg_read_burst(dev, 0, buffer, size);
}

void sx127x_rx_chain_calibration(sx127x_t *dev)
{
    uint8_t reg_pa_config_init_val;
    uint32_t initial_freq;
    uint32_t temp = 0;

    /* Save context */
    reg_pa_config_init_val = sx127x_reg_read(dev, SX127X_REG_PACONFIG);
    temp |= ((uint32_t)sx127x_reg_read(dev, SX127X_REG_FRFMSB) << 16);
    temp |= ((uint32_t)sx127x_reg_read(dev, SX127X_REG_FRFMID) << 8);
    temp |= sx127x_reg_read(dev, SX127X_REG_FRFLSB);
    initial_freq = (uint32_t)((double)temp * SX127X_FREQUENCY_RESOLUTION);

    /* Cut the PA just in case, RFO output, power = -1 dBm */
    sx127x_reg_write(dev, SX127X_REG_PACONFIG, 0x00);

    /* Launch Rx chain calibration for LF band */
    sx127x_reg_write(dev,
                     SX127X_REG_IMAGECAL,
                     (sx127x_reg_read(dev, SX127X_REG_IMAGECAL) & SX127X_RF_IMAGECAL_IMAGECAL_MASK)
                     | SX127X_RF_IMAGECAL_IMAGECAL_START);

    while ((sx127x_reg_read(dev, SX127X_REG_IMAGECAL) & SX127X_RF_IMAGECAL_IMAGECAL_RUNNING)
           == SX127X_RF_IMAGECAL_IMAGECAL_RUNNING) {
    }

    /* Set a frequency in HF band */
    sx127x_set_channel(dev, SX127X_HF_CHANNEL_DEFAULT);

    /* Launch Rx chain calibration for HF band */
    sx127x_reg_write(dev,
                     SX127X_REG_IMAGECAL,
                     (sx127x_reg_read(dev, SX127X_REG_IMAGECAL) & SX127X_RF_IMAGECAL_IMAGECAL_MASK)
                     | SX127X_RF_IMAGECAL_IMAGECAL_START);
    while ((sx127x_reg_read(dev, SX127X_REG_IMAGECAL) & SX127X_RF_IMAGECAL_IMAGECAL_RUNNING)
           == SX127X_RF_IMAGECAL_IMAGECAL_RUNNING) {
    }

    /* Restore context */
    sx127x_reg_write(dev, SX127X_REG_PACONFIG, reg_pa_config_init_val);
    sx127x_set_channel(dev, initial_freq);
}

int16_t sx127x_read_rssi(const sx127x_t *dev)
{
    int16_t rssi = 0;

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            rssi = -(sx127x_reg_read(dev, SX127X_REG_RSSIVALUE) >> 1);
            break;
        case SX127X_MODEM_LORA:
#if defined(MODULE_SX1272)
            rssi = SX127X_RSSI_OFFSET + sx127x_reg_read(dev, SX127X_REG_LR_RSSIVALUE);
#else /* MODULE_SX1276 */
            if (dev->settings.channel > SX127X_RF_MID_BAND_THRESH) {
                rssi = SX127X_RSSI_OFFSET_HF + sx127x_reg_read(dev, SX127X_REG_LR_RSSIVALUE);
            }
            else {
                rssi = SX127X_RSSI_OFFSET_LF + sx127x_reg_read(dev, SX127X_REG_LR_RSSIVALUE);
            }
#endif
            break;
        default:
            rssi = -1;
            break;
    }

    return rssi;
}

void sx127x_start_cad(sx127x_t *dev)
{
    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            break;
        case SX127X_MODEM_LORA:
            /* Disable all interrupts except CAD-related */
            sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGSMASK,
                             SX127X_RF_LORA_IRQFLAGS_RXTIMEOUT |
                             SX127X_RF_LORA_IRQFLAGS_RXDONE |
                             SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR |
                             SX127X_RF_LORA_IRQFLAGS_VALIDHEADER |
                             SX127X_RF_LORA_IRQFLAGS_TXDONE |
                             /*SX127X_RF_LORA_IRQFLAGS_CADDONE |*/
                             SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL
                             /* | SX127X_RF_LORA_IRQFLAGS_CADDETECTED*/
                             );

            /* DIO3 = CADDone */
            sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1,
                             (sx127x_reg_read(dev, SX127X_REG_DIOMAPPING1) &
                              SX127X_RF_LORA_DIOMAPPING1_DIO3_MASK) |
                             SX127X_RF_LORA_DIOMAPPING1_DIO3_00);

            sx127x_set_state(dev,  SX127X_RF_CAD);
            sx127x_set_op_mode(dev, SX127X_RF_LORA_OPMODE_CAD);
            break;
        default:
            break;
    }
}

bool sx127x_is_channel_free(sx127x_t *dev, uint32_t freq, int16_t rssi_threshold)
{
    int16_t rssi = 0;

    sx127x_set_channel(dev, freq);
    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_RECEIVER);

    sx127x_timer_msleep(1); /* wait 1 millisecond */

    rssi = sx127x_read_rssi(dev);
    sx127x_set_sleep(dev);
    sx127x_log(SX127X_WARNING, "RSSI: %d\n", rssi);

    return (rssi <= rssi_threshold);
}

