/*
 * Copyright (c) 2017 Smetro Technologies
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include "sx127x_hal.h"
#include "sx127x_internal.h"
#include "sx127x_registers.h"

/* Internal functions */
static void _init_isrs(sx127x_t *dev);
static void _init_timers(sx127x_t *dev);
static void _on_tx_timeout(void *arg);
static void _on_rx_timeout(void *arg);

void sx127x_reset(const sx127x_t *dev)
{
    /*
     * This reset scheme complies with 7.2 chapter of the SX1272/1276 datasheet
     * See http://www.semtech.com/images/datasheet/sx1276.pdf for SX1276
     * See http://www.semtech.com/images/datasheet/sx1272.pdf for SX1272
     *
     * 1. Set NReset pin to LOW for at least 100 us
     * 2. Set NReset in Hi-Z state
     * 3. Wait at least 5 milliseconds
     */
    sx127x_gpio_init(dev->params.reset_pin, GPIO_OUT, GPIO_PULL_NONE);

    /* Set reset pin to 0 */
    sx127x_gpio_clear(dev->params.reset_pin);

    /* Wait 1 ms */
    sx127x_timer_msleep(1);

    /* Put reset pin in High-Z */
    sx127x_gpio_init(dev->params.reset_pin, GPIO_IN, GPIO_PULL_NONE);

    /* Wait 10 ms */
    sx127x_timer_msleep(10);
}

int sx127x_init(sx127x_t *dev)
{
    /* Check presence of SX127X */
    if (!sx127x_test(dev)) {
        sx127x_log(SX127X_ERROR, "[Error] init : sx127x test failed\n");
        return -SX127X_ERR_TEST_FAILED;
    }

    _init_timers(dev);
    sx127x_timer_msleep(1); /* wait 1 millisecond */
    sx127x_reset(dev);

    sx127x_rx_chain_calibration(dev);
    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_SLEEP);

    _init_isrs(dev);

    return SX127X_INIT_OK;
}

void sx127x_init_radio_settings(sx127x_t *dev)
{
    sx127x_set_freq_hop(dev, SX127X_FREQUENCY_HOPPING);
    sx127x_set_iq_invert(dev, SX127X_IQ_INVERSION);
    sx127x_set_rx_single(dev, SX127X_RX_SINGLE);
    sx127x_set_tx_timeout(dev, SX127X_TX_TIMEOUT_DEFAULT);
    sx127x_set_modem(dev, SX127X_MODEM_DEFAULT);
    sx127x_set_channel(dev, SX127X_CHANNEL_DEFAULT);
    sx127x_set_bandwidth(dev, SX127X_BW_DEFAULT);
    sx127x_set_spreading_factor(dev, SX127X_SF_DEFAULT);
    sx127x_set_coding_rate(dev, SX127X_CR_DEFAULT);

    sx127x_set_fixed_header_len_mode(dev, SX127X_FIXED_HEADER_LEN_MODE);
    sx127x_set_crc(dev, SX127X_PAYLOAD_CRC_ON);
    sx127x_set_symbol_timeout(dev, SX127X_SYMBOL_TIMEOUT);
    sx127x_set_preamble_length(dev, SX127X_PREAMBLE_LENGTH);
    sx127x_set_payload_length(dev, SX127X_PAYLOAD_LENGTH);
    sx127x_set_hop_period(dev, SX127X_FREQUENCY_HOPPING_PERIOD);

    sx127x_set_tx_power(dev, SX127X_RADIO_TX_POWER);
}

uint32_t sx127x_random(sx127x_t *dev)
{
    uint32_t rnd = 0;

    sx127x_set_modem(dev, SX127X_MODEM_LORA); /* Set LoRa modem ON */

    /* Disable LoRa modem interrupts */
    sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGSMASK, SX127X_RF_LORA_IRQFLAGS_RXTIMEOUT |
                     SX127X_RF_LORA_IRQFLAGS_RXDONE |
                     SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR |
                     SX127X_RF_LORA_IRQFLAGS_VALIDHEADER |
                     SX127X_RF_LORA_IRQFLAGS_TXDONE |
                     SX127X_RF_LORA_IRQFLAGS_CADDONE |
                     SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL |
                     SX127X_RF_LORA_IRQFLAGS_CADDETECTED);

    /* Set radio in continuous reception */
    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_RECEIVER);

    for (unsigned i = 0; i < 32; i++) {
        sx127x_timer_usleep(1000); /* wait for the chaos */

        /* Non-filtered RSSI value reading. Only takes the LSB value */
        rnd |= ((uint32_t) sx127x_reg_read(dev, SX127X_REG_LR_RSSIWIDEBAND) & 0x01) << i;
    }

    sx127x_set_sleep(dev);

    return rnd;
}

/**
 * IRQ handlers
 */
static void sx127x_on_dio0(int pin, void *arg)
{
    sx127x_t *dev = (sx127x_t *) arg;

    switch (dev->settings.state) {
        case SX127X_RF_RX_RUNNING:
            dev->event_callback(dev, SX127X_RX_DONE);
            break;
        case SX127X_RF_TX_RUNNING:
            sx127x_timer_disable(&dev->_internal.tx_timeout_timer);
            switch (dev->settings.modem) {
                case SX127X_MODEM_LORA:
                    /* Clear IRQ */
                    sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS,
                                     SX127X_RF_LORA_IRQFLAGS_TXDONE);
                /* Intentional fall-through */
                case SX127X_MODEM_FSK:
                default:
                    sx127x_set_state(dev, SX127X_RF_IDLE);
                    dev->event_callback(dev, SX127X_TX_DONE);
                    break;
            }
            break;
        case SX127X_RF_IDLE:
            sx127x_log(SX127X_DEBUG, "sx127x_on_dio0: IDLE state\n");
            break;
        default:
            sx127x_log(SX127X_DEBUG, "sx127x_on_dio0: Unknown state [%d]\n", dev->settings.state);
            break;
    }
}

static void sx127x_on_dio1(int pin, void *arg)
{
    /* Get interrupt context */
    sx127x_t *dev = (sx127x_t *) arg;

    switch (dev->settings.state) {
        case SX127X_RF_RX_RUNNING:
            switch (dev->settings.modem) {
                case SX127X_MODEM_FSK:
                    /* todo */
                    break;
                case SX127X_MODEM_LORA:
                    sx127x_timer_disable(&dev->_internal.rx_timeout_timer);
                    /*  Clear Irq */
                    sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS, SX127X_RF_LORA_IRQFLAGS_RXTIMEOUT);
                    sx127x_set_state(dev, SX127X_RF_IDLE);
                    dev->event_callback(dev, SX127X_RX_TIMEOUT);
                    break;
                default:
                    break;
            }
            break;
        case SX127X_RF_TX_RUNNING:
            switch (dev->settings.modem) {
                case SX127X_MODEM_FSK:
                    /* todo */
                    break;
                case SX127X_MODEM_LORA:
                    break;
                default:
                    break;
            }
            break;
        default:
            sx127x_log(SX127X_DEBUG, "sx127x_on_dio1: Unknown state");
            break;
    }
}

static void sx127x_on_dio2(int pin, void *arg)
{
    /* Get interrupt context */
    sx127x_t *dev = (sx127x_t *) arg;

    switch (dev->settings.state) {
        case SX127X_RF_RX_RUNNING:
            switch (dev->settings.modem) {
                case SX127X_MODEM_FSK:
                    /* todo */
                    break;
                case SX127X_MODEM_LORA:
                    if (dev->settings.lora.flags & SX127X_CHANNEL_HOPPING_FLAG) {
                        /* Clear IRQ */
                        sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS,
                                         SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL);

                        dev->_internal.last_channel = (sx127x_reg_read(dev, SX127X_REG_LR_HOPCHANNEL) &
                                                       SX127X_RF_LORA_HOPCHANNEL_CHANNEL_MASK);
                        dev->event_callback(dev, SX127X_FHSS_CHANGE_CHANNEL);
                    }

                    break;
                default:
                    break;
            }
            break;
        case SX127X_RF_TX_RUNNING:
            switch (dev->settings.modem) {
                case SX127X_MODEM_FSK:
                    break;
                case SX127X_MODEM_LORA:
                    if (dev->settings.lora.flags & SX127X_CHANNEL_HOPPING_FLAG) {
                        /* Clear IRQ */
                        sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS,
                                         SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL);

                        dev->_internal.last_channel = (sx127x_reg_read(dev, SX127X_REG_LR_HOPCHANNEL) &
                                                       SX127X_RF_LORA_HOPCHANNEL_CHANNEL_MASK);
                        dev->event_callback(dev, SX127X_FHSS_CHANGE_CHANNEL);
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            sx127x_log(SX127X_DEBUG, "sx127x_on_dio2: Unknown state");
            break;
    }
}

static void sx127x_on_dio3(int pin, void *arg)
{
    /* Get interrupt context */
    sx127x_t *dev = (sx127x_t *) arg;

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            break;
        case SX127X_MODEM_LORA:
            /* Clear IRQ */
            sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS,
                             SX127X_RF_LORA_IRQFLAGS_CADDETECTED |
                             SX127X_RF_LORA_IRQFLAGS_CADDONE);

            /* Send event message */
            dev->_internal.is_last_cad_success = (sx127x_reg_read(dev, SX127X_REG_LR_IRQFLAGS) &
                                                  SX127X_RF_LORA_IRQFLAGS_CADDETECTED) == SX127X_RF_LORA_IRQFLAGS_CADDETECTED;
            dev->event_callback(dev, SX127X_CAD_DONE);
            break;
        default:
            sx127x_log(SX127X_DEBUG, "sx127x_on_dio3: Unknown modem");
            break;
    }
}

static void _init_isrs(sx127x_t *dev)
{
    if (!sx127x_gpio_init_int(dev->params.dio0_pin, GPIO_IN, GPIO_PULL_UP,
        GPIO_RISING, sx127x_on_dio0, dev)) {
        sx127x_log(SX127X_ERROR, "Error: cannot initialize DIO0 pin\n");
    }

    if (!sx127x_gpio_init_int(dev->params.dio1_pin, GPIO_IN, GPIO_PULL_UP,
        GPIO_RISING, sx127x_on_dio1, dev)) {
        sx127x_log(SX127X_ERROR, "Error: cannot initialize DIO1 pin\n");
    }

    if (!sx127x_gpio_init_int(dev->params.dio2_pin, GPIO_IN, GPIO_PULL_UP,
        GPIO_RISING, sx127x_on_dio2, dev)) {
        sx127x_log(SX127X_ERROR, "Error: cannot initialize DIO2 pin\n");
    }

    if (!sx127x_gpio_init_int(dev->params.dio3_pin, GPIO_IN, GPIO_PULL_UP,
        GPIO_RISING, sx127x_on_dio3, dev)) {
        sx127x_log(SX127X_ERROR, "Error: cannot initialize DIO3 pin\n");
    }
}

static void _on_tx_timeout(void *arg)
{
    sx127x_t *dev = (sx127x_t *) arg;

    dev->event_callback(dev, SX127X_TX_TIMEOUT);
}

static void _on_rx_timeout(void *arg)
{
    sx127x_t *dev = (sx127x_t *) arg;

    dev->event_callback(dev, SX127X_RX_TIMEOUT);
}

static void _init_timers(sx127x_t *dev)
{
    dev->_internal.tx_timeout_timer.callback = _on_tx_timeout;
    dev->_internal.rx_timeout_timer.callback = _on_rx_timeout;
}