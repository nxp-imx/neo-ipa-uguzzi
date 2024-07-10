/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef DRC_GLOBAL_TONEMAP_CTRL_H_
#define DRC_GLOBAL_TONEMAP_CTRL_H_

#include <stdint.h>

typedef struct {
    uint16_t x; /* min: 0, max: 65535 */
    uint16_t y; /* min: 0, max: 65535 */
    uint16_t width; /* min: 0, max: 65535 */
    uint16_t height; /* min: 0, max: 65535 */
    uint16_t right_shift; /* min: 0, max: 31 */
} imx9x_isp_drc_global_roi_cfg_t;

typedef struct {
    imx9x_isp_drc_global_roi_cfg_t hist_roi_0;
    imx9x_isp_drc_global_roi_cfg_t hist_roi_1;
} imx9x_isp_drc_global_tonemap_ctrl_cfg_t;

#endif /* DRC_GLOBAL_TONEMAP_CTRL_H_ */
