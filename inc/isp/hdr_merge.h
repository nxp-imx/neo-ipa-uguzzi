/*******************************************************************************
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef HDR_MERGE_H_
#define HDR_MERGE_H_

#include <stdint.h>

/** Values from enum correspond to the values of leveled_dcg_bpp, leveled_vs_bpp and output_bpp struct members */
typedef enum {
    HDR_MERGE_BPP_12_IN_16,
    HDR_MERGE_BPP_14_IN_16,
    HDR_MERGE_BPP_16_IN_16,
    HDR_MERGE_BPP_20_IN_20
} imx9x_isp_hdr_merge_bpp_t;

/** Values from enum correspond to the values of blend_mode struct member */
typedef enum {
    HDR_MERGE_BLEND_MODE_1X1,
    HDR_MERGE_BLEND_MODE_3X3
} imx9x_isp_hdr_merge_blend_mode_t;

typedef struct {
    uint8_t enable; /* min: 0, max: 1 */
    uint8_t enable_motion_artifact_fixing; /* min: 0, max: 1 */
    uint8_t blend_mode; /* min: 0, max: 1 */
    /* The bpp of the image from the DCG path after applying the
     * leveling settings. Use values from imx9x_isp_hdr_merge_bpp_t enum.
     */
    uint8_t leveled_dcg_bpp; /* min: 0, max: 3 */
    /* The bpp of the image from the VS path after applying the
     * leveling settings. Use values from imx9x_isp_hdr_merge_bpp_t enum.
     */
    uint8_t leveled_vs_bpp; /* min: 0, max: 3 */
    /* The bpp of the merged image. Use values from imx9x_isp_hdr_merge_bpp_t enum */
    uint8_t output_bpp; /* min: 0, max: 3 */
} imx9x_isp_hdr_merge_ctrl_cfg_t;

typedef struct {
    imx9x_isp_hdr_merge_ctrl_cfg_t ctrl;
    uint16_t leveling_offset_dcg; /* min: 0, max: 65535 */
    uint16_t leveling_offset_vs; /* min: 0, max: 65535 */
    uint16_t leveling_scale_dcg; /* min: 0, max: 65535 */
    uint16_t leveling_scale_vs; /* min: 0, max: 65535 */
    uint8_t leveling_shift_dcg; /* min: 0, max: 31 */
    uint8_t leveling_shift_vs; /* min: 0, max: 31 */
    uint16_t luma_threshold; /* min: 0, max: 65535 */
    uint16_t luma_scale; /* min: 0, max: 65535 */
    uint8_t luma_scale_shift; /* min: 0, max: 31 */
    uint8_t luma_threshold_shift; /* min: 0, max: 31 */
    uint8_t downscale_dcg; /* min: 0, max: 31 */
    uint8_t downscale_vs; /* min: 0, max: 31 */
    uint8_t upscale_dcg; /* min: 0, max: 10 */
    uint8_t upscale_vs; /* min: 0, max: 10 */
    /* NOTE(mms00430): uint16_t is used to avoid padding! */
    uint16_t output_scale; /* min: 0, max: 31 */
} imx9x_isp_hdr_merge_cfg_t;

#endif /* HDR_MERGE_H_ */
