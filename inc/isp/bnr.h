/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef BNR_H_
#define BNR_H_

#include <stdint.h>

/** Values from enum correspond to the values of neighbourhood_pattern struct member */
typedef enum {
    BNR_NHOOD_PATTERN_2x2,
    BNR_NHOOD_PATTERN_1x1,
    BNR_NHOOD_PATTERN_INVALID = INT32_MAX
} imx9x_isp_bnr_neighbourhood_pattern_t;

/** Values from enum correspond to the values of debug struct member */
typedef enum {
    BNR_DEBUG_OFF,
    BNR_DEBUG_FINAL_EDGE_VERSUS_TEXTURE,
    BNR_DEBUG_FINAL_EDGE_VERSUS_BLACK,
    BNR_DEBUG_EDGE_PIX_L_FILTER,
    BNR_DEBUG_EDGE_PIX_S_FILTER,
    BNR_DEBUG_L_VERSUS_S,
    BNR_DEBUG_INVALID = INT32_MAX
} imx9x_isp_bnr_debug_t;

/** Values from enum correspond to the values of output_bpp struct member */
typedef enum {
    BNR_OUT_BPP12,
    BNR_OUT_BPP14,
    BNR_OUT_BPP16,
    BNR_OUT_BPP20,
    BNR_OUT_BPP_INVALID = INT32_MAX
} imx9x_isp_bnr_out_bpp_t;

/** Values from enum correspond to the values of peak_select struct member */
typedef enum {
    BNR_POS_1_FROM_EXTREME,
    BNR_POS_2_FROM_EXTREME,
    BNR_POS_3_FROM_EXTREME,
    BNR_POS_4_FROM_EXTREME,
    BNR_BPP_INVALID = INT32_MAX
} imx9x_isp_bnr_peak_select_t;

/* TODO(tkatsarski): Possibly combine c and y structures into a single one.
    Verify with NXP if the diferring fields have the same names. */
typedef struct {
    uint16_t peak_output_scaling_enable; /* min: 0, max: 1 */
    uint16_t peak_select; /* min: 0, max: 3 */
    uint16_t peak_lower_scale; /* min: 0, max: 4095, format: u4.8 */
    uint16_t peak_higher_scale; /* min: 0, max: 4095, format: u4.8 */
    uint32_t edge_lower_threshold; /* min: 0, max: 1048575 */
    uint16_t long_scale; /* min: 0, max: 65535 */
    uint16_t long_shift; /* min: 0, max: 31 */
    uint32_t short_edge_lower_threshold; /* min: 0, max: 1048575 */
    uint16_t short_scale; /* min: 0, max: 65535 */
    uint16_t short_shift; /* min: 0, max: 31 */
    uint32_t alpha_edge_lower_threshold; /* min: 0, max: 1048575 */
    uint16_t alpha_scale; /* min: 0, max: 65535 */
    uint16_t alpha_shift; /* min: 0, max: 31 */
    uint16_t alpha_gain; /* min: 0, max: 65535, format: u8.8 */
    uint16_t alpha_offset; /* min: 0, max: 65535 */
    uint32_t luma_x_threhshold; /* min: 0, max: 1048575 */
    uint16_t luma_y_threshold_0; /* min: 0, max: 1023 */
    uint16_t luma_y_threshold_1; /* min: 0, max: 1023 */
    uint16_t luma_scale; /* min: 0, max: 65535 */
    uint16_t luma_shift; /* min: 0, max: 31 */
} imx9x_isp_bnr_y_cfg_t;

typedef struct {
    uint16_t peak_output_scaling_enable; /* min: 0, max: 1 */
    uint16_t peak_select; /* min: 0, max: 3 */
    uint16_t peak_lower_scale; /* min: 0, max: 4095, format: u4.8 */
    uint16_t peak_higher_scale; /* min: 0, max: 4095, format: u4.8 */
    uint32_t edge_lower_threshold; /* min: 0, max: 1048575 */
    uint16_t scale; /* min: 0, max: 65535 */
    uint16_t shift; /* min: 0, max: 31 */
    uint32_t short_edge_lower_threshold; /* min: 0, max: 1048575 */
    uint16_t short_scale; /* min: 0, max: 65535 */
    uint16_t short_shift; /* min: 0, max: 31 */
    uint32_t alpha_edge_lower_threshold; /* min: 0, max: 1048575 */
    uint16_t alpha_scale; /* min: 0, max: 65535 */
    uint16_t alpha_shift; /* min: 0, max: 31 */
    uint16_t alpha_gain; /* min: 0, max: 65535, format: u8.8 */
    uint16_t alpha_offset; /* min: 0, max: 65535 */
    uint32_t luma_x_threhshold; /* min: 0, max: 1048575 */
    uint16_t luma_y_threshold_0; /* min: 0, max: 1023 */
    uint16_t luma_y_threshold_1; /* min: 0, max: 1023 */
    uint16_t luma_scale; /* min: 0, max: 65535 */
    uint16_t luma_shift; /* min: 0, max: 31 */
} imx9x_isp_bnr_c_cfg_t;

typedef struct {
    uint8_t enable; /* min: 0, max: 1 */
    uint8_t debug; /* min: 0, max: 5 */
    uint8_t output_bpp; /* min: 0, max: 3 */
    uint8_t neighbourhood_pattern; /* min: 0, max: 1 */
    imx9x_isp_bnr_y_cfg_t y_config;
    imx9x_isp_bnr_c_cfg_t c_config;
    uint16_t output_gain; /* min: 0, max: 65535, format: u8.8 */
} imx9x_isp_bnr_cfg_t;

#endif /* BNR_H_ */
