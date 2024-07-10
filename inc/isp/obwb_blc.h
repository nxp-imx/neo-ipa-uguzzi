/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef OBWB_BLC_H_
#define OBWB_BLC_H_

#include <stdint.h>

typedef struct {
    uint16_t r; /* min: 0, max: 65535 */
    uint16_t gr; /* min: 0, max: 65535 */
    uint16_t gb; /* min: 0, max: 65535 */
    uint16_t b; /* min: 0, max: 65535 */
} imx9x_isp_obwb_blc_cfg_t;

#endif /* OBWB_BLC_H_ */
