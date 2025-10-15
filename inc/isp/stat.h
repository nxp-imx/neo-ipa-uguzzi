/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef STAT_H_
#define STAT_H_

#include "stat_common.h"
#include <stdint.h>

#define STAT_HIST_CNT (4U)

/** Values from enum correspond to the values of imx9x_isp_stat_hist_cfg_t channel_selection struct member */
typedef enum {
    STAT_CHANNEL_R = 1,
    STAT_CHANNEL_GR = 2,
    STAT_CHANNEL_GB = 4,
    STAT_CHANNEL_B = 8
} imx9x_isp_stat_channel_t;

typedef struct {
    imx9x_isp_stat_roi_cfg_t foreground; /* ROI0 */
    imx9x_isp_stat_roi_cfg_t background; /* ROI1 */
    imx9x_isp_stat_hist_cfg_t hists[STAT_HIST_CNT];
} imx9x_isp_stat_cfg_t;

#endif /* STAT_H_ */
