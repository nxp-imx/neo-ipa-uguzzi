/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef EE_H_
#define EE_H_

#include <stdint.h>

typedef enum {
    EE_DEBUG_OFF,
    EE_DEBUG_TEXTURE,
    EE_DEBUG_WHITE,
    EE_DEBUG_INVALID = INT32_MAX
} imx9x_isp_ee_debug_t;

typedef struct {
    uint32_t enable; /* min: 0, max: 1 */
    imx9x_isp_ee_debug_t output_debug_info; /* min: 0, max: 2 */
    uint32_t coring; /* min: 0, max: 1048575 */
    uint32_t clip; /* min: 0, max: 1048575 */
    uint32_t gain; /* min: 0, max: 255, format: u4.4 */
} imx9x_isp_ee_cfg_t;

#endif /* EE_H_ */
