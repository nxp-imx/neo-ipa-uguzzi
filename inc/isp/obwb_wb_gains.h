/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef OBWB_WB_GAINS_H_
#define OBWB_WB_GAINS_H_

#include <stdint.h>

typedef struct {
    uint16_t r; /* min: 0, max: 65535, format: u8.8 */
    uint16_t gr; /* min: 0, max: 65535, format: u8.8 */
    uint16_t gb; /* min: 0, max: 65535, format: u8.8 */
    uint16_t b; /* min: 0, max: 65535, format: u8.8 */
} imx9x_isp_obwb_wb_gains_cfg_t;

#endif /* OBWB_WB_GAINS_H_ */
