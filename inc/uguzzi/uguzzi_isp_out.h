/* =============================================================================
* Copyright 2006-2025 MM Solutions EAD (MMS)
*
* MMS Proprietary. This software is owned or controlled by MMS and may only be
* used strictly in accordance with the applicable license terms. By expressly
* accepting such terms or by downloading, installing, activating and/or
* otherwise using the software, you are agreeing that you have read, and that
* you agree to comply with and are bound by, such license terms. If you do not
* agree to be bound by the applicable license terms, then you may not retain,
* install, activate or otherwise use the software.
* =========================================================================== */
/**
* @file uguzzi_isp_out.h
*
* @brief ISP related set of defines and types used as input/output from uGuzzi
*/

#ifndef UGUZZI_ISP_OUT_H_
#define UGUZZI_ISP_OUT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 *******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <isp/hc.h>
#include <isp/hdr_decompress_dcg.h>
#include <isp/hdr_decompress_vs.h>
#include <isp/hdr_merge.h>
#include <isp/rgbir.h>
#include <isp/rgbir_stat.h>
#include <isp/ir_compress.h>
#include <isp/stat.h>
#include <isp/obwb_ctrl.h>
#include <isp/obwb_blc.h>
#include <isp/obwb_wb_gains.h>
#include <isp/bnr.h>
#include <isp/vignetting_ctrl.h>
#include <isp/vignetting_lut.h>
#include <isp/ctemp.h>
#include <isp/ctemp_csc.h>
#include <isp/ctemp_gr_vs_gb.h>
#include <isp/demosaic.h>
#include <isp/rgb2yuv.h>
#include <isp/ccm.h>
#include <isp/drc_alpha_blending.h>
#include <isp/drc_global_gain_factor.h>
#include <isp/drc_global_tonemap_ctrl.h>
#include <isp/drc_global_tonemap_lut.h>
#include <isp/drc_local_tonemap_ctrl.h>
#include <isp/drc_local_tonemap_lut.h>
#include <isp/drc_local_stretch_offset.h>
#include <isp/autofocus.h>
#include <isp/nr.h>
#include <isp/df.h>
#include <isp/ee.h>
#include <isp/cas.h>
#include <isp/convmed.h>
#include <isp/gcm_input_csc.h>
#include <isp/gcm_gamma.h>
#include <isp/gcm_output_csc.h>
#include <isp/packetizer.h>
#include <isp/pipe_conf.h>

/*
 *******************************************************************************
 *  Defines
 *******************************************************************************
 */
#define IMX9X_ISP_PARAMS_MAX  (64U)
#define IMX9X_ISP_MAXNUM_PASS (1U)

/* AWB HW Block Statistics */
#define IMX9X_ISP_MAX_GRID_VERTICAL   (8U)
#define IMX9X_ISP_MAX_GRID_HORIZONTAL (8U)

typedef enum {
    HC_CFG,
    HDR_DECOMPRESS_DCG_CFG,
    HDR_DECOMPRESS_VS_CFG,
    HDR_MERGE_CFG,
    RGBIR_CFG,
    RGBIR_STAT_CFG,
    IR_COMPRESS_CFG,
    STAT_CFG,
    OBWB_CTRL_CFG,
    OBWB_BLC_DCG_CFG,
    OBWB_BLC_VS_CFG,
    OBWB_BLC_HDR_CFG,
    OBWB_WB_GAINS_DCG_CFG,
    OBWB_WB_GAINS_VS_CFG,
    OBWB_WB_GAINS_HDR_CFG,
    BNR_CFG,
    VIGNETTING_CTRL_CFG,
    VIGNETTING_LUT_CFG,
    CTEMP_CFG,
    CTEMP_CSC_CFG,
    CTEMP_GR_VS_GB_CFG,
    DEMOSAIC_CFG,
    RGB2YUV_CFG,
    CCM_CFG,
    DRC_ALPHA_BLENDING_CFG,
    DRC_GLOBAL_TONEMAP_CTRL_CFG,
    DRC_GLOBAL_TONEMAP_LUT_CFG,
    DRC_LOCAL_TONEMAP_CTRL_CFG,
    DRC_LOCAL_TONEMAP_LUT_CFG,
    DRC_GLOBAL_GAIN_FACTOR_CFG,
    DRC_LOCAL_STRETCH_OFFSET_CFG,
    AUTOFOCUS_CFG,
    NR_CFG,
    DF_CFG,
    EE_CFG,
    CAS_CFG,
    CONVMED_CFG,
    GCM_INPUT_CSC_CFG,
    GCM_GAMMA_CFG,
    GCM_OUTPUT_CSC_CFG,
    PACKETIZER_CFG,
    PIPE_CONF_CFG
} imx9x_isp_update;

typedef struct {
    uint32_t flag_init[IMX9X_ISP_PARAMS_MAX];
    uint32_t flag_config[IMX9X_ISP_PARAMS_MAX];
    uint32_t flag_frame[IMX9X_ISP_PARAMS_MAX];
} imx9x_isp_update_flags_t;

typedef struct {
    uint32_t channel_id;
    uint32_t update[IMX9X_ISP_PARAMS_MAX];

    /* Pipeline 1 */
    imx9x_isp_hc_cfg_t hc;
    imx9x_isp_hdr_decompress_dcg_cfg_t decompress_dcg;
    imx9x_isp_hdr_decompress_vs_cfg_t decompress_vs;
    imx9x_isp_hdr_merge_cfg_t hdr_merge;
    imx9x_isp_rgbir_cfg_t rgbir;
    imx9x_isp_rgbir_stat_cfg_t rgbir_stat;
    imx9x_isp_ir_compress_cfg_t ir_compress;
    imx9x_isp_stat_cfg_t stat;
    imx9x_isp_obwb_ctrl_cfg_t obwb_ctrl;
    imx9x_isp_obwb_blc_cfg_t obwb_blc_dcg;
    imx9x_isp_obwb_blc_cfg_t obwb_blc_vs;
    imx9x_isp_obwb_blc_cfg_t obwb_blc_hdr;
    imx9x_isp_obwb_wb_gains_cfg_t obwb_wb_gains_dcg;
    imx9x_isp_obwb_wb_gains_cfg_t obwb_wb_gains_vs;
    imx9x_isp_obwb_wb_gains_cfg_t obwb_wb_gains_hdr;
    imx9x_isp_bnr_cfg_t bnr;
    imx9x_isp_vignetting_ctrl_cfg_t vignetting_ctrl;
    imx9x_isp_vignetting_lut_cfg_t vignetting_lut;
    imx9x_isp_ctemp_cfg_t ctemp;
    imx9x_isp_ctemp_csc_cfg_t ctemp_csc;
    imx9x_isp_ctemp_gr_vs_gb_cfg_t ctemp_gr_vs_gb;

    /* Pipeline 2 */
    imx9x_isp_demosaic_cfg_t demosaic;
    imx9x_isp_rgb2yuv_cfg_t rgb2yuv;
    imx9x_isp_ccm_cfg_t ccm;
    imx9x_isp_drc_alpha_blending_cfg_t drc_alpha_blending;
    imx9x_isp_drc_global_tonemap_ctrl_cfg_t drc_global_tonemap_ctrl;
    imx9x_isp_drc_global_tonemap_lut_cfg_t drc_global_tonemap_lut;
    imx9x_isp_drc_local_tonemap_ctrl_cfg_t drc_local_tonemap_ctrl;
    imx9x_isp_drc_local_tonemap_lut_cfg_t drc_local_tonemap_lut;
    imx9x_isp_drc_global_gain_factor_cfg_t drc_global_gain_factor;
    imx9x_isp_drc_local_stretch_offset_cfg_t drc_local_stretch_offset;

    /* Pipeline 3 */
    imx9x_isp_autofocus_cfg_t autofocus;
    imx9x_isp_nr_cfg_t nr;
    imx9x_isp_df_cfg_t df;
    imx9x_isp_ee_cfg_t ee;
    imx9x_isp_cas_cfg_t cas;
    imx9x_isp_convmed_cfg_t convmed;
    imx9x_isp_gcm_input_csc_cfg_t gcm_input_csc;
    imx9x_isp_gcm_gamma_cfg_t gcm_gamma;
    imx9x_isp_gcm_output_csc_cfg_t gcm_output_csc;
    imx9x_isp_packetizer_cfg_t packetizer;
    imx9x_isp_pipe_conf_cfg_t pipe_conf;
} imx9x_isp_cfg_prms_t;

typedef struct {
    imx9x_isp_cfg_prms_t isp_cfg_params[IMX9X_ISP_MAXNUM_PASS];
} vpipe_settings_hw_t;

/* Starting color for the bayer pattern */
typedef enum {
    IMX9X_ISP_BAYER_RGrGbB = 0x0,
    IMX9X_ISP_BAYER_GrRBGb = 0x1,
    IMX9X_ISP_BAYER_GbBRGr = 0x2,
    IMX9X_ISP_BAYER_BGbGrR = 0x3,
    IMX9X_ISP_BAYER_INVALID = INT32_MAX
} imx9x_isp_bayer_t;

typedef enum {
    /* CTemp Block Statistics */
    IMX9X_ISP_AWB_STATS_PAXELS,
    /* CTemp Color ROIs */
    IMX9X_ISP_AWB_STATS_CROIS,
    /* CTemp Black Body Curve */
    IMX9X_ISP_AWB_STATS_BBC,
    /* Invalid statistics type. Used primarily to force 32 bits for the enum */
    IMX9X_ISP_AWB_STATS_INVALID = INT32_MAX
} imx9x_isp_awb_stats_type_t;

/* The grid is always IMX9X_ISP_MAX_GRID_HORIZONTAL*IMX9X_ISP_MAX_GRID_VERTICAL
 * and is fixed in the hardware */
typedef struct {
    /* array with number of pixels accumulated for each block/paxel. Plain 16 bit counters */
    const uint16_t *pixel_counts;
    /* array with the sum of red pixels for each block/paxel.
     * Each sum is a pseudo floating point number with
     * 28 bit mantissa and 4 bit exponent in the LSBs */
    const uint32_t *red_sums;
    /* array with the sum of green pixels for each block/paxel
     * Each sum is a pseudo floating point number with
     * 28 bit mantissa and 4 bit exponent in the LSBs */
    const uint32_t *green_sums;
    /* array with the sum of blue pixels for each block/paxel
     * Each sum is a pseudo floating point number with
     * 28 bit mantissa and 4 bit exponent in the LSBs */
    const uint32_t *blue_sums;
} imx9x_isp_awb_paxels_data_t;

typedef struct {
    imx9x_isp_awb_stats_type_t type;
    union {
        imx9x_isp_awb_paxels_data_t paxels_data;
        imx9x_isp_ctemp_color_rois_output_t crois;
        imx9x_isp_ctemp_bbc_output_t bbc;
    };
} imx9x_isp_awb_statistics_t;

/* Histogram structure */
typedef struct {
    uint32_t *Long;         /* Pointer to histogram long array */
    uint32_t *Short;        /* Pointer to histogram short array */
    uint32_t *VShort;       /* Pointer to histogram very short array */

    uint32_t sum;           /* The sum of all the bins in a single array */
    uint16_t bins;          /* The number of bins in the histogram */
} hat_hist_stat_t;

typedef struct {
    const uint32_t *global_hist_roi0;
    const uint32_t *global_hist_roi1;
    const uint32_t *local_stats;
} imx9x_isp_aglbce_stats_hw_t;

typedef struct {
    imx9x_isp_autofocus_output_t af_rois_stat;
    /* The ROIs used to generate the above stats */
    imx9x_isp_autofocus_roi_cfg_t rois_config[AUTOFOCUS_ROI_CNT];

    /* Pointer to DRC block stats */
    const uint32_t *block_stats;
    /* The DRC grid configuration for the above stats */
    uint16_t block_width;
    uint16_t block_height;
    uint16_t block_cnt_horz;
    uint16_t block_cnt_vert;
} imx9x_isp_af_stats_t;

typedef imx9x_isp_bayer_t uguzzi_bayer_pattern_t;
typedef imx9x_isp_aglbce_stats_hw_t ae_statistics_data_hw_t;
typedef hat_hist_stat_t ae_histogram_data_hw_t;
typedef imx9x_isp_awb_statistics_t awb_statistics_data_hw_t;
typedef imx9x_isp_af_stats_t af_statistics_data_hw_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UGUZZI_ISP_OUT_H_ */
