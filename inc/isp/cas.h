/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef CAS_H_
#define CAS_H_

#include <stdint.h>

typedef struct {
    uint16_t gain_scale; /* min: 0, max: 65535 */
    uint16_t gain_shift; /* min: 0, max: 31 */
    uint16_t correction; /* min: 0, max: 256, format: u1.8 */
    uint16_t offset; /* min: 0, max: 65535 */
} imx9x_isp_cas_cfg_t;

#endif /* CAS_H_ */
