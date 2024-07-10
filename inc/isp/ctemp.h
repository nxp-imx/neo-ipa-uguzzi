/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef CTEMP_H_
#define CTEMP_H_

#include <stdint.h>

#define CTEMP_CFG_SUBTYPE_ID (3U)

#define CTEMP_COLOR_ROIS_CNT (10U)

/* NOTE(mms00430): Possibly combine these BPP enums into one for all IP blocks? */
/** Values from enum correspond to the values of input_bpp struct member */
typedef enum {
    CTEMP_CTRL_BPP_12_IN_16,
    CTEMP_CTRL_BPP_14_IN_16,
    CTEMP_CTRL_BPP_16_IN_16,
    CTEMP_CTRL_BPP_20_IN_20
} imx9x_isp_ctemp_ctrl_bpp_t;

typedef struct {
    uint8_t enable; /* min: 0, max: 1 */
    uint8_t use_csc; /* min: 0, max: 1 */
    uint8_t input_bpp; /* min: 0, max: 3 */
    /**
     * Select which CTemp model will be used for the AWB algorithm.
     * There is no HW register for this field! It is purely for SW convenience.
     * Use one of the following values:
     * 0 for CTemp Block Statistics
     * 1 for CTemp Color ROIs
     * 2 for CTemp Black Body Curve
     */
    uint8_t awb_stat_type; /* min: 0, max: 2 */
    uint16_t low_luma_threshold; /* min: 0, max: 65535 */
    uint16_t high_luma_threshold; /* min: 0, max: 65535 */
} imx9x_isp_ctemp_ctrl_cfg_t;

typedef struct {
    uint16_t x; /* min: 0, max: 65535 */
    uint16_t y; /* min: 0, max: 65535 */
    uint16_t width; /* min: 0, max: 65535 */
    uint16_t height; /* min: 0, max: 65535 */
} imx9x_isp_ctemp_roi_cfg_t;

typedef struct {
    uint8_t red_gain_min; /* min: 0, max: 255, format: u1.7 */
    uint8_t red_gain_max; /* min: 0, max: 255, format: u1.7 */
    uint8_t blue_gain_min; /* min: 0, max: 255, format: u1.7 */
    uint8_t blue_gain_max; /* min: 0, max: 255, format: u1.7 */
    uint8_t point1_blue; /* min: 0, max: 255, format: u1.7 */
    uint8_t point1_red; /* min: 0, max: 255, format: u1.7 */
    uint8_t point2_blue; /* min: 0, max: 255, format: u1.7 */
    uint8_t point2_red; /* min: 0, max: 255, format: u1.7 */
    uint8_t hoffset_right; /* min: 0, max: 255, format: u1.7 */
    uint8_t hoffset_left; /* min: 0, max: 255, format: u1.7 */
    uint8_t voffset_up; /* min: 0, max: 255, format: u1.7 */
    uint8_t voffset_down; /* min: 0, max: 255, format: u1.7 */
    int16_t point1_slope_left; /* min: -32768, max: 32767, format: s8.8 */
    int16_t point1_slope_right; /* min: -32768, max: 32767, format: s8.8 */
    int16_t point2_slope_left; /* min: -32768, max: 32767, format: s8.8 */
    int16_t point2_slope_right; /* min: -32768, max: 32767, format: s8.8 */
} imx9x_isp_ctemp_bbc_cfg_t;

typedef struct {
    uint32_t white_pixel_count;
    uint64_t red_sum;
    uint64_t green_sum;
    uint64_t blue_sum;
    /* 32 bits because these are sums of u1.7 values */
    uint32_t r_over_g_sum;
    uint32_t b_over_g_sum;
} imx9x_isp_ctemp_bbc_output_t;

typedef struct {
    uint8_t red_over_green_low; /* min: 0, max: 255, format: u1.7 */
    uint8_t red_over_green_high; /* min: 0, max: 255, format: u1.7 */
    uint8_t blue_over_green_low; /* min: 0, max: 255, format: u1.7 */
    uint8_t blue_over_green_high; /* min: 0, max: 255, format: u1.7 */
} imx9x_isp_ctemp_color_roi_desc_t;

typedef struct {
    /* 24 bit plain counter. Saturates at 2^24 - 1 */
    uint32_t pixel_count;
    /* pseudo floating point number with 28 bit mantissa and 4 bit exponent in the LSBs */
    uint32_t red_sum;
    /* pseudo floating point number with 28 bit mantissa and 4 bit exponent in the LSBs */
    uint32_t green_sum;
    /* pseudo floating point number with 28 bit mantissa and 4 bit exponent in the LSBs */
    uint32_t blue_sum;
} imx9x_isp_ctemp_color_roi_output_desc_t;

typedef struct {
    imx9x_isp_ctemp_color_roi_desc_t rois[CTEMP_COLOR_ROIS_CNT];
} imx9x_isp_ctemp_color_rois_cfg_t;

typedef struct {
    imx9x_isp_ctemp_color_roi_output_desc_t rois[CTEMP_COLOR_ROIS_CNT];
} imx9x_isp_ctemp_color_rois_output_t;

typedef struct {
    uint16_t paxels_width; /* min: 0, max: 65535 */
    uint16_t paxels_height; /* min: 0, max: 65535 */
} imx9x_isp_ctemp_blk_stats_cfg_t;

typedef struct {
    imx9x_isp_ctemp_ctrl_cfg_t ctrl;
    imx9x_isp_ctemp_roi_cfg_t roi;
    imx9x_isp_ctemp_bbc_cfg_t bbc;
    imx9x_isp_ctemp_blk_stats_cfg_t blocks_stats;
    imx9x_isp_ctemp_color_rois_cfg_t color_rois;
} imx9x_isp_ctemp_cfg_t;

#endif /* CTEMP_H_ */
