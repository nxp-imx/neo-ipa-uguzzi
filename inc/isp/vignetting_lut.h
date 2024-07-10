/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef VIGNETTING_LUT_H_
#define VIGNETTING_LUT_H_

#include <stdint.h>
#define VIGNETTING_TABLE_SIZE 3072u

typedef struct {
    uint16_t vignetting_table[VIGNETTING_TABLE_SIZE]; /* min: 0, max: 1023, format: u3.7 */
} imx9x_isp_vignetting_lut_cfg_t;

#endif /* VIGNETTING_LUT_H_ */
