/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef DRC_LOCAL_STRETCH_OFFSET_H_
#define DRC_LOCAL_STRETCH_OFFSET_H_

#include <stdint.h>

typedef struct {
    uint16_t stretch; /* min: 0, max: 65535, format: u8.8 */
    uint16_t offset; /* min: 0, max: 65535 */
} imx9x_isp_drc_local_stretch_offset_cfg_t;

#endif /* DRC_LOCAL_STRETCH_OFFSET_H_ */
