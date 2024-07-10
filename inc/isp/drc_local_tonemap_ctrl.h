/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef DRC_LOCAL_TONEMAP_CTRL_CFG_H_
#define DRC_LOCAL_TONEMAP_CTRL_CFG_H_

#include <stdint.h>

#define DRC_LOCAL_TONEMAP_CTRL_CFG_SUBTYPE_ID (2U)

typedef struct {
    uint16_t block_size_x; /* min: 0, max: 65535 */
    uint16_t block_size_y; /* min: 0, max: 65535 */
    uint16_t step_x; /* min: 0, max: 32768, format: u1.15 */
    uint16_t step_y; /* min: 0, max: 32768, format: u1.15 */
    uint16_t sum_right_shift; /* min: 0, max: 31 */
} imx9x_isp_drc_local_tonemap_ctrl_cfg_t;

#endif /* DRC_LOCAL_TONEMAP_CTRL_CFG_H_ */
