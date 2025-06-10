/*******************************************************************************
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef PIPE_CONF_H_
#define PIPE_CONF_H_

#include <stdint.h>

#define PIPE_CONF_CFG_SUBTYPE_ID (3U)

/** Values from enum correspond to the values of input_image_0_bpp struct member */
typedef enum {
    PIPE_CONF_IMG_0_BPP_12 = 0,
    PIPE_CONF_IMG_0_BPP_14 = 1,
    PIPE_CONF_IMG_0_BPP_16 = 2,
    PIPE_CONF_IMG_0_BPP_20 = 3,
    PIPE_CONF_IMG_0_BPP_10 = 4,
    PIPE_CONF_IMG_0_BPP_10_PCK = 5,
    PIPE_CONF_IMG_0_BPP_8 = 6,
} imx9x_isp_pipe_conf_img_0_bpp_t;

/** Values from enum correspond to the values of input_image_1_bpp struct member */
typedef enum {
    PIPE_CONF_IMG_1_BPP_12 = 0,
    PIPE_CONF_IMG_1_BPP_14 = 1,
    PIPE_CONF_IMG_1_BPP_16 = 2,
    PIPE_CONF_IMG_1_BPP_10 = 4,
    PIPE_CONF_IMG_1_BPP_10_PCK = 5,
    PIPE_CONF_IMG_1_BPP_8 = 6,
} imx9x_isp_pipe_conf_img_1_bpp_t;

typedef struct {
    uint8_t  input_image_0_bpp;             /* min: 0, max: 6 */
    uint8_t  input_image_1_bpp;             /* min: 0, max: 6 */
    uint8_t  input_image_0_alignment;       /* min: 0, max: 1 */
    uint8_t  input_image_1_alignment;       /* min: 0, max: 1 */
    uint8_t  line_path_0_pixel_alignment;   /* min: 0, max: 1 */
    uint8_t  line_path_1_pixel_alignment;   /* min: 0, max: 1 */
    uint16_t image_width;                   /* min: 0, max: 65534 */
    uint16_t image_height;                  /* min: 8, max: 65534 */
    uint16_t pixel_preskip;                 /* min: 0, max: 65534 */
    uint16_t pixels_postskip;               /* min: 0, max: 65534 */
} imx9x_isp_pipe_conf_cfg_t;

#endif /* PIPE_CONF_H_ */
