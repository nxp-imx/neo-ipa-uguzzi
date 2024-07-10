/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef IR_COMPRESS_H_
#define IR_COMPRESS_H_

#include <stdint.h>

/* TODO(mms00430): Is this correct? Is it really 8 bits in 16 bits? */
/** Values from enum correspond to the values of output_bpp struct member */
typedef enum {
    IR_COMPRESS_BPP_8_IN_16,
    IR_COMPRESS_BPP_16_IN_16,
} imx9x_isp_ir_compress_bpp_t;

typedef struct {
    uint16_t enable; /* min: 0, max: 1 */
    uint16_t output_bpp; /* min: 0, max: 1 */
    uint32_t knee_point1; /* min: 0, max: 1048575 */
    uint32_t knee_point2; /* min: 0, max: 1048575 */
    uint32_t knee_point3; /* min: 0, max: 1048575 */
    uint32_t knee_point4; /* min: 0, max: 1048575 */
    uint32_t knee_offset0; /* min: 0, max: 1048575 */
    uint32_t knee_offset1; /* min: 0, max: 1048575 */
    uint32_t knee_offset2; /* min: 0, max: 1048575 */
    uint32_t knee_offset3; /* min: 0, max: 1048575 */
    uint32_t knee_offset4; /* min: 0, max: 1048575 */
    uint16_t ratio0; /* min: 0, max: 65535, format: u1.15 */
    uint16_t ratio1; /* min: 0, max: 65535, format: u1.15 */
    uint16_t ratio2; /* min: 0, max: 65535, format: u1.15 */
    uint16_t ratio3; /* min: 0, max: 65535, format: u1.15 */
    uint16_t ratio4; /* min: 0, max: 65535, format: u1.15 */
    uint16_t knee_npoint0; /* min: 0, max: 65535 */
    uint16_t knee_npoint1; /* min: 0, max: 65535 */
    uint16_t knee_npoint2; /* min: 0, max: 65535 */
    uint16_t knee_npoint3; /* min: 0, max: 65535 */
    uint16_t knee_npoint4; /* min: 0, max: 65535 */
} imx9x_isp_ir_compress_cfg_t;

#endif /* IR_COMPRESS_H_ */
