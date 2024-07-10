/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef GCM_GAMMA_H_
#define GCM_GAMMA_H_

#include <stdint.h>

#define GCM_GAMMA_CFG_SUBTYPE_ID (1U)

typedef struct {
    uint16_t gamma_power_ch0; /* min: 0, max: 511, format: u1.8 */
    uint16_t gamma_power_ch1; /* min: 0, max: 511, format: u1.8 */
    uint16_t gamma_power_ch2; /* min: 0, max: 511, format: u1.8 */
    uint16_t gamma_offset_ch0; /* min: 0, max: 4095 */
    uint16_t gamma_offset_ch1; /* min: 0, max: 4095 */
    uint16_t gamma_offset_ch2; /* min: 0, max: 4095 */
    uint16_t linear_gain_ch0; /* min: 0, max: 65535, format: u8.8 */
    uint16_t linear_gain_ch1; /* min: 0, max: 65535, format: u8.8 */
    uint16_t linear_gain_ch2; /* min: 0, max: 65535, format: u8.8 */
    int16_t linear_offset_ch0; /* min: -32768, max: 32767 */
    int16_t linear_offset_ch1; /* min: -32768, max: 32767 */
    int16_t linear_offset_ch2; /* min: -32768, max: 32767 */
    uint16_t linear_threshold_ch0; /* min: 0, max: 65535, format: u12.4 */
    uint16_t linear_threshold_ch1; /* min: 0, max: 65535, format: u12.4 */
    uint16_t linear_threshold_ch2; /* min: 0, max: 65535, format: u12.4 */
} imx9x_isp_gcm_gamma_cfg_t;

#endif /* GCM_GAMMA_H_ */
