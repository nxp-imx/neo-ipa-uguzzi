/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef NR_H_
#define NR_H_

#include <stdint.h>

typedef enum {
    NR_DEBUG_OFF,
    NR_DEBUG_NON_FILTERED_TEXTURE,
    NR_DEBUG_NON_FILTERED_WHITE,
    NR_DEBUG_INVALID = INT32_MAX
} imx9x_isp_nr_debug_t;

typedef struct {
    uint32_t enable; /* min: 0, max: 1 */
    imx9x_isp_nr_debug_t output_debug_info; /* min: 0, max: 2 */
    uint32_t blending_scale; /* min: 0, max: 65535 */
    uint32_t blending_right_shift; /* min: 0, max: 255 */
    uint32_t blending_gain; /* min: 0, max: 255 */
    uint32_t blending_threshold_0; /* min: 0, max: 1048575 */
} imx9x_isp_nr_cfg_t;

#endif /* NR_H_ */
