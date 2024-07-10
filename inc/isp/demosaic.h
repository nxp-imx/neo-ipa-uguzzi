/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef DEMOSAIC_H_
#define DEMOSAIC_H_

#include <stdint.h>

typedef enum {
    DEMOSAIC_FORMAT_RGGB,
    DEMOSAIC_FORMAT_RCCC,
    DEMOSAIC_FORMAT_MONOCHROME,
    DEMOSAIC_FORMAT_FORCE_32BITS = INT32_MAX
} imx9x_isp_demosaic_format_t;

/* TODO(mms00430): Discuss with NXP the format and ranges of the fields */
typedef struct {
    imx9x_isp_demosaic_format_t format; /* min: 0, max: 2 */
    uint32_t alpha; /* min: 0, max: 256, format: u1.8 */
    uint32_t activity_ratio; /* min: 256, max: 65535, format: u8.8 */
    uint32_t green_strength; /* min: 0, max: 511, format: u8.8 */
    uint32_t red_blue_strength; /* min: 0, max: 511, format: u8.8 */
    uint32_t max_impact_factor; /* min: 0, max: 256, format: u8.8 */
} imx9x_isp_demosaic_cfg_t;

#endif /* DEMOSAIC_H_ */
