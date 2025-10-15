/*******************************************************************************
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef AUTOFOCUS_H_
#define AUTOFOCUS_H_

#include <stdint.h>

#define AUTOFOCUS_ROI_CNT 9u
#define AUTOFOCUS_FIL_COEFFS_CNT 9u

typedef struct {
    /* The sum from filter0 of each ROI */
    uint32_t filter0_sums[AUTOFOCUS_ROI_CNT];
    /* The sum from filter1 of each ROI */
    uint32_t filter1_sums[AUTOFOCUS_ROI_CNT];
} imx9x_isp_autofocus_output_t;

typedef struct {
    uint16_t x; /* min: 0, max: 65535 */
    uint16_t y; /* min: 0, max: 65535 */
    uint16_t width; /* min: 0, max: 65535 */
    uint16_t height; /* min: 0, max: 65535 */
} imx9x_isp_autofocus_roi_cfg_t;

typedef struct {
    imx9x_isp_autofocus_roi_cfg_t rois_config[AUTOFOCUS_ROI_CNT];
    int8_t filter0_coefficients[AUTOFOCUS_FIL_COEFFS_CNT]; /* min: -128, max: 127 */
    uint8_t filter0_shift; /* min: 0, max: 31 */
    int8_t filter1_coefficients[AUTOFOCUS_FIL_COEFFS_CNT]; /* min: -128, max: 127 */
    uint8_t filter1_shift; /* min: 0, max: 31 */
} imx9x_isp_autofocus_cfg_t;

#endif /* AUTOFOCUS_H_ */
