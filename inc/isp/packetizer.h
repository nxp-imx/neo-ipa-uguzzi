/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef PACKETIZER_H_
#define PACKETIZER_H_

#include <stdint.h>

/** Values from enum correspond to the values of ch0_output_bpp struct member */
typedef enum {
    PACKETIZER_BPP_CH0_BPP12 = 0,
    PACKETIZER_BPP_CH0_BPP16 = 2,
    PACKETIZER_BPP_CH0_BPP10 = 4,
    PACKETIZER_BPP_CH0_BPP8 = 6,
    PACKETIZER_BPP_CH0_INVALID = INT32_MAX
} imx9x_isp_packetizer_bpp_ch0_t;

/** Values from enum correspond to the values of ch12_output_bpp struct member */
typedef enum {
    PACKETIZER_BPP_CH12_BPP12 = 0,
    PACKETIZER_BPP_CH12_BPP10 = 4,
    PACKETIZER_BPP_CH12_BPP8 = 6,
    PACKETIZER_BPP_CH12_INVALID = INT32_MAX
} imx9x_isp_packetizer_bpp_ch1_t;

/** Values from enum correspond to the values of ch12_subsample struct member */
typedef enum {
    PACKETIZER_SUBSAMPLE_YUV444 = 0,
    PACKETIZER_SUBSAMPLE_YUV422 = 1,
    PACKETIZER_SUBSAMPLE_YUV420 = 2,
    PACKETIZER_SUBSAMPLE_INVALID = INT32_MAX
} imx9x_isp_packetizer_subsample_t;

/** Values from enum correspond to the values of packing_type struct member */
typedef enum {
    PACKETIZER_PCK_TYPE_SEMI_PLANAR,
    PACKETIZER_PCK_TYPE_INTERLEAVED,
    PACKETIZER_PCK_TYPE_INVALID = INT32_MAX
} imx9x_isp_packetizer_pck_type_t;

typedef struct {
    uint8_t packing_type; /* min: 0, max: 1 */
    uint8_t order_of_ch0; /* min: 0, max: 3 */
    uint8_t order_of_ch1; /* min: 0, max: 3 */
    uint8_t order_of_ch2; /* min: 0, max: 3 */
    uint8_t append_zeros; /* min: 0, max: 7 */
} imx9x_isp_packetizer_pack_ctrl_cfg_t;

typedef struct {
    uint8_t ch0_output_bpp; /* min: 0, max: 6 */
    uint8_t ch0_right_shift_alignment; /* min: 0, max: 7 */
    uint8_t ch0_left_shift_alignment; /* min: 0, max: 7 */
    uint8_t ch12_output_bpp; /* min: 0, max: 6 */
    uint8_t ch12_right_shift_alignment; /* min: 0, max: 7 */
    uint8_t ch12_left_shift_alignment; /* min: 0, max: 7 */
    uint8_t ch12_subsample; /* min: 0, max: 2 */
    imx9x_isp_packetizer_pack_ctrl_cfg_t pack_ctrl;
} imx9x_isp_packetizer_cfg_t;

#endif /* PACKETIZER_H_ */
