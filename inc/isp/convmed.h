/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef CONVMED_H_
#define CONVMED_H_

#include <stdint.h>

typedef enum {
    CCONVMED_FLT_BYPASS,
    CCONVMED_FLT_CONVOLUTION,
    CCONVMED_FLT_MEDIAN,
    CCONVMED_FLT_INVALID = INT32_MAX
} imx9x_isp_convmed_filter_t;

typedef struct {
    imx9x_isp_convmed_filter_t filter_type; /* min: 0, max: 2 */
} imx9x_isp_convmed_cfg_t;

#endif /* CONVMED_H_ */
