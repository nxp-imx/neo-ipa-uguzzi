/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef DRC_GLOBAL_TONEMAP_LUT_H_
#define DRC_GLOBAL_TONEMAP_LUT_H_

#include <stdint.h>

#define DRC_GLOBAL_TONEMAP_SIZE (416u)

typedef struct {
    uint16_t global_tonemap_LUT[DRC_GLOBAL_TONEMAP_SIZE]; /* min: 0, max: 65535, format: u8.8 */
} imx9x_isp_drc_global_tonemap_lut_cfg_t;

#endif /* DRC_GLOBAL_TONEMAP_LUT_H_ */
