/*
 * Copyright (c) 2017 Smetro Technologies
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "sx127x_hal.h"
#include "sx127x_registers.h"
#include "sx127x_internal.h"

#include "sx127x_drv.h"

/* Internal helper functions */
static int _set_state(sx127x_t *dev, sx127x_opt_state_t state);
static int _get_state(sx127x_t *dev, void *val);

int sx127x_send(sx127x_t *dev, void *buf, unsigned len)
{
    if (sx127x_get_state(dev) == SX127X_RF_TX_RUNNING) {
        sx127x_log(SX127X_WARNING, "Cannot send packet: radio alredy in transmitting state.");
        return -ENOTSUP;
    }
    switch (dev->settings.modem) {
    case SX127X_MODEM_FSK:
        /* todo */
        break;
    case SX127X_MODEM_LORA:
        /* Initializes the payload size */
        sx127x_set_payload_length(dev, len);

        /* Full buffer used for Tx */
        sx127x_reg_write(dev, SX127X_REG_LR_FIFOTXBASEADDR, 0x00);
        sx127x_reg_write(dev, SX127X_REG_LR_FIFOADDRPTR, 0x00);

        /* FIFO operations can not take place in Sleep mode
         * So wake up the chip */
        if (sx127x_get_op_mode(dev) == SX127X_RF_OPMODE_SLEEP) {
          sx127x_set_standby(dev);
          sx127x_timer_msleep(SX127X_RADIO_WAKEUP_TIME); /* wait for chip wake up */
        }

        /* Write payload buffer */
        sx127x_write_fifo(dev, buf, len);
        break;
    default:
        sx127x_log(SX127X_WARNING, "Unsupported modem");
        break;
    }

    /* Enable TXDONE interrupt */
    sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGSMASK,
                SX127X_RF_LORA_IRQFLAGS_RXTIMEOUT |
                SX127X_RF_LORA_IRQFLAGS_RXDONE |
                SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR |
                SX127X_RF_LORA_IRQFLAGS_VALIDHEADER |
                /* SX127X_RF_LORA_IRQFLAGS_TXDONE | */
                SX127X_RF_LORA_IRQFLAGS_CADDONE |
                SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL |
                SX127X_RF_LORA_IRQFLAGS_CADDETECTED);

    /* Set TXDONE interrupt to the DIO0 line */
    sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1,
                (sx127x_reg_read(dev, SX127X_REG_DIOMAPPING1) &
                SX127X_RF_LORA_DIOMAPPING1_DIO0_MASK) |
                SX127X_RF_LORA_DIOMAPPING1_DIO0_01);

    /* Start TX timeout timer */
    sx127x_timer_set(&dev->_internal.tx_timeout_timer,
    dev->settings.lora.tx_timeout, dev);

    /* Put chip into transfer mode */
    sx127x_set_state(dev, SX127X_RF_TX_RUNNING);
    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_TRANSMITTER);

    return 0;
}

int sx127x_recv(sx127x_t *dev, void *buf, size_t len,
                sx127x_lora_packet_info_t *info)
{
    volatile uint8_t irq_flags = 0;
    uint8_t size = 0;

    switch (dev->settings.modem) {
    case SX127X_MODEM_FSK:
        /* todo */
        break;
    case SX127X_MODEM_LORA:
        /* Clear IRQ */
        sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS, SX127X_RF_LORA_IRQFLAGS_RXDONE);

        irq_flags = sx127x_reg_read(dev, SX127X_REG_LR_IRQFLAGS);
        if ((irq_flags & SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR_MASK) ==
            SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR) {
            /* Clear IRQ */
            sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS,
                          SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR);

            if (!(dev->settings.lora.flags & SX127X_RX_CONTINUOUS_FLAG)) {
                sx127x_set_state(dev, SX127X_RF_IDLE);
            }

            sx127x_timer_disable(&dev->_internal.rx_timeout_timer);
            dev->event_callback(dev, SX127X_RX_ERROR_CRC);
            return -EBADMSG;
        }

        if (info) {
            /* there is no LQI for LoRa */
            info->lqi = 0;
            uint8_t snr_value = sx127x_reg_read(dev, SX127X_REG_LR_PKTSNRVALUE);
            if (snr_value & 0x80) { /* The SNR is negative */
                /* Invert and divide by 4 */
                info->snr = -1 * ((~snr_value + 1) & 0xFF) >> 2;
            } else {
                /* Divide by 4 */
                info->snr = (snr_value & 0xFF) >> 2;
            }

            int16_t rssi = sx127x_reg_read(dev, SX127X_REG_LR_PKTRSSIVALUE);

            if (info->snr < 0) {
        #if defined(MODULE_SX1272)
                info->rssi = SX127X_RSSI_OFFSET + rssi + (rssi >> 4) + info->snr;
        #else /* MODULE_SX1276 */
                if (dev->settings.channel > SX127X_RF_MID_BAND_THRESH) {
                    info->rssi = SX127X_RSSI_OFFSET_HF + rssi + (rssi >> 4) + info->snr;
                } else {
                    info->rssi = SX127X_RSSI_OFFSET_LF + rssi + (rssi >> 4) + info->snr;
                }
        #endif
            } else {
#if defined(MODULE_SX1272)
                packet_info->rssi = SX127X_RSSI_OFFSET + rssi + (rssi >> 4);
#else /* MODULE_SX1276 */
                if (dev->settings.channel > SX127X_RF_MID_BAND_THRESH) {
                    info->rssi = SX127X_RSSI_OFFSET_HF + rssi + (rssi >> 4);
                } else {
                    info->rssi = SX127X_RSSI_OFFSET_LF + rssi + (rssi >> 4);
                }
#endif
            }
            info->time_on_air = sx127x_get_time_on_air(dev, len);
        }

        size = sx127x_reg_read(dev, SX127X_REG_LR_RXNBBYTES);
        if (buf == NULL) {
            return size;
        }

        if (size > len) {
            return -ENOBUFS;
        }

        if (!(dev->settings.lora.flags & SX127X_RX_CONTINUOUS_FLAG)) {
            sx127x_set_state(dev, SX127X_RF_IDLE);
        }

        sx127x_timer_disable(&dev->_internal.rx_timeout_timer);

        /* Read the last packet from FIFO */
        uint8_t last_rx_addr = sx127x_reg_read(dev, SX127X_REG_LR_FIFORXCURRENTADDR);
        sx127x_reg_write(dev, SX127X_REG_LR_FIFOADDRPTR, last_rx_addr);
        sx127x_read_fifo(dev, (uint8_t*)buf, size);
        break;
    default:
        break;
    }

    return size;
}

int sx127x_setup(sx127x_t *dev, sx127x_radio_settings_t *radio)
{
    dev->irq = 0;
    sx127x_radio_settings_t settings;
    settings.channel = radio->channel;
    settings.modem = SX127X_MODEM_DEFAULT;
    settings.state = SX127X_RF_IDLE;

    dev->settings = settings;

    /* Launch initialization of driver and device */
    sx127x_log(SX127X_DEBUG, "init_radio: initializing driver...\n");
    if (sx127x_init(dev) < 0) {
        sx127x_log(SX127X_ERROR, "init_radio: error initializing driver!\n");
        return -1;
    }

    sx127x_init_radio_settings(dev, radio);
    /* Put chip into sleep */
    sx127x_set_sleep(dev);

    sx127x_log(SX127X_DEBUG, "init_radio: sx127x initialization done\n");

    return 0;
}

int sx127x_get(sx127x_t *dev, sx127x_opt_t opt, void *val, size_t max_len)
{
    if (dev == NULL) {
        return -ENODEV;
    }

    switch(opt) {
        case SX127X_OPT_STATE:
            assert(max_len >= sizeof(sx127x_opt_state_t));
            return _get_state(dev, val);

        case SX127X_OPT_DEVICE_MODE:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = dev->settings.modem;
            return sizeof(uint8_t);

        case SX127X_OPT_CHANNEL:
            assert(max_len >= sizeof(uint32_t));
            *((uint32_t*) val) = sx127x_get_channel(dev);
            return sizeof(uint32_t);

        case SX127X_OPT_BANDWIDTH:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_bandwidth(dev);
            return sizeof(uint8_t);

        case SX127X_OPT_SPREADING_FACTOR:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_spreading_factor(dev);
            return sizeof(uint8_t);

        case SX127X_OPT_CODING_RATE:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_coding_rate(dev);
            return sizeof(uint8_t);

        case SX127X_OPT_MAX_PACKET_SIZE:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_max_payload_len(dev);
            return sizeof(uint8_t);

        case SX127X_OPT_INTEGRITY_CHECK:
            assert(max_len >= sizeof(sx127x_opt_enable_t));
            *((sx127x_opt_enable_t*) val) = sx127x_get_crc(dev) ? SX127X_OPT_ENABLE : SX127X_OPT_DISABLE;
            break;

        case SX127X_OPT_CHANNEL_HOP:
            assert(max_len >= sizeof(sx127x_opt_enable_t));
            *((sx127x_opt_enable_t*) val) = (dev->settings.lora.flags & SX127X_CHANNEL_HOPPING_FLAG) ? SX127X_OPT_ENABLE : SX127X_OPT_DISABLE;
            break;

        case SX127X_OPT_CHANNEL_HOP_PERIOD:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_hop_period(dev);
            return sizeof(uint8_t);

        case SX127X_OPT_SINGLE_RECEIVE:
            assert(max_len >= sizeof(uint8_t));
            *((sx127x_opt_enable_t*) val) = sx127x_get_rx_single(dev) ? SX127X_OPT_ENABLE : SX127X_OPT_DISABLE;
            break;

        default:
            break;
    }

    return 0;
}

int sx127x_set(sx127x_t *dev, sx127x_opt_t opt, const void *val, size_t len)
{
    int res = -ENOTSUP;

    if (dev == NULL) {
        return -ENODEV;
    }

    switch(opt) {
    case SX127X_OPT_STATE:
        assert(len <= sizeof(sx127x_opt_state_t));
        return _set_state(dev, *((const sx127x_opt_state_t*) val));

    case SX127X_OPT_DEVICE_MODE:
        assert(len <= sizeof(uint8_t));
        sx127x_set_modem(dev, *((const uint8_t*) val));
        return sizeof(sx127x_opt_enable_t);

    case SX127X_OPT_CHANNEL:
        assert(len <= sizeof(uint32_t));
        sx127x_set_channel(dev, *((const uint32_t*) val));
        return sizeof(uint32_t);

    case SX127X_OPT_BANDWIDTH:
        assert(len <= sizeof(uint8_t));
        uint8_t bw = *((const uint8_t *)val);
        if (bw > SX127X_BW_500_KHZ) {
            res = -EINVAL;
            break;
        }
        sx127x_set_bandwidth(dev, bw);
        return sizeof(uint8_t);

    case SX127X_OPT_SPREADING_FACTOR:
        assert(len <= sizeof(uint8_t));
        uint8_t sf = *((const uint8_t *)val);
        if (sf < SX127X_SF6 ||
            sf > SX127X_SF12) {
            res = -EINVAL;
            break;
        }
        sx127x_set_spreading_factor(dev, sf);
        return sizeof(uint8_t);

    case SX127X_OPT_CODING_RATE:
        assert(len <= sizeof(uint8_t));
        uint8_t cr = *((const uint8_t *)val);
        if (cr < SX127X_CR_4_5 ||
            cr > SX127X_CR_4_8) {
            res = -EINVAL;
            break;
        }
        sx127x_set_coding_rate(dev, cr);
        return sizeof(uint8_t);

    case SX127X_OPT_MAX_PACKET_SIZE:
        assert(len <= sizeof(uint8_t));
        sx127x_set_max_payload_len(dev, *((const uint8_t*) val));
        return sizeof(uint8_t);

    case SX127X_OPT_INTEGRITY_CHECK:
        assert(len <= sizeof(sx127x_opt_enable_t));
        sx127x_set_crc(dev, *((const sx127x_opt_enable_t*) val) ? true : false);
        return sizeof(sx127x_opt_enable_t);

    case SX127X_OPT_CHANNEL_HOP:
        assert(len <= sizeof(sx127x_opt_enable_t));
        sx127x_set_freq_hop(dev, *((const sx127x_opt_enable_t*) val) ? true : false);
        return sizeof(sx127x_opt_enable_t);

    case SX127X_OPT_CHANNEL_HOP_PERIOD:
        assert(len <= sizeof(uint8_t));
        sx127x_set_hop_period(dev, *((const uint8_t*) val));
        return sizeof(uint8_t);

    case SX127X_OPT_SINGLE_RECEIVE:
        assert(len <= sizeof(uint8_t));
        sx127x_set_rx_single(dev, *((const sx127x_opt_enable_t*) val) ? true : false);
        return sizeof(sx127x_opt_enable_t);

    case SX127X_OPT_RX_TIMEOUT:
        assert(len <= sizeof(uint32_t));
        sx127x_set_rx_timeout(dev, *((const uint32_t*) val));
        return sizeof(uint32_t);

    case SX127X_OPT_TX_TIMEOUT:
        assert(len <= sizeof(uint32_t));
        sx127x_set_tx_timeout(dev, *((const uint32_t*) val));
        return sizeof(uint32_t);

    case SX127X_OPT_TX_POWER:
        assert(len <= sizeof(uint8_t));
        sx127x_set_tx_power(dev, *((const uint8_t*) val));
        return sizeof(uint16_t);

    case SX127X_OPT_FIXED_HEADER:
        assert(len <= sizeof(sx127x_opt_enable_t));
        sx127x_set_fixed_header_len_mode(dev, *((const sx127x_opt_enable_t*) val) ? true : false);
        return sizeof(sx127x_opt_enable_t);

    case SX127X_OPT_PREAMBLE_LENGTH:
        assert(len <= sizeof(uint16_t));
        sx127x_set_preamble_length(dev, *((const uint16_t*) val));
        return sizeof(uint16_t);

    case SX127X_OPT_IQ_INVERT:
        assert(len <= sizeof(sx127x_opt_enable_t));
        sx127x_set_iq_invert(dev, *((const sx127x_opt_enable_t*) val) ? true : false);
        return sizeof(bool);

    default:
        break;
    }

    return res;
}

static int _set_state(sx127x_t *dev, sx127x_opt_state_t state)
{
    switch (state) {
    case SX127X_OPT_STATE_SLEEP:
        sx127x_set_sleep(dev);
        break;

    case SX127X_OPT_STATE_STANDBY:
        sx127x_set_standby(dev);
        break;

    case SX127X_OPT_STATE_IDLE:
        /* set permanent listening */
        sx127x_set_rx_timeout(dev, 0);
        sx127x_set_rx(dev);;
        break;

    case SX127X_OPT_STATE_RX:
        sx127x_set_rx(dev);
        break;

    case SX127X_OPT_STATE_TX:
        sx127x_set_tx(dev);
        break;

    case SX127X_OPT_STATE_RESET:
        sx127x_reset(dev);
        break;

    default:
        return -ENOTSUP;
    }
    return sizeof(sx127x_opt_state_t);
}

static int _get_state(sx127x_t *dev, void *val)
{
    uint8_t op_mode;
    op_mode = sx127x_get_op_mode(dev);
    sx127x_opt_state_t state = SX127X_OPT_STATE_OFF;
    switch(op_mode) {
    case SX127X_RF_OPMODE_SLEEP:
        state = SX127X_OPT_STATE_SLEEP;
        break;

    case SX127X_RF_OPMODE_STANDBY:
        state = SX127X_OPT_STATE_STANDBY;
        break;

    case SX127X_RF_OPMODE_TRANSMITTER:
        state = SX127X_OPT_STATE_TX;
        break;

    case SX127X_RF_OPMODE_RECEIVER:
    case SX127X_RF_LORA_OPMODE_RECEIVER_SINGLE:
        state = SX127X_OPT_STATE_IDLE;
        break;

    default:
        break;
    }
    memcpy(val, &state, sizeof(sx127x_opt_state_t));
    return sizeof(sx127x_opt_state_t);
}
