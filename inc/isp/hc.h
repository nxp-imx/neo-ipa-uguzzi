/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef HC_H_
#define HC_H_

#include <stdint.h>

typedef struct {
    uint8_t horizontal_offset; /* min: 0, max: 3 */
    uint8_t vertical_offset; /* min: 0, max: 3 */
} imx9x_isp_hc_cfg_t;

#endif /* HC_H_ */
