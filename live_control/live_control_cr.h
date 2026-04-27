/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright 2024-2026 NXP
 *
 * NXP NEO uGuzzi Live Control Custom Region
 */

#pragma once

#include <memory>
#include <stdint.h>
#include <string>

#include <linux/nxp_neoisp.h>

#include "utils/stats.h"

extern "C" {
#include "uguzzi/uguzzi.h"
}

namespace libcamera::ipa::nxpneo {

typedef uint32_t lte_ctrl_type;

struct EmbeddedData {
	const uint8_t *topEmbeddedData;
	uint32_t topEmbeddedDataSize;
	const uint8_t *bottomEmbeddedData;
	uint32_t bottomEmbeddedDataSize;
};

/* \todo: need to adapt according to what is currently supported with libcamera uGuzzi IPA */
enum ImageBufferType {
	IMAGE_BUFFER_DCG = 0x00U, /**< DCG */
	IMAGE_BUFFER_VS = 0x01U, /**< VS */
	IMAGE_BUFFER_C012 = 0x02U, /**< interleaved buffer for all 3 components, e.g. YUV */
	IMAGE_BUFFER_C0 = 0x03U, /**< first component of the semiplanar buffer, e.g. Y */
	IMAGE_BUFFER_C12 = 0x04U, /**< second and third components interleaved of the semiplanar buffer, e.g. UV */
	IMAGE_BUFFER_IR = 0x05U, /**< infrared buffer */
	IMAGE_BUFFER_RGB = 0x06U, /**< ISIM specific RGB output */
	IMAGE_BUFFER_CNT = 0x07U, /**< number of available types */
	IMAGE_BUFFER_INVALID = 0xFFU, /**< invalid buffer type */
};

struct ImageBufferView {
	const void *data;
	uint32_t size;
};

struct ImageBufferViewSet {
	ImageBufferView view[IMAGE_BUFFER_CNT];
};

/** Control flags needed for every region */
struct lte_control_t {
	lte_ctrl_type cflags;
	lte_ctrl_type uflags;
};

/** Custom Region 1 structure*/
struct imx9x_isp_output_t {
	uint32_t BLK_STAT_R[NEO_CTEMP_R_SUM_CNT];
	uint32_t BLK_STAT_G[NEO_CTEMP_G_SUM_CNT];
	uint32_t BLK_STAT_B[NEO_CTEMP_B_SUM_CNT];
	uint16_t BLK_STAT_CNT[NEO_CTEMP_PIX_CNT_CNT];
	uint32_t RGBIR_HISTOGRAM[NEO_RGBIR_HIST_CNT];
	uint32_t HISTOGRAM_STATISTICS[NEO_HIST_STAT_CNT];
	uint32_t DRC_LOCAL_SUM[NEO_DRC_LOCAL_SUM_CNT];
	uint32_t DRC_GLOBAL_HIST_ROI0[NEO_DRC_GLOBAL_HIST_ROI_CNT];
	uint32_t DRC_GLOBAL_HIST_ROI1[NEO_DRC_GLOBAL_HIST_ROI_CNT];
	imx9x_isp_ctemp_gr_vs_gb_stats_t ctemp_gr_vs_gb_stats;
};

/**
 * Image transfer custom region (CR2) image header structure
 */
struct imx9x_image_hdr_t {
	uint32_t image_size;
	ImageBufferType image_source;
	uguzzi_sensor_data_t parsed_embedded_data;
};

class LiveControlCr
{
public:
	LiveControlCr()
		: cr1Ctrl_({}), ispOutputData_({}),
		  cr2Ctrl_({}) {}

	int initLiveControlCr(uint32_t maxImgWidth, uint32_t maxImgHeight);

	void handleLiveControlCmdCr(const uguzzi_sensor_data_t *sensorData,
				    const EmbeddedData *embData,
				    const ImageBufferViewSet *imageBuffs,
				    const NxpNeoStats *stats);

	void deinitLiveControlCr() {}

private:
	int initLiveControlCr1();
	int initLiveControlCr2(uint32_t maxImgWidth, uint32_t maxImgHeight);
	void processCr1(const NxpNeoStats *stats);
	void processCr2(const uguzzi_sensor_data_t *sensorData,
			const EmbeddedData *embData,
			const ImageBufferViewSet *imageBuffs);
	bool isImageAvailable(const imx9x_image_hdr_t *imgHeader,
			      const ImageBufferViewSet *imageBuffs) const;
	inline const std::string getImageBufferTypeName(const ImageBufferType type) const;
	void copyImgData(const uguzzi_sensor_data_t *sensorData,
			 const EmbeddedData *embData,
			 const ImageBufferView *imgBuffer,
			 const ImageBufferType imgBuffType) const;

	/** Custom Region 1 values */
	lte_control_t cr1Ctrl_;
	imx9x_isp_output_t ispOutputData_;

	/** Custom Region 2 values */
	lte_control_t cr2Ctrl_;
	std::unique_ptr<uint8_t[]> imgData_;
};

} /* namespace libcamera::ipa::nxpneo */
