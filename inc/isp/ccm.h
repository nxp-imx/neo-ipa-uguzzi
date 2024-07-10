/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef CCM_H_
#define CCM_H_

#include <stdint.h>
#define CCM_MATRIX_SIZE 3u

typedef struct {
    int16_t ccm_matrix[CCM_MATRIX_SIZE][CCM_MATRIX_SIZE]; /* min: -32768, max: 32767, format: s8.8 */
} imx9x_isp_ccm_cfg_t;

#endif /* CCM_H_ */
