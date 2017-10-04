/*
 * Copyright (c) 2017 Smetro Technologies
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include "sx127x_hal.h"
#include "sx127x_internal.h"

int sx127x_init(sx127x_t *dev)
{
    /* Check presence of SX127X */
    if (!sx127x_test(dev)) {
        sx127x_log(SX127X_ERROR, "[Error] init : sx127x test failed\n");
        return -SX127X_ERR_TEST_FAILED;
    }

#if 0
    _init_timers(dev);
    xtimer_usleep(1000); /* wait 1 millisecond */

    sx127x_reset(dev);

    sx127x_rx_chain_calibration(dev);
    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_SLEEP);

    _init_isrs(dev);
#endif

    return SX127X_INIT_OK;
}
