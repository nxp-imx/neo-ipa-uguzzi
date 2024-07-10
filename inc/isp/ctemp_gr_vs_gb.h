/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef CTEMP_GR_VS_GB_H_
#define CTEMP_GR_VS_GB_H_

#include <stdint.h>

typedef struct {
    uint32_t input_gr_average; /* min: 0, max: 1048575 */
    uint32_t input_gb_average; /* min: 0, max: 1048575 */
} imx9x_isp_ctemp_gr_vs_gb_cfg_t;

typedef struct {
    uint32_t pixel_count;
    int64_t gr_sum;
    int64_t gb_sum;
    uint64_t gr_squared_sum;
    uint64_t gb_squared_sum;
    int64_t gr_times_gb_sum;
} imx9x_isp_ctemp_gr_vs_gb_stats_t;

#endif /* CTEMP_GR_VS_GB_H_ */
