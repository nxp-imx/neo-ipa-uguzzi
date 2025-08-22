/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * isp_settings_converter.cpp
 * Conversion between the uGuzzi structures and the NXP NEOISP UAPI
 *
 * Copyright 2024-2025 NXP
 */

#include "isp_settings_converter.h"

#include <algorithm>

using namespace std;

namespace libcamera::ipa::nxpneo {

static void convertPipeConf(neoisp_pipe_conf_cfg_s *neoispPipeConf,
			    imx9x_isp_pipe_conf_cfg_t *pipeconf)
{
	neoispPipeConf->img_conf_inalign0 =
		pipeconf->input_image_0_alignment;
	neoispPipeConf->img_conf_inalign1 =
		pipeconf->input_image_1_alignment;
	neoispPipeConf->img_conf_lpalign0 =
		pipeconf->line_path_0_pixel_alignment;
	neoispPipeConf->img_conf_lpalign1 =
		pipeconf->line_path_1_pixel_alignment;
}

static void convertHc(neoisp_head_color_cfg_s *neoispHc,
		      imx9x_isp_hc_cfg_t *hc)
{
	neoispHc->ctrl_hoffset = (__u8)hc->horizontal_offset;
	neoispHc->ctrl_voffset = (__u8)hc->vertical_offset;
}

static void convertHdrDecompress0(neoisp_hdr_decompress0_cfg_s *neoispHdrDecompress0,
				  imx9x_isp_hdr_decompress_dcg_cfg_t *decompressDcg)
{
	neoispHdrDecompress0->ctrl_enable = (__u8)decompressDcg->enable;
	neoispHdrDecompress0->knee_point1 = (__u16)decompressDcg->knee_point1;
	neoispHdrDecompress0->knee_point2 = (__u16)decompressDcg->knee_point2;
	neoispHdrDecompress0->knee_point3 = (__u16)decompressDcg->knee_point3;
	neoispHdrDecompress0->knee_point4 = (__u16)decompressDcg->knee_point4;
	neoispHdrDecompress0->knee_offset0 = (__u16)decompressDcg->knee_offset0;
	neoispHdrDecompress0->knee_offset1 = (__u16)decompressDcg->knee_offset1;
	neoispHdrDecompress0->knee_offset2 = (__u16)decompressDcg->knee_offset2;
	neoispHdrDecompress0->knee_offset3 = (__u16)decompressDcg->knee_offset3;
	neoispHdrDecompress0->knee_offset4 = (__u16)decompressDcg->knee_offset4;
	neoispHdrDecompress0->knee_ratio0 = (__u16)decompressDcg->ratio0;
	neoispHdrDecompress0->knee_ratio1 = (__u16)decompressDcg->ratio1;
	neoispHdrDecompress0->knee_ratio2 = (__u16)decompressDcg->ratio2;
	neoispHdrDecompress0->knee_ratio3 = (__u16)decompressDcg->ratio3;
	neoispHdrDecompress0->knee_ratio4 = (__u16)decompressDcg->ratio4;
	neoispHdrDecompress0->knee_npoint0 = (__u32)decompressDcg->knee_npoint0;
	neoispHdrDecompress0->knee_npoint1 = (__u32)decompressDcg->knee_npoint1;
	neoispHdrDecompress0->knee_npoint2 = (__u32)decompressDcg->knee_npoint2;
	neoispHdrDecompress0->knee_npoint3 = (__u32)decompressDcg->knee_npoint3;
	neoispHdrDecompress0->knee_npoint4 = (__u32)decompressDcg->knee_npoint4;
}

static void convertHdrDecompress1(neoisp_hdr_decompress1_cfg_s *neoispHdrDecompress1,
				  imx9x_isp_hdr_decompress_vs_cfg_t *decompressVs)
{
	neoispHdrDecompress1->ctrl_enable = (__u8)decompressVs->enable;
	neoispHdrDecompress1->knee_point1 = (__u16)decompressVs->knee_point1;
	neoispHdrDecompress1->knee_point2 = (__u16)decompressVs->knee_point2;
	neoispHdrDecompress1->knee_point3 = (__u16)decompressVs->knee_point3;
	neoispHdrDecompress1->knee_point4 = (__u16)decompressVs->knee_point4;
	neoispHdrDecompress1->knee_offset0 = (__u16)decompressVs->knee_offset0;
	neoispHdrDecompress1->knee_offset1 = (__u16)decompressVs->knee_offset1;
	neoispHdrDecompress1->knee_offset2 = (__u16)decompressVs->knee_offset2;
	neoispHdrDecompress1->knee_offset3 = (__u16)decompressVs->knee_offset3;
	neoispHdrDecompress1->knee_offset4 = (__u16)decompressVs->knee_offset4;
	neoispHdrDecompress1->knee_ratio0 = (__u16)decompressVs->ratio0;
	neoispHdrDecompress1->knee_ratio1 = (__u16)decompressVs->ratio1;
	neoispHdrDecompress1->knee_ratio2 = (__u16)decompressVs->ratio2;
	neoispHdrDecompress1->knee_ratio3 = (__u16)decompressVs->ratio3;
	neoispHdrDecompress1->knee_ratio4 = (__u16)decompressVs->ratio4;
	neoispHdrDecompress1->knee_npoint0 = (__u16)decompressVs->knee_npoint0;
	neoispHdrDecompress1->knee_npoint1 = (__u16)decompressVs->knee_npoint1;
	neoispHdrDecompress1->knee_npoint2 = (__u16)decompressVs->knee_npoint2;
	neoispHdrDecompress1->knee_npoint3 = (__u16)decompressVs->knee_npoint3;
	neoispHdrDecompress1->knee_npoint4 = (__u16)decompressVs->knee_npoint4;
}

static void convertBNR(neoisp_bnr_cfg_s *neoispBnr, imx9x_isp_bnr_cfg_t *bnr)
{
	neoispBnr->ctrl_enable = (__u8)bnr->enable;
	neoispBnr->ctrl_debug = (__u8)bnr->debug;
	neoispBnr->ctrl_obpp = (__u8)bnr->output_bpp;
	neoispBnr->ctrl_nhood = (__u8)bnr->neighbourhood_pattern;
	neoispBnr->ypeak_peak_outsel = (__u8)bnr->y_config.peak_output_scaling_enable;
	neoispBnr->ypeak_peak_sel = (__u8)bnr->y_config.peak_select;
	neoispBnr->ypeak_peak_low = (__u16)bnr->y_config.peak_lower_scale;
	neoispBnr->ypeak_peak_high = (__u16)bnr->y_config.peak_higher_scale;
	neoispBnr->yedge_th0_edge_th0 = (__u32)bnr->y_config.edge_lower_threshold;
	neoispBnr->yedge_scale_scale = (__u16)bnr->y_config.long_scale;
	neoispBnr->yedge_scale_shift = (__u8)bnr->y_config.long_shift;
	neoispBnr->yedges_th0_edge_th0 =
		(__u32)bnr->y_config.short_edge_lower_threshold;
	neoispBnr->yedges_scale_scale = (__u16)bnr->y_config.short_scale;
	neoispBnr->yedges_scale_shift = (__u8)bnr->y_config.short_shift;
	neoispBnr->yedgea_th0_edge_th0 =
		(__u32)bnr->y_config.alpha_edge_lower_threshold;
	neoispBnr->yedgea_scale_scale = (__u16)bnr->y_config.alpha_scale;
	neoispBnr->yedgea_scale_shift = (__u8)bnr->y_config.alpha_shift;
	neoispBnr->yluma_x_th0_th = (__u32)bnr->y_config.luma_x_threhshold;
	neoispBnr->yluma_y_th_luma_y_th0 = (__u16)bnr->y_config.luma_y_threshold_0;
	neoispBnr->yluma_y_th_luma_y_th1 = (__u16)bnr->y_config.luma_y_threshold_1;
	neoispBnr->yluma_scale_scale = (__u16)bnr->y_config.luma_scale;
	neoispBnr->yluma_scale_shift = (__u8)bnr->y_config.luma_shift;
	neoispBnr->yalpha_gain_gain = (__u16)bnr->y_config.alpha_gain;
	neoispBnr->yalpha_gain_offset = (__u16)bnr->y_config.alpha_offset;
	neoispBnr->cpeak_peak_outsel = (__u8)bnr->c_config.peak_output_scaling_enable;
	neoispBnr->cpeak_peak_sel = (__u8)bnr->c_config.peak_select;
	neoispBnr->cpeak_peak_low = (__u16)bnr->c_config.peak_lower_scale;
	neoispBnr->cpeak_peak_high = (__u16)bnr->c_config.peak_higher_scale;
	neoispBnr->cedge_th0_edge_th0 = (__u32)bnr->c_config.edge_lower_threshold;
	neoispBnr->cedge_scale_scale = (__u16)bnr->c_config.scale;
	neoispBnr->cedge_scale_shift = (__u8)bnr->c_config.shift;
	neoispBnr->cedges_th0_edge_th0 =
		(__u32)bnr->c_config.short_edge_lower_threshold;
	neoispBnr->cedges_scale_scale = (__u16)bnr->c_config.short_scale;
	neoispBnr->cedges_scale_shift = (__u8)bnr->c_config.short_shift;
	neoispBnr->cedgea_th0_edge_th0 =
		(__u32)bnr->c_config.alpha_edge_lower_threshold;
	neoispBnr->cedgea_scale_scale = (__u16)bnr->c_config.alpha_scale;
	neoispBnr->cedgea_scale_shift = (__u8)bnr->c_config.alpha_shift;
	neoispBnr->cluma_x_th0_th = (__u32)bnr->c_config.luma_x_threhshold;
	neoispBnr->cluma_y_th_luma_y_th0 = (__u16)bnr->c_config.luma_y_threshold_0;
	neoispBnr->cluma_y_th_luma_y_th1 = (__u16)bnr->c_config.luma_y_threshold_1;
	neoispBnr->cluma_scale_scale = (__u16)bnr->c_config.luma_scale;
	neoispBnr->cluma_scale_shift = (__u8)bnr->c_config.luma_shift;
	neoispBnr->calpha_gain_gain = (__u16)bnr->c_config.alpha_gain;
	neoispBnr->calpha_gain_offset = (__u16)bnr->c_config.alpha_offset;
	neoispBnr->stretch_gain = (__u16)bnr->output_gain;
}

static void convertVignettingCtrl(neoisp_vignetting_ctrl_cfg_s *neoispVignetting,
				  const imx9x_isp_vignetting_ctrl_cfg_t *vignettingCtrl)
{
	neoispVignetting->ctrl_enable = (__u8)vignettingCtrl->enable;
	neoispVignetting->blk_conf_rows = (__u8)vignettingCtrl->blocks_cnt_y;
	neoispVignetting->blk_conf_cols = (__u8)vignettingCtrl->blocks_cnt_x;
	neoispVignetting->blk_size_ysize = (__u16)vignettingCtrl->block_height;
	neoispVignetting->blk_size_xsize = (__u16)vignettingCtrl->block_width;
	neoispVignetting->blk_stepy_step = (__u16)vignettingCtrl->step_y;
	neoispVignetting->blk_stepx_step = (__u16)vignettingCtrl->step_x;
}

static void convertDemosaic(neoisp_demosaic_cfg_s *neoispDemosaic,
			    imx9x_isp_demosaic_cfg_t *demosaic)
{
	neoispDemosaic->ctrl_fmt = (__u8)demosaic->format;
	neoispDemosaic->activity_ctl_alpha = (__u16)demosaic->alpha;
	neoispDemosaic->activity_ctl_act_ratio = (__u16)demosaic->activity_ratio;
	neoispDemosaic->dynamics_ctl0_strengthg = (__u16)demosaic->green_strength;
	neoispDemosaic->dynamics_ctl0_strengthc = (__u16)demosaic->red_blue_strength;
	neoispDemosaic->dynamics_ctl2_max_impact = (__u16)demosaic->max_impact_factor;
}

static void convertCtemp(neoisp_ctemp_cfg_s *neoispCtemp, imx9x_isp_ctemp_cfg_t *ctemp,
			 imx9x_isp_ctemp_csc_cfg_t *ctempCsc,
			 imx9x_isp_ctemp_gr_vs_gb_cfg_t *ctempGrVsGb)
{
	neoispCtemp->ctrl_enable = (__u8)ctemp->ctrl.enable;
	neoispCtemp->ctrl_cscon = (__u8)ctemp->ctrl.use_csc;
	neoispCtemp->ctrl_ibpp = (__u8)ctemp->ctrl.input_bpp;
	neoispCtemp->luma_th_thl = (__u16)ctemp->ctrl.low_luma_threshold;
	neoispCtemp->luma_th_thh = (__u16)ctemp->ctrl.high_luma_threshold;
	neoispCtemp->roi.xpos = (__u16)ctemp->roi.x;
	neoispCtemp->roi.ypos = (__u16)ctemp->roi.y;
	neoispCtemp->roi.width = (__u16)ctemp->roi.width;
	neoispCtemp->roi.height = (__u16)ctemp->roi.height;
	neoispCtemp->redgain_min = (__u8)ctemp->bbc.red_gain_min;
	neoispCtemp->redgain_max = (__u8)ctemp->bbc.red_gain_max;
	neoispCtemp->bluegain_min = (__u8)ctemp->bbc.blue_gain_min;
	neoispCtemp->bluegain_max = (__u8)ctemp->bbc.blue_gain_max;
	neoispCtemp->point1_blue = (__u8)ctemp->bbc.point1_blue;
	neoispCtemp->point1_red = (__u8)ctemp->bbc.point1_red;
	neoispCtemp->point2_blue = (__u8)ctemp->bbc.point2_blue;
	neoispCtemp->point2_red = (__u8)ctemp->bbc.point2_red;
	neoispCtemp->hoffset_right = (__u8)ctemp->bbc.hoffset_right;
	neoispCtemp->hoffset_left = (__u8)ctemp->bbc.hoffset_left;
	neoispCtemp->voffset_up = (__u8)ctemp->bbc.voffset_up;
	neoispCtemp->voffset_down = (__u8)ctemp->bbc.voffset_down;
	neoispCtemp->point1_slope_slope_l = (__s16)ctemp->bbc.point1_slope_left;
	neoispCtemp->point1_slope_slope_r = (__s16)ctemp->bbc.point1_slope_right;
	neoispCtemp->point2_slope_slope_l = (__s16)ctemp->bbc.point2_slope_left;
	neoispCtemp->point2_slope_slope_r = (__s16)ctemp->bbc.point2_slope_right;
	for (int i = 0; i < NEO_CTEMP_COLOR_ROIS_CNT; i++) {
		neoispCtemp->color_rois[i].pos_roverg_low =
			ctemp->color_rois.rois[i].red_over_green_low;
		neoispCtemp->color_rois[i].pos_roverg_high =
			ctemp->color_rois.rois[i].red_over_green_high;
		neoispCtemp->color_rois[i].pos_boverg_low =
			ctemp->color_rois.rois[i].blue_over_green_low;
		neoispCtemp->color_rois[i].pos_boverg_high =
			ctemp->color_rois.rois[i].blue_over_green_high;
	}
	neoispCtemp->stat_blk_size0_xsize = (__u16)ctemp->blocks_stats.paxels_width;
	neoispCtemp->stat_blk_size0_ysize = (__u16)ctemp->blocks_stats.paxels_height;
	for (uint32_t i = 0; i < CTEMP_CSC_MATRIX_SIZE; i++)
		for (uint32_t j = 0; j < CTEMP_CSC_MATRIX_SIZE; j++) {
			neoispCtemp->csc_matrix[i][j] = (__s16)ctempCsc->csc_matrix[i][j];
		}
	for (uint32_t i = 0; i < CTEMP_CSC_OFFSET_VECTOR_SIZE; i++) {
		neoispCtemp->offsets[i] = (__u16)ctempCsc->offsets[i];
	}
	neoispCtemp->gr_avg_in_gr_agv = (__u32)ctempGrVsGb->input_gr_average;
	neoispCtemp->gb_avg_in_gb_agv = (__u32)ctempGrVsGb->input_gb_average;
}

static void convertHdrMerge(neoisp_hdr_merge_cfg_s *neoispHdrMerge,
			    imx9x_isp_hdr_merge_cfg_t *hdrMerge)
{
	neoispHdrMerge->ctrl_enable = (__u8)hdrMerge->ctrl.enable;
	neoispHdrMerge->ctrl_motion_fix_en =
		(__u8)hdrMerge->ctrl.enable_motion_artifact_fixing;
	neoispHdrMerge->ctrl_blend_3x3 = (__u8)hdrMerge->ctrl.blend_mode;
	neoispHdrMerge->ctrl_gain1bpp = (__u8)hdrMerge->ctrl.leveled_dcg_bpp;
	neoispHdrMerge->ctrl_gain0bpp = (__u8)hdrMerge->ctrl.leveled_vs_bpp;
	neoispHdrMerge->ctrl_obpp = (__u8)hdrMerge->ctrl.output_bpp;
	neoispHdrMerge->gain_offset_offset1 = (__u16)hdrMerge->leveling_offset_dcg;
	neoispHdrMerge->gain_offset_offset0 = (__u16)hdrMerge->leveling_offset_vs;
	neoispHdrMerge->gain_scale_scale1 = (__u16)hdrMerge->leveling_scale_dcg;
	neoispHdrMerge->gain_scale_scale0 = (__u16)hdrMerge->leveling_scale_vs;
	neoispHdrMerge->gain_shift_shift1 = (__u8)hdrMerge->leveling_shift_dcg;
	neoispHdrMerge->gain_shift_shift0 = (__u8)hdrMerge->leveling_shift_vs;
	neoispHdrMerge->luma_th_th0 = (__u16)hdrMerge->luma_threshold;
	neoispHdrMerge->luma_scale_scale = (__u16)hdrMerge->luma_scale;
	neoispHdrMerge->luma_scale_shift = (__u8)hdrMerge->luma_scale_shift;
	neoispHdrMerge->luma_scale_thshift = (__u8)hdrMerge->luma_threshold_shift;
	neoispHdrMerge->downscale_imgscale1 = (__u8)hdrMerge->downscale_dcg;
	neoispHdrMerge->downscale_imgscale0 = (__u8)hdrMerge->downscale_vs;
	neoispHdrMerge->upscale_imgscale1 = (__u8)hdrMerge->upscale_dcg;
	neoispHdrMerge->upscale_imgscale0 = (__u8)hdrMerge->upscale_vs;
	neoispHdrMerge->post_scale_scale = (__u8)hdrMerge->output_scale;
}

static void convertEe(neoisp_ee_cfg_s *neoispEe, imx9x_isp_ee_cfg_t *ee)
{
	neoispEe->ctrl_enable = (__u8)ee->enable;
	neoispEe->ctrl_debug = (__u8)ee->output_debug_info;
	neoispEe->maskgain_gain = (__u8)ee->gain;
	neoispEe->coring_coring = (__u32)ee->coring;
	neoispEe->clip_clip = (__u32)ee->clip;
}

static void convertDf(neoisp_df_cfg_s *neoispDf, imx9x_isp_df_cfg_t *df)
{
	neoispDf->ctrl_enable = (__u8)df->enable;
	neoispDf->ctrl_debug = (__u8)df->output_debug_info;
	neoispDf->blend_shift_shift = (__u8)df->blending_right_shift;
	neoispDf->th_scale_scale = (__u32)df->blending_scale;
	neoispDf->blend_th0_th = (__u32)df->blending_threshold_0;
}

static void convertCas(neoisp_cas_cfg_s *neoispCas, imx9x_isp_cas_cfg_t *cas)
{
	neoispCas->gain_shift = (__u8)cas->gain_shift;
	neoispCas->gain_scale = (__u16)cas->gain_scale;
	neoispCas->corr_corr = (__u16)cas->correction;
	neoispCas->offset_offset = (__u16)cas->offset;
}

static void convertGcmInputCsc(neoisp_gcm_cfg_s *neoispGcm,
			       const imx9x_isp_gcm_input_csc_cfg_t *gcmInputCsc)
{
	for (uint32_t row = 0; row < GCM_INPUT_CSC_MATRIX_ROWS; row++)
		for (uint32_t col = 0; col < GCM_INPUT_CSC_MATRIX_COLS; col++)
			neoispGcm->imat_rxcy[row][col] = (__s16)gcmInputCsc->matrix[row][col];

	for (uint32_t i = 0; i < GCM_INPUT_CSC_OFFSETS_SIZE; i++)
		neoispGcm->ioffsets[i] = (__s16)gcmInputCsc->offsets[i];
}

static void convertGcmGamma(neoisp_gcm_cfg_s *neoispGcm,
			    const imx9x_isp_gcm_gamma_cfg_t *gcmGamma)
{
	neoispGcm->gamma0_gamma0 = (__u16)gcmGamma->gamma_power_ch0;
	neoispGcm->gamma1_gamma1 = (__u16)gcmGamma->gamma_power_ch1;
	neoispGcm->gamma2_gamma2 = (__u16)gcmGamma->gamma_power_ch2;
	neoispGcm->gamma0_offset0 = (__u16)gcmGamma->gamma_offset_ch0;
	neoispGcm->gamma1_offset1 = (__u16)gcmGamma->gamma_offset_ch1;
	neoispGcm->gamma2_offset2 = (__u16)gcmGamma->gamma_offset_ch2;
	neoispGcm->blklvl0_ctrl_gain0 = (__u16)gcmGamma->linear_gain_ch0;
	neoispGcm->blklvl1_ctrl_gain1 = (__u16)gcmGamma->linear_gain_ch1;
	neoispGcm->blklvl2_ctrl_gain2 = (__u16)gcmGamma->linear_gain_ch2;
	neoispGcm->blklvl0_ctrl_offset0 = (__s16)gcmGamma->linear_offset_ch0;
	neoispGcm->blklvl1_ctrl_offset1 = (__s16)gcmGamma->linear_offset_ch1;
	neoispGcm->blklvl2_ctrl_offset2 = (__s16)gcmGamma->linear_offset_ch2;
	neoispGcm->lowth_ctrl01_threshold0 = (__u16)gcmGamma->linear_threshold_ch0;
	neoispGcm->lowth_ctrl01_threshold1 = (__u16)gcmGamma->linear_threshold_ch1;
	neoispGcm->lowth_ctrl2_threshold2 = (__u16)gcmGamma->linear_threshold_ch2;
}

static void convertGcmOutputCsc(neoisp_gcm_cfg_s *neoispGcm,
				const imx9x_isp_gcm_output_csc_cfg_t *gcmOutputCsc)
{
	neoispGcm->mat_confg_sign_confg = (__u8)gcmOutputCsc->sign_config;

	for (uint32_t row = 0; row < GCM_OUTPUT_CSC_MATRIX_ROWS; row++)
		for (uint32_t col = 0; col < GCM_OUTPUT_CSC_MATRIX_COLS; col++)
			neoispGcm->omat_rxcy[row][col] = (__s16)gcmOutputCsc->matrix[row][col];

	for (uint32_t i = 0; i < GCM_OUTPUT_CSC_OFFSETS_SIZE; i++)
		neoispGcm->ooffsets[i] = (__s16)gcmOutputCsc->offsets[i];
}

static void convertNr(neoisp_nr_cfg_s *neoispNr, imx9x_isp_nr_cfg_t *nr)
{
	neoispNr->ctrl_enable = (__u8)nr->enable;
	neoispNr->ctrl_debug = (__u8)nr->output_debug_info;
	neoispNr->blend_scale_gain = (__u8)nr->blending_gain;
	neoispNr->blend_scale_shift = (__u8)nr->blending_right_shift;
	neoispNr->blend_scale_scale = (__u16)nr->blending_scale;
	neoispNr->blend_th0_th = (__u32)nr->blending_threshold_0;
}

static void convertRoi(neoisp_roi_cfg_s *neoispRoi, imx9x_isp_autofocus_roi_cfg_t *roi)
{
	neoispRoi->xpos = (__u16)roi->x;
	neoispRoi->ypos = (__u16)roi->y;
	neoispRoi->width = (__u16)roi->width;
	neoispRoi->height = (__u16)roi->height;
}

static void convertAf(neoisp_af_cfg_s *neoispAf, imx9x_isp_autofocus_cfg_t *autofocus)
{
	neoispAf->fil1_shift_shift = (__u8)autofocus->filter1_shift;
	std::copy(&autofocus->filter1_coefficients[0],
		  &autofocus->filter1_coefficients[0] + NEO_AF_FILTERS_CNT,
		  &neoispAf->fil1_coeffs[0]);
	neoispAf->fil0_shift_shift = (__u8)autofocus->filter0_shift;
	std::copy(&autofocus->filter0_coefficients[0],
		  &autofocus->filter0_coefficients[0] + NEO_AF_FILTERS_CNT,
		  &neoispAf->fil0_coeffs[0]);

	for (int i = 0; i < NEO_AF_ROIS_CNT; i++) {
		convertRoi(&neoispAf->af_roi[i], &autofocus->rois_config[i]);
	}
}

static inline void convertConvmed(neoisp_convmed_cfg_s *neoispConvmed,
				  const imx9x_isp_convmed_cfg_t *convmed)
{
	neoispConvmed->ctrl_flt = (__u8)convmed->filter_type;
}

static void convertIrCompress(neoisp_ir_compress_cfg_s *neoispIrCompress,
			      imx9x_isp_ir_compress_cfg_t *irCompress)
{
	neoispIrCompress->ctrl_enable = (__u8)irCompress->enable;
	neoispIrCompress->ctrl_obpp = (__u8)irCompress->output_bpp;
	neoispIrCompress->knee_point1_kneepoint = (__u32)irCompress->knee_point1;
	neoispIrCompress->knee_point2_kneepoint = (__u32)irCompress->knee_point2;
	neoispIrCompress->knee_point3_kneepoint = (__u32)irCompress->knee_point3;
	neoispIrCompress->knee_point4_kneepoint = (__u32)irCompress->knee_point4;
	neoispIrCompress->knee_offset0_offset = (__u32)irCompress->knee_offset0;
	neoispIrCompress->knee_offset1_offset = (__u32)irCompress->knee_offset1;
	neoispIrCompress->knee_offset2_offset = (__u32)irCompress->knee_offset2;
	neoispIrCompress->knee_offset3_offset = (__u32)irCompress->knee_offset3;
	neoispIrCompress->knee_offset4_offset = (__u32)irCompress->knee_offset4;
	neoispIrCompress->knee_ratio01_ratio0 = (__u16)irCompress->ratio0;
	neoispIrCompress->knee_ratio01_ratio1 = (__u16)irCompress->ratio1;
	neoispIrCompress->knee_ratio23_ratio2 = (__u16)irCompress->ratio2;
	neoispIrCompress->knee_ratio23_ratio3 = (__u16)irCompress->ratio3;
	neoispIrCompress->knee_ratio4_ratio4 = (__u16)irCompress->ratio4;
	neoispIrCompress->knee_npoint0_kneepoint = (__u16)irCompress->knee_npoint0;
	neoispIrCompress->knee_npoint1_kneepoint = (__u16)irCompress->knee_npoint1;
	neoispIrCompress->knee_npoint2_kneepoint = (__u16)irCompress->knee_npoint2;
	neoispIrCompress->knee_npoint3_kneepoint = (__u16)irCompress->knee_npoint3;
	neoispIrCompress->knee_npoint4_kneepoint = (__u16)irCompress->knee_npoint4;
}

static void convertHist(const imx9x_isp_stat_hist_cfg_t *hist,
			neoisp_stat_hist_cfg_s *neoispHist)
{
	neoispHist->hist_scale_scale = (__u32)hist->scale;
	neoispHist->hist_ctrl_offset = (__u16)hist->blc_offset;
	neoispHist->hist_ctrl_channel = (__u8)hist->channel_selection;
	neoispHist->hist_ctrl_pattern = (__u8)hist->neighboring_pattern;
	neoispHist->hist_ctrl_dir_input1_dif = (__u8)hist->binning_method;
	neoispHist->hist_ctrl_lin_input1_log = (__u8)hist->type;
}

static void convertStat(neoisp_stat_cfg_s *neoispStat,
			const imx9x_isp_stat_cfg_t *stat)
{
	neoispStat->roi0.xpos = (__u16)stat->foreground.x;
	neoispStat->roi0.ypos = (__u16)stat->foreground.y;
	neoispStat->roi0.width = (__u16)stat->foreground.width;
	neoispStat->roi0.height = (__u16)stat->foreground.height;
	neoispStat->roi1.xpos = (__u16)stat->background.x;
	neoispStat->roi1.ypos = (__u16)stat->background.y;
	neoispStat->roi1.width = (__u16)stat->background.width;
	neoispStat->roi1.height = (__u16)stat->background.height;

	for (int i = 0; i < NEO_STAT_HIST_CNT; i++) {
		convertHist(&stat->hists[i], &neoispStat->hists[i]);
	}
}

static void convertRgbir(neoisp_rgbir_cfg_s *neoispRgbir, imx9x_isp_rgbir_cfg_t *rgbir,
			 uint32_t longest2ShortestFrameRatio)
{
	neoispRgbir->ctrl_enable = (__u8)rgbir->enable;
	neoispRgbir->ccm0_ccm = (__u16)rgbir->red_correction;
	neoispRgbir->ccm1_ccm = (__u16)rgbir->green_correction;
	neoispRgbir->ccm2_ccm = (__u16)rgbir->blue_correction;
	uint64_t imgMax = longest2ShortestFrameRatio;
	imgMax <<= rgbir->frame_bitdepth;
	uint64_t redThreshold = imgMax;
	redThreshold *= rgbir->crosstalk_threshold_red_prc;
	redThreshold = (redThreshold + ((uint32_t)1 << 22)) >> 23;

	uint64_t greenThreshold = imgMax;
	greenThreshold *= rgbir->crosstalk_threshold_green_prc;
	greenThreshold = (greenThreshold + ((uint32_t)1 << 22)) >> 23;

	uint64_t blueThreshold = imgMax;
	blueThreshold *= rgbir->crosstalk_threshold_blue_prc;
	blueThreshold = (blueThreshold + ((uint32_t)1 << 22)) >> 23;

	neoispRgbir->ccm0_th_threshold =
		(redThreshold < 0xFFFC0U) ? (uint32_t)redThreshold : 0xFFFC0U;
	neoispRgbir->ccm1_th_threshold =
		(greenThreshold < 0xFFFC0U) ? (uint32_t)greenThreshold : 0xFFFC0U;
	neoispRgbir->ccm2_th_threshold =
		(blueThreshold < 0xFFFC0U) ? (uint32_t)blueThreshold : 0xFFFC0U;
}

static inline void convertRgbirStat(const imx9x_isp_rgbir_stat_cfg_t *rgbirStat,
				    neoisp_rgbir_cfg_s *neoispRgbir)
{
	static_assert(NEO_RGBIR_ROI_CNT == 2, "Expected NEO_RGBIR_ROI_CNT to be 2");
	neoispRgbir->roi[0].xpos = rgbirStat->foreground.x;
	neoispRgbir->roi[0].ypos = rgbirStat->foreground.y;
	neoispRgbir->roi[0].width = rgbirStat->foreground.width;
	neoispRgbir->roi[0].height = rgbirStat->foreground.height;
	neoispRgbir->roi[1].xpos = rgbirStat->background.x;
	neoispRgbir->roi[1].ypos = rgbirStat->background.y;
	neoispRgbir->roi[1].width = rgbirStat->background.width;
	neoispRgbir->roi[1].height = rgbirStat->background.height;
	for (int i = 0; i < NEO_RGBIR_STAT_HIST_CNT; i++) {
		convertHist(&rgbirStat->hists[i], &neoispRgbir->hists[i]);
	}
}

static void convertObwbDcg(neoisp_obwb_cfg_s *neoispObwb,
			   imx9x_isp_obwb_ctrl_cfg_t *obwb,
			   imx9x_isp_obwb_blc_cfg_t *dcg,
			   imx9x_isp_obwb_wb_gains_cfg_t *genDcg)
{
	neoispObwb->ctrl_obpp = (__u8)obwb->dcg_output_bpp;
	neoispObwb->r_ctrl_offset = (__u16)dcg->r;
	neoispObwb->r_ctrl_gain = (__u16)genDcg->r;
	neoispObwb->gr_ctrl_offset = (__u16)dcg->gr;
	neoispObwb->gr_ctrl_gain = (__u16)genDcg->gr;
	neoispObwb->gb_ctrl_offset = (__u16)dcg->gb;
	neoispObwb->gb_ctrl_gain = (__u16)genDcg->gb;
	neoispObwb->b_ctrl_offset = (__u16)dcg->b;
	neoispObwb->b_ctrl_gain = (__u16)genDcg->b;
}

static void convertObwbVs(neoisp_obwb_cfg_s *neoispObwb,
			  imx9x_isp_obwb_ctrl_cfg_t *obwb,
			  imx9x_isp_obwb_blc_cfg_t *vs,
			  imx9x_isp_obwb_wb_gains_cfg_t *genVs)
{
	neoispObwb->ctrl_obpp = (__u8)obwb->vs_output_bpp;
	neoispObwb->r_ctrl_offset = (__u16)vs->r;
	neoispObwb->r_ctrl_gain = (__u16)genVs->r;
	neoispObwb->gr_ctrl_offset = (__u16)vs->gr;
	neoispObwb->gr_ctrl_gain = (__u16)genVs->gr;
	neoispObwb->gb_ctrl_offset = (__u16)vs->gb;
	neoispObwb->gb_ctrl_gain = (__u16)genVs->gb;
	neoispObwb->b_ctrl_offset = (__u16)vs->b;
	neoispObwb->b_ctrl_gain = (__u16)genVs->b;
}

static void convertObwbHdr(neoisp_obwb_cfg_s *neoispObwb,
			   imx9x_isp_obwb_ctrl_cfg_t *obwb,
			   imx9x_isp_obwb_blc_cfg_t *hdr,
			   imx9x_isp_obwb_wb_gains_cfg_t *genHdr)
{
	neoispObwb->ctrl_obpp = (__u8)obwb->hdr_output_bpp;
	neoispObwb->r_ctrl_offset = (__u16)hdr->r;
	neoispObwb->r_ctrl_gain = (__u16)genHdr->r;
	neoispObwb->gr_ctrl_offset = (__u16)hdr->gr;
	neoispObwb->gr_ctrl_gain = (__u16)genHdr->gr;
	neoispObwb->gb_ctrl_offset = (__u16)hdr->gb;
	neoispObwb->gb_ctrl_gain = (__u16)genHdr->gb;
	neoispObwb->b_ctrl_offset = (__u16)hdr->b;
	neoispObwb->b_ctrl_gain = (__u16)genHdr->b;
}

static void convertDrc(
	neoisp_dr_comp_cfg_s *neoispDrc,
	imx9x_isp_drc_alpha_blending_cfg_t *drcAlpha,
	imx9x_isp_drc_global_tonemap_ctrl_cfg_t *drcGlobalControl,
	imx9x_isp_drc_local_tonemap_ctrl_cfg_t *drcLocalControl,
	imx9x_isp_drc_global_gain_factor_cfg_t *drcGlobalGain,
	imx9x_isp_drc_local_stretch_offset_cfg_t *drcLocalStretchOffset)
{
	neoispDrc->alpha_alpha = (__u16)drcAlpha->alpha_blending_factor;
	neoispDrc->roi0.xpos = (__u16)drcGlobalControl->hist_roi_0.x;
	neoispDrc->roi0.ypos = (__u16)drcGlobalControl->hist_roi_0.y;
	neoispDrc->roi0.width = (__u16)drcGlobalControl->hist_roi_0.width;
	neoispDrc->roi0.height = (__u16)drcGlobalControl->hist_roi_0.height;
	neoispDrc->roi1.xpos = (__u16)drcGlobalControl->hist_roi_1.x;
	neoispDrc->roi1.ypos = (__u16)drcGlobalControl->hist_roi_1.y;
	neoispDrc->roi1.width = (__u16)drcGlobalControl->hist_roi_1.width;
	neoispDrc->roi1.height = (__u16)drcGlobalControl->hist_roi_1.height;
	neoispDrc->groi_sum_shift_shift0 =
		(__u8)drcGlobalControl->hist_roi_0.right_shift;
	neoispDrc->groi_sum_shift_shift1 =
		(__u8)drcGlobalControl->hist_roi_1.right_shift;
	neoispDrc->lcl_blk_size_xsize = (__u16)drcLocalControl->block_size_x;
	neoispDrc->lcl_blk_size_ysize = (__u16)drcLocalControl->block_size_y;
	neoispDrc->lcl_blk_stepx_step = (__u16)drcLocalControl->step_x;
	neoispDrc->lcl_blk_stepy_step = (__u16)drcLocalControl->step_y;
	neoispDrc->lcl_sum_shift_shift = (__u8)drcLocalControl->sum_right_shift;
	neoispDrc->lcl_stretch_offset = (__u16)drcLocalStretchOffset->offset;
	neoispDrc->lcl_stretch_stretch = (__u16)drcLocalStretchOffset->stretch;
	neoispDrc->gbl_gain_gain = (__u16)drcGlobalGain->gain_factor;
}

static void convertDrcGlobal(neoisp_drc_global_tonemap_mem_params_s *neoispDrcGlobal,
			     imx9x_isp_drc_global_tonemap_lut_cfg_t *drcGlobalLUT)
{
	std::copy(&drcGlobalLUT->global_tonemap_LUT[0],
		  &drcGlobalLUT->global_tonemap_LUT[0] + NEO_DRC_GLOBAL_TONEMAP_SIZE,
		  &neoispDrcGlobal->drc_global_tonemap[0]);
}

static void convertDrcLocal(neoisp_drc_local_tonemap_mem_params_s *neoispDrcLocal,
			    imx9x_isp_drc_local_tonemap_lut_cfg_t *drcLocalLUT)
{
	std::copy(&drcLocalLUT->local_tonemap_LUT[0],
		  &drcLocalLUT->local_tonemap_LUT[0] + NEO_DRC_LOCAL_TONEMAP_SIZE,
		  &neoispDrcLocal->drc_local_tonemap[0]);
}

static void clipValue(int64_t *apValue, int64_t aMin, int64_t aMax)
{
	if ((*apValue) > aMax) {
		(*apValue) = aMax;
	} else {
		if ((*apValue) < aMin) {
			(*apValue) = aMin;
		}
	}
}

static int64_t ccmShift(int64_t lValue)
{
	uint64_t lTmp = (uint64_t)lValue;
	const uint32_t lcCcmShift = 8u;

	lTmp >>= lcCcmShift;
	if (lValue < 0) {
		// for negative values make sure 1s are filled from left
		lTmp = lTmp | 0xFF00000000000000U;
	}

	return (int64_t)lTmp;
}

static int16_t mulRowCol(
	int16_t appCSC[3][3], int16_t appCCM[3][3],
	int16_t lRowIdx, int16_t lColIdx)
{
	int64_t lResult;

	lResult =
		(int64_t)appCSC[lRowIdx][0] * (int64_t)appCCM[0][lColIdx] +
		(int64_t)appCSC[lRowIdx][1] * (int64_t)appCCM[1][lColIdx] +
		(int64_t)appCSC[lRowIdx][2] * (int64_t)appCCM[2][lColIdx];

	lResult = ccmShift(lResult);
	clipValue(&lResult, INT16_MIN, INT16_MAX);

	return (int16_t)lResult;
}

static void convertRgb2yuv(neoisp_rgb2yuv_cfg_s *neoispRgb2yuv,
			   imx9x_isp_rgb2yuv_cfg_t *rgb2yuv,
			   imx9x_isp_ccm_cfg_t *ccm)
{
	neoispRgb2yuv->gain_ctrl_rgain = (__u16)rgb2yuv->red_gain;
	neoispRgb2yuv->gain_ctrl_bgain = (__u16)rgb2yuv->blue_gain;
	std::copy(&rgb2yuv->csc_offsets[0],
		  &rgb2yuv->csc_offsets[0] + NEO_RGB2YUV_MATRIX_SIZE,
		  &neoispRgb2yuv->csc_offsets[0]);
	neoispRgb2yuv->mat_rxcy[0][0] = mulRowCol(rgb2yuv->csc_matrix, ccm->ccm_matrix, 0, 0);
	neoispRgb2yuv->mat_rxcy[0][1] = mulRowCol(rgb2yuv->csc_matrix, ccm->ccm_matrix, 0, 1);
	neoispRgb2yuv->mat_rxcy[0][2] = mulRowCol(rgb2yuv->csc_matrix, ccm->ccm_matrix, 0, 2);
	neoispRgb2yuv->mat_rxcy[1][0] = mulRowCol(rgb2yuv->csc_matrix, ccm->ccm_matrix, 1, 0);
	neoispRgb2yuv->mat_rxcy[1][1] = mulRowCol(rgb2yuv->csc_matrix, ccm->ccm_matrix, 1, 1);
	neoispRgb2yuv->mat_rxcy[1][2] = mulRowCol(rgb2yuv->csc_matrix, ccm->ccm_matrix, 1, 2);
	neoispRgb2yuv->mat_rxcy[2][0] = mulRowCol(rgb2yuv->csc_matrix, ccm->ccm_matrix, 2, 0);
	neoispRgb2yuv->mat_rxcy[2][1] = mulRowCol(rgb2yuv->csc_matrix, ccm->ccm_matrix, 2, 1);
	neoispRgb2yuv->mat_rxcy[2][2] = mulRowCol(rgb2yuv->csc_matrix, ccm->ccm_matrix, 2, 2);
}

static void convertVigTable(neoisp_vignetting_table_mem_params_s *neoispVigTable,
			    const imx9x_isp_vignetting_lut_cfg_t *vignetLut)
{
	for (uint32_t i = 0; i < VIGNETTING_TABLE_SIZE; i++) {
		neoispVigTable->vignetting_table[i] =
			vignetLut->vignetting_table[i];
	}
}

void convertUguzziIspCfg2IspDrvCfg(imx9x_isp_cfg_prms_t *cfgParams,
				   uint32_t longest2ShortestFrameRatio,
				   NxpNeoParams *params)
{
	/* pipeline 1 */
	if (cfgParams->update[PIPE_CONF_CFG]) {
		auto config = params->block<BlockParamsType::PipeConf>();
		convertPipeConf(config.params(), &cfgParams->pipe_conf);
		config.setUpdate(true);
	}
	if (cfgParams->update[HC_CFG]) {
		auto config = params->block<BlockParamsType::HeadColor>();
		convertHc(config.params(), &cfgParams->hc);
		config.setUpdate(true);
	}
	if (cfgParams->update[HDR_DECOMPRESS_DCG_CFG]) {
		auto config = params->block<BlockParamsType::HdrDec0>();
		convertHdrDecompress0(config.params(), &cfgParams->decompress_dcg);
		config.setUpdate(true);
	}
	if (cfgParams->update[HDR_DECOMPRESS_VS_CFG]) {
		auto config = params->block<BlockParamsType::HdrDec1>();
		convertHdrDecompress1(config.params(), &cfgParams->decompress_vs);
		config.setUpdate(true);
	}
	if (cfgParams->update[HDR_MERGE_CFG]) {
		auto config = params->block<BlockParamsType::HdrMerge>();
		convertHdrMerge(config.params(), &cfgParams->hdr_merge);
		config.setUpdate(true);
	}
	if (cfgParams->update[RGBIR_CFG] || cfgParams->update[RGBIR_STAT_CFG]) {
		auto config = params->block<BlockParamsType::RgbIr>();
		convertRgbir(config.params(),
			     &cfgParams->rgbir,
			     longest2ShortestFrameRatio);
		convertRgbirStat(&cfgParams->rgbir_stat, config.params());
		config.setUpdate(true);
	}
	if (cfgParams->update[STAT_CFG]) {
		auto config = params->block<BlockParamsType::Stat>();
		convertStat(config.params(), &cfgParams->stat);
		config.setUpdate(true);
	}
	if (cfgParams->update[IR_COMPRESS_CFG]) {
		auto config = params->block<BlockParamsType::IrComp>();
		convertIrCompress(config.params(), &cfgParams->ir_compress);
		config.setUpdate(true);
	}
	if (cfgParams->update[BNR_CFG]) {
		auto config = params->block<BlockParamsType::Bnr>();
		convertBNR(config.params(), &cfgParams->bnr);
		config.setUpdate(true);
	}
	if (cfgParams->update[VIGNETTING_CTRL_CFG]) {
		auto config = params->block<BlockParamsType::VigCtrl>();
		convertVignettingCtrl(config.params(), &cfgParams->vignetting_ctrl);
		config.setUpdate(true);
	}
	if (cfgParams->update[VIGNETTING_LUT_CFG]) {
		auto config = params->block<BlockParamsType::VigTable>();
		convertVigTable(config.params(), &cfgParams->vignetting_lut);
		config.setUpdate(true);
	}
	if (cfgParams->update[CTEMP_CFG] || cfgParams->update[CTEMP_CSC_CFG] || cfgParams->update[CTEMP_GR_VS_GB_CFG]) {
		auto config = params->block<BlockParamsType::CTemp>();
		convertCtemp(config.params(), &cfgParams->ctemp, &cfgParams->ctemp_csc, &cfgParams->ctemp_gr_vs_gb);
		config.setUpdate(true);
	}
	static_assert(NEO_OBWB_CNT == 3, "Expected NEO_OBWB_CNT to be 3");
	if (cfgParams->update[OBWB_BLC_DCG_CFG] || cfgParams->update[OBWB_CTRL_CFG] || cfgParams->update[OBWB_WB_GAINS_DCG_CFG]) {
		auto config = params->block<BlockParamsType::Obwb0>();
		convertObwbDcg(config.params(), &cfgParams->obwb_ctrl, &cfgParams->obwb_blc_dcg, &cfgParams->obwb_wb_gains_dcg);
		config.setUpdate(true);
	}
	if (cfgParams->update[OBWB_BLC_VS_CFG] || cfgParams->update[OBWB_CTRL_CFG] || cfgParams->update[OBWB_WB_GAINS_VS_CFG]) {
		auto config = params->block<BlockParamsType::Obwb1>();
		convertObwbVs(config.params(), &cfgParams->obwb_ctrl, &cfgParams->obwb_blc_vs, &cfgParams->obwb_wb_gains_vs);
		config.setUpdate(true);
	}
	if (cfgParams->update[OBWB_BLC_HDR_CFG] || cfgParams->update[OBWB_CTRL_CFG] || cfgParams->update[OBWB_WB_GAINS_HDR_CFG]) {
		auto config = params->block<BlockParamsType::Obwb2>();
		convertObwbHdr(config.params(), &cfgParams->obwb_ctrl, &cfgParams->obwb_blc_hdr, &cfgParams->obwb_wb_gains_hdr);
		config.setUpdate(true);
	}
	/* Pipeline 2 */
	if (cfgParams->update[DEMOSAIC_CFG]) {
		auto config = params->block<BlockParamsType::Demosaic>();
		convertDemosaic(config.params(), &cfgParams->demosaic);
		config.setUpdate(true);
	}
	if (cfgParams->update[RGB2YUV_CFG] || cfgParams->update[CCM_CFG]) {
		auto config = params->block<BlockParamsType::Rgb2Yuv>();
		convertRgb2yuv(config.params(), &cfgParams->rgb2yuv, &cfgParams->ccm);
		config.setUpdate(true);
	}
	if (cfgParams->update[DRC_ALPHA_BLENDING_CFG] || cfgParams->update[DRC_GLOBAL_TONEMAP_CTRL_CFG] || cfgParams->update[DRC_LOCAL_TONEMAP_CTRL_CFG] || cfgParams->update[DRC_GLOBAL_GAIN_FACTOR_CFG] || cfgParams->update[DRC_LOCAL_STRETCH_OFFSET_CFG]) {
		auto config = params->block<BlockParamsType::DrComp>();
		convertDrc(config.params(), &cfgParams->drc_alpha_blending, &cfgParams->drc_global_tonemap_ctrl, &cfgParams->drc_local_tonemap_ctrl, &cfgParams->drc_global_gain_factor, &cfgParams->drc_local_stretch_offset);
		config.setUpdate(true);
	}
	if (cfgParams->update[DRC_GLOBAL_TONEMAP_LUT_CFG]) {
		auto config = params->block<BlockParamsType::DrcGlobalTonemap>();
		convertDrcGlobal(config.params(), &cfgParams->drc_global_tonemap_lut);
		config.setUpdate(true);
	}
	if (cfgParams->update[DRC_LOCAL_TONEMAP_LUT_CFG]) {
		auto config = params->block<BlockParamsType::DrcLocalTonemap>();
		convertDrcLocal(config.params(), &cfgParams->drc_local_tonemap_lut);
		config.setUpdate(true);
	}
	/* Denoising Pipeline */
	if (cfgParams->update[NR_CFG]) {
		auto config = params->block<BlockParamsType::Nr>();
		convertNr(config.params(), &cfgParams->nr);
		config.setUpdate(true);
	}
	if (cfgParams->update[AUTOFOCUS_CFG]) {
		auto config = params->block<BlockParamsType::Af>();
		convertAf(config.params(), &cfgParams->autofocus);
		config.setUpdate(true);
	}
	if (cfgParams->update[EE_CFG]) {
		auto config = params->block<BlockParamsType::Ee>();
		convertEe(config.params(), &cfgParams->ee);
		config.setUpdate(true);
	}
	if (cfgParams->update[DF_CFG]) {
		auto config = params->block<BlockParamsType::Df>();
		convertDf(config.params(), &cfgParams->df);
		config.setUpdate(true);
	}
	if (cfgParams->update[CONVMED_CFG]) {
		auto config = params->block<BlockParamsType::Convmed>();
		convertConvmed(config.params(), &cfgParams->convmed);
		config.setUpdate(true);
	}
	if (cfgParams->update[CAS_CFG]) {
		auto config = params->block<BlockParamsType::Cas>();
		convertCas(config.params(), &cfgParams->cas);
		config.setUpdate(true);
	}
	const bool gcmUpdated =
		cfgParams->update[GCM_INPUT_CSC_CFG] != 0 ||
		cfgParams->update[GCM_GAMMA_CFG] != 0 ||
		cfgParams->update[GCM_OUTPUT_CSC_CFG] != 0;
	if (gcmUpdated) {
		auto config = params->block<BlockParamsType::Gcm>();
		convertGcmInputCsc(config.params(), &cfgParams->gcm_input_csc);
		convertGcmGamma(config.params(), &cfgParams->gcm_gamma);
		convertGcmOutputCsc(config.params(), &cfgParams->gcm_output_csc);
		config.setUpdate(true);
	}
}

} /* namespace libcamera::ipa::nxpneo */
