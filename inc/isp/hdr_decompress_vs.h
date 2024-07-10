/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef HDR_DECOMPRESS_VS_H_
#define HDR_DECOMPRESS_VS_H_

#include <stdint.h>

/* TODO(mms00430): Sync with NXP the signedness, format and ranges */
typedef struct {
    uint16_t enable; /* min: 0, max: 1 */
    uint16_t knee_point1; /* min: 0, max: 65535 */
    uint16_t knee_point2; /* min: 0, max: 65535 */
    uint16_t knee_point3; /* min: 0, max: 65535 */
    uint16_t knee_point4; /* min: 0, max: 65535 */
    uint16_t knee_offset0; /* min: 0, max: 65535 */
    uint16_t knee_offset1; /* min: 0, max: 65535 */
    uint16_t knee_offset2; /* min: 0, max: 65535 */
    uint16_t knee_offset3; /* min: 0, max: 65535 */
    uint16_t knee_offset4; /* min: 0, max: 65535 */
    uint16_t ratio0; /* min: 0, max: 4095, format: u7.5 */
    uint16_t ratio1; /* min: 0, max: 4095, format: u7.5 */
    uint16_t ratio2; /* min: 0, max: 4095, format: u7.5 */
    uint16_t ratio3; /* min: 0, max: 4095, format: u7.5 */
    uint16_t ratio4; /* min: 0, max: 4095, format: u7.5 */
    uint16_t knee_npoint0; /* min: 0, max: 65535 */
    uint16_t knee_npoint1; /* min: 0, max: 65535 */
    uint16_t knee_npoint2; /* min: 0, max: 65535 */
    uint16_t knee_npoint3; /* min: 0, max: 65535 */
    uint16_t knee_npoint4; /* min: 0, max: 65535 */
} imx9x_isp_hdr_decompress_vs_cfg_t;

#endif /* HDR_DECOMPRESS_VS_H_ */
