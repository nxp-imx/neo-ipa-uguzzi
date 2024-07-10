/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef RGB2YUV_H_
#define RGB2YUV_H_

#include <stdint.h>
#define RGB2YUV_MATRIX_SIZE 3u

typedef struct {
    uint16_t red_gain; /* min: 0, max: 65535, format: u8.8 */
    uint16_t blue_gain; /* min: 0, max: 65535, format: u8.8 */
    int16_t csc_matrix[RGB2YUV_MATRIX_SIZE][RGB2YUV_MATRIX_SIZE]; /* min: 0, max: 32767, format: s8.8 */
    int32_t csc_offsets[RGB2YUV_MATRIX_SIZE]; /* min: -1048576, max: 1048575 */
} imx9x_isp_rgb2yuv_cfg_t;

#endif /* RGB2YUV_H_ */
