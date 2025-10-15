/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef GCM_OUTPUT_CSC_H_
#define GCM_OUTPUT_CSC_H_

#include <stdint.h>

#define GCM_OUTPUT_CSC_CFG_SUBTYPE_ID (1U)

#define GCM_OUTPUT_CSC_MATRIX_ROWS (3U)
#define GCM_OUTPUT_CSC_MATRIX_COLS (3U)
#define GCM_OUTPUT_CSC_OFFSETS_SIZE (3U)

/** Values from enum correspond to the values of sign_config struct member */
typedef enum {
    GCM_OUTPUT_CSC_SIGNED = 0,
    GCM_OUTPUT_CSC_UNSIGNED = 1
} imx9x_isp_gcm_output_csc_signedness_t;

typedef struct {
    uint16_t sign_config; /* min: 0, max: 1 */
    int16_t matrix[GCM_OUTPUT_CSC_MATRIX_ROWS][GCM_OUTPUT_CSC_MATRIX_COLS]; /* min: -32768, max: 32767, format: s8.8 */
    int16_t offsets[GCM_OUTPUT_CSC_OFFSETS_SIZE]; /* min: -4096, max: 4095 */
} imx9x_isp_gcm_output_csc_cfg_t;

#endif /* GCM_OUTPUT_CSC_H_ */
