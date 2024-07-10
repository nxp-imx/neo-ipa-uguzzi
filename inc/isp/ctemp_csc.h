/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef CTEMP_CSC_H_
#define CTEMP_CSC_H_

#include <stdint.h>

#define CTEMP_CSC_MATRIX_SIZE (3U)
#define CTEMP_CSC_OFFSET_VECTOR_SIZE (4U)

typedef struct {
    int16_t csc_matrix[CTEMP_CSC_MATRIX_SIZE][CTEMP_CSC_MATRIX_SIZE]; /* min: -32768, max: 32767, format: s8.8 */
    /* Order is R Gr Gb B */
    /* NOTE(mms00430): The format and range of the values is to be confirmed by NXP */
    int16_t offsets[CTEMP_CSC_OFFSET_VECTOR_SIZE]; /* min: -32768, max: 32767 */
} imx9x_isp_ctemp_csc_cfg_t;

#endif /* CTEMP_CSC_H_ */
