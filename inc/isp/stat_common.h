/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef STAT_COMMON_H_
#define STAT_COMMON_H_

#include <stdint.h>

/** Values from enum correspond to the values of type struct member */
typedef enum {
    STAT_HIST_TYPE_LINEAR,
    STAT_HIST_TYPE_LOGARITHMIC
} imx9x_isp_stat_hist_type_t;

/** Values from enum correspond to the values of binning_method struct member */
typedef enum {
    STAT_HIST_BINNING_DIRECT,
    STAT_HIST_BINNING_DIFFERENCE
} imx9x_isp_stat_hist_binning_t;

/** Values from enum correspond to the values of neighboring_pattern struct member */
typedef enum {
    STAT_HIST_NEIGHBORING_1X1,
    STAT_HIST_NEIGHBORING_2X2
} imx9x_isp_stat_hist_neighboring_t;

typedef struct {
    uint16_t x; /* min: 0, max: 65535 */
    uint16_t y; /* min: 0, max: 65535 */
    uint16_t width; /* min: 0, max: 65535 */
    uint16_t height; /* min: 0, max: 65535 */
} imx9x_isp_stat_roi_cfg_t;

typedef struct {
    uint8_t type; /* min: 0, max: 1 */
    uint8_t binning_method; /* min: 0, max: 1 */
    uint8_t neighboring_pattern; /* min: 0, max: 1 */
    uint8_t channel_selection; /* min: 0, max: 15 */
    /* NOTE(mms00430): uint32_t is used to avoid padding bytes! */
    uint32_t blc_offset; /* min: 0, max: 65535 */
    uint32_t scale; /* min: 0, max: 16777215, format: u8.16 */
} imx9x_isp_stat_hist_cfg_t;

#endif /* STAT_COMMON_H_ */
