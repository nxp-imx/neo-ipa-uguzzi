/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef GCM_INPUT_CSC_H_
#define GCM_INPUT_CSC_H_

#include <stdint.h>

#define GCM_INPUT_CSC_CFG_SUBTYPE_ID (1U)

#define GCM_INPUT_CSC_MATRIX_ROWS (3U)
#define GCM_INPUT_CSC_MATRIX_COLS (3U)
#define GCM_INPUT_CSC_OFFSETS_SIZE (3U)

typedef struct {
    int16_t matrix[GCM_INPUT_CSC_MATRIX_ROWS][GCM_INPUT_CSC_MATRIX_COLS]; /* min: -32768, max: 32767, format: s8.8 */
    int16_t offsets[GCM_INPUT_CSC_OFFSETS_SIZE]; /* min: -32768, max: 32767 */
} imx9x_isp_gcm_input_csc_cfg_t;

#endif /* GCM_INPUT_CSC_H_ */
