/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef VIGNETTING_CTRL_H_
#define VIGNETTING_CTRL_H_

#include <stdint.h>

typedef struct {
    uint16_t enable; /* min: 0, max: 1 */
    uint16_t blocks_cnt_x; /* min: 0, max: 255 */
    uint16_t blocks_cnt_y; /* min: 0, max: 255 */
    uint16_t block_width; /* min: 0, max: 65535 */
    uint16_t block_height; /* min: 0, max: 65535 */
    uint16_t step_x; /* min: 0, max: 32768, format: u1.15 */
    uint16_t step_y; /* min: 0, max: 32768, format: u1.15 */
} imx9x_isp_vignetting_ctrl_cfg_t;

#endif /* VIGNETTING_CTRL_H_ */
