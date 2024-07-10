/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef DF_H_
#define DF_H_

#include <stdint.h>

typedef enum {
    DF_DEBUG_OFF,
    DF_DEBUG_FILT_TEXTURE,
    DF_DEBUG_FILT_WHITE,
    DF_DEBUG_HOR_FILT_WHITE,
    DF_DEBUG_VER_FILT_WHITE,
    DF_DEBUG_DIAG1_FILT_WHITE,
    DF_DEBUG_DIAG2_FILT_WHITE,
    DF_DEBUG_INVALID = INT32_MAX
} imx9x_isp_df_debug_t;

typedef struct {
    uint32_t enable; /* min: 0, max: 1 */
    imx9x_isp_df_debug_t output_debug_info; /* min: 0, max: 6 */
    uint32_t blending_scale; /* min: 0, max: 1048575 */
    uint32_t blending_right_shift; /* min: 0, max: 63 */
    uint32_t blending_threshold_0; /* min: 0, max: 1048575 */
} imx9x_isp_df_cfg_t;

#endif /* DF_H_ */
