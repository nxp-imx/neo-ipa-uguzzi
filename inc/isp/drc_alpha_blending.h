/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef DRC_ALPHA_BLENDING_H_
#define DRC_ALPHA_BLENDING_H_

#include <stdint.h>

typedef struct {
    uint16_t alpha_blending_factor; /* min: 0, max: 256 */
} imx9x_isp_drc_alpha_blending_cfg_t;

#endif /* DRC_ALPHA_BLENDING_H_ */
