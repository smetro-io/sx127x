/*
 * Copyright (c) 2017 Smetro Technologies
 * All rights reserved
 */

#ifndef SX127X_H
#define SX127X_H

#include <stdbool.h>
#include "sx127x_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SX127X device default configuration
 */
#define SX127X_MODEM_DEFAULT             (SX127X_MODEM_LORA)    /**< Use LoRa as default modem */
#define SX127X_CHANNEL_DEFAULT           (915000000UL)          /**< Default channel frequency, 915MHz (North America) */
#define SX127X_HF_CHANNEL_DEFAULT        (915000000UL)          /**< Use to calibrate RX chain for LF and HF bands */
#define SX127X_RF_MID_BAND_THRESH        (525000000UL)          /**< Mid-band threshold */
#define SX127X_FREQUENCY_RESOLUTION      (61.03515625)          /**< Frequency resolution in Hz */
#define SX127X_XTAL_FREQ                 (32000000UL)           /**< Internal oscillator frequency, 32MHz */
#define SX127X_RADIO_WAKEUP_TIME         (1U)                   /**< In miliseconds [ms] */

#define SX127X_PREAMBLE_LENGTH           (8U)                   /**< Preamble length, same for Tx and Rx */
#define SX127X_SYMBOL_TIMEOUT            (512U)                  /**< Symbols timeout (s) */

#define SX127X_BW_DEFAULT                (SX127X_BW_125_KHZ)    /**< Set default bandwidth to 125kHz */
#define SX127X_SF_DEFAULT                (SX127X_SF11)          /**< Set default spreading factor to 10 */
#define SX127X_CR_DEFAULT                (SX127X_CR_4_5)        /**< Set default coding rate to 8 */
#define SX127X_FIX_LENGTH_PAYLOAD_ON     (false)                /**< Set fixed payload length on */
#define SX127X_IQ_INVERSION              (false)                /**< Set inverted IQ on */
#define SX127X_FREQUENCY_HOPPING         (false)                /**< Frequency hopping on */
#define SX127X_FREQUENCY_HOPPING_PERIOD  (0U)                   /**< Frequency hopping period */
#define SX127X_FIXED_HEADER_LEN_MODE     (false)                /**< Set fixed header length mode (implicit header) */
#define SX127X_PAYLOAD_CRC_ON            (true)                 /**< Enable payload CRC, optional */
#define SX127X_PAYLOAD_LENGTH            (0U)                   /**< Set payload length, unused with implicit header */

#define SX127X_TX_TIMEOUT_DEFAULT        (1000U * 4UL)         /**< TX timeout, 4s */
#define SX127X_RX_TIMEOUT_DEFAULT        (1000U * 2UL)         /**< RX timeout, 2s */
#define SX127X_RX_SINGLE                 (false)                /**< Single byte receive mode => continuous by default */
#define SX127X_RX_BUFFER_SIZE            (256)                  /**< RX buffer size */

#define SX127X_RADIO_TX_POWER            (14U)                  /**< Radio power in dBm */

#ifndef SX1272_DEFAULT_PASELECT
/** Default PA selection config (1: RFO, 0: PABOOST)
 *
 * This depends on the module configuration.
 */
#define SX1272_DEFAULT_PASELECT          (1U)
#endif

#define SX127X_EVENT_HANDLER_STACK_SIZE  (2048U) /**< Stack size event handler */
#define SX127X_IRQ_DIO0                  (1<<0)  /**< DIO0 IRQ */
#define SX127X_IRQ_DIO1                  (1<<1)  /**< DIO1 IRQ */
#define SX127X_IRQ_DIO2                  (1<<2)  /**< DIO2 IRQ */
#define SX127X_IRQ_DIO3                  (1<<3)  /**< DIO3 IRQ */
#define SX127X_IRQ_DIO4                  (1<<4)  /**< DIO4 IRQ */
#define SX127X_IRQ_DIO5                  (1<<5)  /**< DIO5 IRQ */

/**
 * SX127X initialization result.
 */
enum {
    SX127X_INIT_OK = 0,                /**< Initialization was successful */
    SX127X_ERR_SPI,                    /**< Failed to initialize SPI bus or CS line */
    SX127X_ERR_TEST_FAILED,            /**< SX127X testing failed during initialization (check chip) */
    SX127X_ERR_THREAD                  /**< Unable to create DIO handling thread (check amount of free memory) */
};

/**
 * Radio driver supported modems.
 */
enum {
    SX127X_MODEM_FSK = 0,              /**< FSK modem driver */
    SX127X_MODEM_LORA,                 /**< LoRa modem driver */
};

/**
 * LoRa signal bandwidth.
 */
enum {
    SX127X_BW_125_KHZ = 0,             /**< 125 kHz bandwidth */
    SX127X_BW_250_KHZ,                 /**< 250 kHz bandwidth */
    SX127X_BW_500_KHZ                  /**< 500 kHz bandwidth */
};

/**
 * LoRa spreading factor rate
 */
enum {
    SX127X_SF6 = 6,                    /**< spreading factor 6 */
    SX127X_SF7,                        /**< spreading factor 7 */
    SX127X_SF8,                        /**< spreading factor 8 */
    SX127X_SF9,                        /**< spreading factor 9 */
    SX127X_SF10,                       /**< spreading factor 10 */
    SX127X_SF11,                       /**< spreading factor 11 */
    SX127X_SF12                        /**< spreading factor 12 */
};

/**
 * LoRa error coding rate.
 */
enum {
    SX127X_CR_4_5 = 1,                 /**< coding rate 4/5 */
    SX127X_CR_4_6,                     /**< coding rate 4/6 */
    SX127X_CR_4_7,                     /**< coding rate 4/7 */
    SX127X_CR_4_8                      /**< coding rate 4/8 */
};

/**
 * Radio driver internal state machine states definition.
 */
enum {
    SX127X_RF_IDLE = 0,                /**< Idle state */
    SX127X_RF_RX_RUNNING,              /**< Sending state */
    SX127X_RF_TX_RUNNING,              /**< Receiving state */
    SX127X_RF_CAD,                     /**< Channel activity detection state */
};

/**
 * Event types.
 */
enum {
    SX127X_RX_DONE = 0,                /**< Receiving complete */
    SX127X_TX_DONE,                    /**< Sending complete*/
    SX127X_RX_TIMEOUT,                 /**< Receiving timeout */
    SX127X_TX_TIMEOUT,                 /**< Sending timeout */
    SX127X_RX_ERROR_CRC,               /**< Receiving CRC error */
    SX127X_FHSS_CHANGE_CHANNEL,        /**< Channel change */
    SX127X_CAD_DONE,                   /**< Channel activity detection complete */
};

/**
 * SX127X device descriptor boolean flags
 */
#define SX127X_LOW_DATARATE_OPTIMIZE_FLAG       (1 << 0)
#define SX127X_ENABLE_FIXED_HEADER_LENGTH_FLAG  (1 << 1)
#define SX127X_ENABLE_CRC_FLAG                  (1 << 2)
#define SX127X_CHANNEL_HOPPING_FLAG             (1 << 3)
#define SX127X_IQ_INVERTED_FLAG                 (1 << 4)
#define SX127X_RX_CONTINUOUS_FLAG               (1 << 5)

/**
 * LoRa configuration structure.
 */
typedef struct {
    uint16_t preamble_len;             /**< Length of preamble header */
    uint8_t power;                     /**< Signal power */
    uint8_t bandwidth;                 /**< Signal bandwidth */
    uint8_t datarate;                  /**< Spreading factor rate, e.g datarate */
    uint8_t coderate;                  /**< Error coding rate */
    uint8_t freq_hop_period;           /**< Frequency hop period */
    uint8_t flags;                     /**< Boolean flags */
    uint32_t rx_timeout;               /**< RX timeout in symbols */
    uint32_t tx_timeout;               /**< TX timeout in symbols */
} sx127x_lora_settings_t;

/**
 * Radio settings.
 */
typedef struct {
    uint32_t channel;                  /**< Radio channel */
    uint8_t state;                     /**< Radio state */
    uint8_t modem;                     /**< Driver model (FSK or LoRa) */
    sx127x_lora_settings_t lora;       /**< LoRa settings */
} sx127x_radio_settings_t;

/**
 * SX127X internal data.
 */
typedef struct {
    /* Data that will be passed to events handler in application */
    sx127x_timer_t tx_timeout_timer;   /**< TX operation timeout timer */
    sx127x_timer_t rx_timeout_timer;   /**< RX operation timeout timer */
    uint32_t last_channel;             /**< Last channel in frequency hopping sequence */
    bool is_last_cad_success;          /**< Sign of success of last CAD operation (activity detected) */
} sx127x_internal_t;

/**
 * SX127X hardware and global parameters.
 */
typedef struct {
    sx127x_spi_t *spi;              /**< SPI device */
    int reset_pin;                  /**< Reset pin */
    int dio0_pin;                   /**< Interrupt line DIO0 (Tx done) */
    int dio1_pin;                   /**< Interrupt line DIO1 (Rx timeout) */
    int dio2_pin;                   /**< Interrupt line DIO2 (FHSS channel change) */
    int dio3_pin;                   /**< Interrupt line DIO3 (CAD done) */
    int dio4_pin;                   /**< Interrupt line DIO4 (not used) */
    int dio5_pin;                   /**< Interrupt line DIO5 (not used) */
} sx127x_params_t;

/**
 * SX127X IRQ flags.
 */
typedef uint8_t sx127x_flags_t;

/**
 * SX127X events callback.
 */
typedef void (*sx127x_event_callback)(void *param, int event);

/**
 * SX127X device descriptor.
 */
typedef struct {
    sx127x_radio_settings_t settings;  /**< Radio settings */
    sx127x_params_t params;            /**< Device driver parameters */
    sx127x_internal_t _internal;       /**< Internal sx127x data used within the driver */
    sx127x_flags_t irq;                /**< Device IRQ flags */
    sx127x_event_callback event_callback; /**< Events callback */
} sx127x_t;

/**
 * Resets the SX127X
 *
 * @param[in] dev                      The sx127x device descriptor
 */
void sx127x_reset(const sx127x_t *dev);

/**
 * Initializes the transceiver.
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return result of initialization
 */
int sx127x_init(sx127x_t *dev);

/**
 * Initialize radio settings with default values
 *
 * @param[in] dev                      The sx127x device pointer
 */
void sx127x_init_radio_settings(sx127x_t *dev, sx127x_radio_settings_t *radio);

/**
 * Generates 32 bits random value based on the RSSI readings
 *
 * This function sets the radio in LoRa mode and disables all
 * interrupts from it. After calling this function either
 * sx127x_set_rx_config or sx127x_set_tx_config functions must
 * be called.
 *
 * @param[in] dev                      The sx127x device structure pointer
 *
 * @return random 32 bits value
 */
uint32_t sx127x_random(sx127x_t *dev);

/**
 * Start a channel activity detection.
 *
 * @param[in] dev                      The sx127x device descriptor
 */
void sx127x_start_cad(sx127x_t *dev);

/**
 * Checks that channel is free with specified RSSI threshold.
 *
 * @param[in] dev                      The sx127x device structure pointer
 * @param[in] freq                     channel RF frequency
 * @param[in] rssi_threshold           RSSI threshold
 *
 * @return true if channel is free, false otherwise
 */
bool sx127x_is_channel_free(sx127x_t *dev, uint32_t freq, int16_t rssi_threshold);

/**
 * Gets current state of transceiver.
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return radio state [RF_IDLE, RF_RX_RUNNING, RF_TX_RUNNING]
 */
uint8_t sx127x_get_state(const sx127x_t *dev);

/**
 * Sets current state of transceiver.
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] state                    The new radio state
 *
 * @return radio state [RF_IDLE, RF_RX_RUNNING, RF_TX_RUNNING]
 */
void sx127x_set_state(sx127x_t *dev, uint8_t state);

/**
 * Configures the radio with the given modem.
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] modem                    Modem to be used [0: FSK, 1: LoRa]
 */
void sx127x_set_modem(sx127x_t *dev, uint8_t modem);

/**
 * Gets the synchronization word.
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return The synchronization word
 */
uint8_t sx127x_get_syncword(const sx127x_t *dev);

/**
 * Sets the synchronization word.
 *
 * @param[in] dev                     The sx127x device descriptor
 * @param[in] syncword                The synchronization word
 */
void sx127x_set_syncword(sx127x_t *dev, uint8_t syncword);

/**
 * Gets the channel RF frequency.
 *
 * @param[in]  dev                     The sx127x device descriptor
 *
 * @return The channel frequency
 */
uint32_t sx127x_get_channel(const sx127x_t *dev);

/**
 * Sets the channel RF frequency.
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] freq                     Channel RF frequency
 */
void sx127x_set_channel(sx127x_t *dev, uint32_t freq);

/**
 * Computes the packet time on air in milliseconds.
 *
 * Can only be called if sx127x_init_radio_settings has already
 * been called.
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] pkt_len                  The received packet payload length
 *
 * @return computed air time (ms) for the given packet payload length
 */
uint32_t sx127x_get_time_on_air(const sx127x_t *dev, uint8_t pkt_len);

/**
 * Sets the radio in sleep mode
 *
 * @param[in] dev                      The sx127x device descriptor
 */
void sx127x_set_sleep(sx127x_t *dev);

/**
 * Sets the radio in stand-by mode
 *
 * @param[in] dev                      The sx127x device descriptor
 */
void sx127x_set_standby(sx127x_t *dev);

/**
 * Sets the radio in reception mode.
 *
 * @param[in] dev                      The sx127x device descriptor
 */
void sx127x_set_rx(sx127x_t *dev);

/**
 * Sets the radio in transmission mode.
 *
 * @param[in] dev                      The sx127x device descriptor
 */
void sx127x_set_tx(sx127x_t *dev);

/**
 * Gets the maximum payload length.
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return The maximum payload length
 */
uint8_t sx127x_get_max_payload_len(const sx127x_t *dev);

/**
 * Sets the maximum payload length.
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] maxlen                   Maximum payload length in bytes
 */
void sx127x_set_max_payload_len(const sx127x_t *dev, uint8_t maxlen);

/**
 * Gets the SX127X operating mode
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return The actual operating mode
 */
uint8_t sx127x_get_op_mode(const sx127x_t *dev);

/**
 * Sets the SX127X operating mode
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] op_mode                  The new operating mode
 */
void sx127x_set_op_mode(const sx127x_t *dev, uint8_t op_mode);

/**
 * Gets the SX127X bandwidth
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return the bandwidth
 */
uint8_t sx127x_get_bandwidth(const sx127x_t *dev);

/**
 * Sets the SX127X bandwidth
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] bandwidth                The new bandwidth
 */
void sx127x_set_bandwidth(sx127x_t *dev, uint8_t bandwidth);

/**
 * Gets the SX127X LoRa spreading factor
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return the spreading factor
 */
uint8_t sx127x_get_spreading_factor(const sx127x_t *dev);

/**
 * Sets the SX127X LoRa spreading factor
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] sf                       The spreading factor
 */
void sx127x_set_spreading_factor(sx127x_t *dev, uint8_t sf);

/**
 * Gets the SX127X LoRa coding rate
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return the current LoRa coding rate
 */
uint8_t sx127x_get_coding_rate(const sx127x_t *dev);

/**
 * @brief   Sets the SX127X LoRa coding rate
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] coderate                 The LoRa coding rate
 */
void sx127x_set_coding_rate(sx127x_t *dev, uint8_t coderate);

/**
 * Checks if the SX127X LoRa RX single mode is enabled/disabled
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return the LoRa single mode
 */
bool sx127x_get_rx_single(const sx127x_t *dev);

/**
 * Enable/disable the SX127X LoRa RX single mode
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] single                   The LoRa RX single mode
 */
void sx127x_set_rx_single(sx127x_t *dev, bool single);

/**
 * Checks if the SX127X CRC verification mode is enabled
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return the LoRa single mode
 */
bool sx127x_get_crc(const sx127x_t *dev);

/**
 * Enable/Disable the SX127X CRC verification mode
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] crc                      The CRC check mode
 */
void sx127x_set_crc(sx127x_t *dev, bool crc);

/**
 * Gets the SX127X frequency hopping period
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return the frequency hopping period
 */
uint8_t sx127x_get_hop_period(const sx127x_t *dev);

/**
 * Sets the SX127X frequency hopping period
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] hop_period               The frequency hopping period
 */
void sx127x_set_hop_period(sx127x_t *dev, uint8_t hop_period);

/**
 * Gets the SX127X LoRa fixed header length mode
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return the LoRa implicit mode
 */
bool sx127x_get_fixed_header_len_mode(const sx127x_t *dev);

/**
 * Sets the SX127X to fixed header length mode (explicit mode)
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] mode                     The header mode
 */
void sx127x_set_fixed_header_len_mode(sx127x_t *dev, bool mode);

/**
 * Gets the SX127X payload length
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return the payload length
 */
uint8_t sx127x_get_payload_length(const sx127x_t *dev);

/**
 * Sets the SX127X payload length
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] len                      The payload len
 */
void sx127x_set_payload_length(sx127x_t *dev, uint8_t len);

/**
 * Gets the SX127X TX radio power
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return the radio power
 */
uint8_t sx127x_get_tx_power(const sx127x_t *dev);

/**
 * Sets the SX127X transmission power
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] power                    The TX power
 */
void sx127x_set_tx_power(sx127x_t *dev, int8_t power);

/**
 * Gets the SX127X preamble length
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return the preamble length
 */
uint16_t sx127x_get_preamble_length(const sx127x_t *dev);

/**
 * Sets the SX127X LoRa preamble length
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] preamble                 The LoRa preamble length
 */
void sx127x_set_preamble_length(sx127x_t *dev, uint16_t preamble);

/**
 * Sets the SX127X LoRa symbol timeout
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] timeout                  The LoRa symbol timeout
 */
void sx127x_set_symbol_timeout(sx127x_t *dev, uint16_t timeout);

/**
 * Sets the SX127X RX timeout
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] timeout                  The RX timeout
 */
void sx127x_set_rx_timeout(sx127x_t *dev, uint32_t timeout);

/**
 * Sets the SX127X TX timeout
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] timeout                  The TX timeout
 */
void sx127x_set_tx_timeout(sx127x_t *dev, uint32_t timeout);

/**
 * Checks if the SX127X LoRa inverted IQ mode is enabled/disabled
 *
 * @param[in] dev                      The sx127x device descriptor
 *
 * @return the LoRa IQ inverted mode
 */
bool sx127x_get_iq_invert(const sx127x_t *dev);

/**
 * Sets the SX127X LoRa IQ inverted mode
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] iq_invert                The LoRa IQ inverted mode
 */
void sx127x_set_iq_invert(sx127x_t *dev, bool iq_invert);

/**
 * Sets the SX127X LoRa frequency hopping mode
 *
 * @param[in] dev                      The sx127x device descriptor
 * @param[in] freq_hop_on              The LoRa frequency hopping mode
 */
void sx127x_set_freq_hop(sx127x_t *dev, bool freq_hop_on);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SX127X_H */
