/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef DRC_GLOBAL_GAIN_FACTOR_H_
#define DRC_GLOBAL_GAIN_FACTOR_H_

#include <stdint.h>

typedef struct {
    uint16_t gain_factor; /* min: 0, max: 65535, format: u8.8 */
} imx9x_isp_drc_global_gain_factor_cfg_t;

#endif /* DRC_GLOBAL_GAIN_FACTOR_H_ */
