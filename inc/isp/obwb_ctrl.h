/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef OBWB_CTRL_H_
#define OBWB_CTRL_H_

#include <stdint.h>

/** Values from enum correspond to the values of all struct members, with vs_output_bpp having no OBWB_CTRL_BPP_20_IN_20 */
typedef enum {
    OBWB_CTRL_BPP_12_IN_16,
    OBWB_CTRL_BPP_14_IN_16,
    OBWB_CTRL_BPP_16_IN_16,
    OBWB_CTRL_BPP_20_IN_20
} imx9x_isp_obwb_ctrl_bpp_t;

typedef struct {
    uint8_t dcg_output_bpp; /* min: 0, max: 3 */
    uint8_t vs_output_bpp; /* min: 0, max: 2 */
    uint8_t hdr_output_bpp; /* min: 0, max: 3 */
} imx9x_isp_obwb_ctrl_cfg_t;

#endif /* OBWB_CTRL_H_ */
