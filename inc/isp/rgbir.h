/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef RGBIR_H_
#define RGBIR_H_

#include <stdint.h>

#define RGBIR_CFG_SUBTYPE_ID (2U)

typedef struct {
    uint8_t enable; /* min: 0, max: 1 */
    uint8_t frame_bitdepth; /* min: 0, max: 20 */
    int16_t red_correction; /* min: -32768, max: 32767, format: s4.12 */
    int16_t green_correction; /* min: -32768, max: 32767, format: s4.12 */
    int16_t blue_correction; /* min: -32768, max: 32767, format: s4.12 */
    uint16_t crosstalk_threshold_red_prc; /* min: 0, max: 32768, format: u0.15 */
    uint16_t crosstalk_threshold_green_prc; /* min: 0, max: 32768, format: u0.15 */
    uint16_t crosstalk_threshold_blue_prc; /* min: 0, max: 32768, format: u0.15 */
} imx9x_isp_rgbir_cfg_t;

#endif /* RGBIR_H_ */
