/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef DRC_LOCAL_TONEMAP_LUT_H_
#define DRC_LOCAL_TONEMAP_LUT_H_

#include <stdint.h>

#define DRC_LOCAL_TONEMAP_SIZE_X (32)
#define DRC_LOCAL_TONEMAP_SIZE_Y (32)
#define DRC_LOCAL_TONEMAP_SIZE (DRC_LOCAL_TONEMAP_SIZE_X * DRC_LOCAL_TONEMAP_SIZE_Y)

typedef struct {
    uint8_t local_tonemap_LUT[DRC_LOCAL_TONEMAP_SIZE]; /* min: 0, max: 255, format: u0.8 */
} imx9x_isp_drc_local_tonemap_lut_cfg_t;

#endif /* DRC_LOCAL_TONEMAP_LUT_H_ */
