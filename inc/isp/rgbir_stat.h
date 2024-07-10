/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef RGBIR_STAT_H_
#define RGBIR_STAT_H_

#include "stat_common.h"
#include <stdint.h>

#define RGBIR_STAT_HIST_CNT (2U)

/** Values from enum correspond to the values of imx9x_isp_stat_hist_cfg_t channel_selection struct member */
typedef enum {
    RGBIR_STAT_CHANNEL_RB = 1, /* Red/Blue pixels in the RGBIr pattern */
    RGBIR_STAT_CHANNEL_GR = 2, /* Gr pixels in the RGBIr pattern */
    RGBIR_STAT_CHANNEL_GB = 4, /* Gb pixels in the RGBIr pattern */
    RGBIR_STAT_CHANNEL_IR = 8 /* Ir pixels in the RGBIr pattern */
} imx9x_isp_rgbir_stat_channel_t;

typedef struct {
    imx9x_isp_stat_roi_cfg_t foreground; /* ROI0 */
    imx9x_isp_stat_roi_cfg_t background; /* ROI1 */
    imx9x_isp_stat_hist_cfg_t hists[RGBIR_STAT_HIST_CNT];
} imx9x_isp_rgbir_stat_cfg_t;

#endif /* RGBIR_STAT_H_ */
