/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright 2024 NXP
 * live_control_cr.cpp - NXP NEO uGuzzi Live Control Custom Region
 */

#include "live_control_cr.h"

#include <map>
#include <string.h>

#include <libcamera/base/log.h>

extern "C" {
#include "uguzzi_connect/uguzzi_connect.h"
#include "uguzzi_connect/uguzzi_commands.h"
#include "uguzzi_connect/tuning_tool_commands.h"
#include "uguzzi_connect/live_tuning.h"
}

/* This is indicating different status regarding a TT action */
#define LTEU_DO_NOTHING (0U)
#define LTEU_READ_OUT (1U)
#define LTEU_READ_ONCE (2U)

#define LTEC_DO_NOTHING (0u)
#define LTEC_OVERWRITE_OUT (1u)
#define UGUZZI_LTE_CR2_MAX_BYTES_PER_CHANNEL (4u)

using namespace std;

namespace libcamera::ipa::nxpneo {

LOG_DEFINE_CATEGORY(NxpNeoUguzziLiveCr)

static const std::map<const ImageBufferType, std::string> imgBufTypeStrMap = {
	{ IMAGE_BUFFER_DCG, "DCG" },
	{ IMAGE_BUFFER_VS, "VS" },
	{ IMAGE_BUFFER_C012, "C012" },
	{ IMAGE_BUFFER_C0, "C0" },
	{ IMAGE_BUFFER_C12, "C12" },
	{ IMAGE_BUFFER_IR, "IR" },
	{ IMAGE_BUFFER_RGB, "RGB" },
};

inline const std::string LiveControlCr::getImageBufferTypeName(const ImageBufferType type) const
{
	auto it = imgBufTypeStrMap.find(type);
	if (it != imgBufTypeStrMap.end())
		return it->second;
	else
		return "Unknown";
}

/*
 * Initialization of the Live Tuning Custom Region 1 with ISP statistics buffer.
 */
int LiveControlCr::initLiveControlCr1()
{
	/* setup Custom Region 1 data */
	con_status_t status = lt_set_region_params(CAMERA_IDX_0,
						   LT_CUSTOM_REGION_1,
						   &ispOutputData_,
						   (uint32_t)sizeof(ispOutputData_),
						   ACCESS_BYTE);
	if (status != CONNECT_SUCCESS) {
		LOG(NxpNeoUguzziLiveCr, Error)
			<< "Failed to setup Custom Region 1 data";
		return -EINVAL;
	}

	/* setup Custom Region 1 control data */
	status = lt_set_region_params(CAMERA_IDX_0,
				      LT_CUSTOM_REGION_1_CONTROL,
				      &cr1Ctrl_,
				      (uint32_t)sizeof(cr1Ctrl_),
				      ACCESS_BYTE);
	if (status != CONNECT_SUCCESS) {
		LOG(NxpNeoUguzziLiveCr, Error)
			<< "Failed to setup Custom Region 1 control data";
		return -EINVAL;
	}

	return 0;
}

/*
 * Initialization of the Live Tuning Custom Region 2 with image data buffer.
 */
int LiveControlCr::initLiveControlCr2(uint32_t maxImgWidth, uint32_t maxImgHeight)
{
	if (!maxImgWidth || !maxImgHeight)
		return -EINVAL;

	const uint32_t maxImgSizeBytes =
		maxImgWidth * maxImgHeight * UGUZZI_LTE_CR2_MAX_BYTES_PER_CHANNEL;
	uint32_t maxSize = maxImgSizeBytes + (uint32_t)sizeof(imx9x_image_hdr_t);

	imgData_ = std::make_unique<uint8_t[]>(maxSize);

	/* setup Custom Region 2 data */
	con_status_t status = lt_set_region_params(CAMERA_IDX_0,
						   LT_CUSTOM_REGION_2,
						   imgData_.get(),
						   maxSize,
						   ACCESS_BYTE);
	if (CONNECT_SUCCESS != status) {
		LOG(NxpNeoUguzziLiveCr, Error)
			<< "Failed to setup Custom Region 2 data";
		return -EINVAL;
	}

	/* setup Custom Region 2 control data */
	status = lt_set_region_params(CAMERA_IDX_0,
				      LT_CUSTOM_REGION_2_CONTROL,
				      &cr2Ctrl_,
				      (uint32_t)sizeof(cr2Ctrl_),
				      ACCESS_BYTE);
	if (CONNECT_SUCCESS != status) {
		LOG(NxpNeoUguzziLiveCr, Error)
			<< "Failed to setup Custom Region 2 control data";
		return -EINVAL;
	}

	return 0;
}

int LiveControlCr::initLiveControlCr(uint32_t maxImgWidth, uint32_t maxImgHeight)
{
	int ret = initLiveControlCr1();
	if (ret)
		return ret;
	ret = initLiveControlCr2(maxImgWidth, maxImgHeight);

	return ret;
}

bool LiveControlCr::isImageAvailable(const imx9x_image_hdr_t *imgHeader,
				     const ImageBufferViewSet *imageBuffs) const
{
	bool available = true;

	ASSERT(imgHeader);
	ASSERT(imageBuffs);

	if (imgHeader->image_source >= IMAGE_BUFFER_CNT) {
		LOG(NxpNeoUguzziLiveCr, Warning)
			<< "Image source " << imgHeader->image_source
			<< " is out of range [" << IMAGE_BUFFER_DCG
			<< ", " << IMAGE_BUFFER_CNT << "]";
		return false;
	}
	const ImageBufferView *imgBuffView =
		&imageBuffs->view[imgHeader->image_source];
	if (!imgBuffView->data) {
		available = false;
		LOG(NxpNeoUguzziLiveCr, Warning)
			<< "No image data is available for buffer type "
			<< getImageBufferTypeName(imgHeader->image_source);
	}

	return available;
}

/*
 * This functions copies image content into the image data buffer.
 *
 * The image content consists of:
 * - image header of type imx9x_image_hdr_t (image size, image source, parsed embedded data)
 * - topEmbeddedData of size topEmbeddedDataSize
 * - image data from image buffer
 * - bottomEmbeddedData of size bottomEmbeddedDataSize
 */
/* \todo: use a struct to encapsulate the different image data type */
void LiveControlCr::copyImgData(const uguzzi_sensor_data_t *sensorData,
				const EmbeddedData *embData,
				const ImageBufferView *imgBuffer,
				const ImageBufferType imgBuffType) const
{
	ASSERT(sensorData);
	ASSERT(embData);
	ASSERT(imgBuffer);

	imx9x_image_hdr_t imgHeader = {};
	const bool isRaw = (imgBuffType == IMAGE_BUFFER_DCG) ||
			   (imgBuffType == IMAGE_BUFFER_VS);

	/** Fill the imx9x_image_hdr_t structure */
	imgHeader.image_size = imgBuffer->size;
	imgHeader.image_source = imgBuffType;

	if (isRaw) {
		imgHeader.image_size += embData->topEmbeddedDataSize +
					embData->bottomEmbeddedDataSize;
	}

	/* Copy the parsed embedded data to the image header */
	memcpy(&imgHeader.parsed_embedded_data,
	       sensorData,
	       sizeof(uguzzi_sensor_data_t));

	/* Copy the image header structure to the imgData_ buffer */
	memcpy(imgData_.get(), &imgHeader, sizeof(imx9x_image_hdr_t));
	uint32_t offset = (uint32_t)sizeof(imx9x_image_hdr_t);

	/*
	 * Copy top embedded lines to the imgData_ buffer
	 * in case the source is a RAW image
	 */
	if (isRaw && embData->topEmbeddedDataSize) {
		memcpy(imgData_.get() + offset,
		       embData->topEmbeddedData,
		       embData->topEmbeddedDataSize);
		offset += embData->topEmbeddedDataSize;
	}

	/* Copy the image data to the imgData_ buffer */
	memcpy(imgData_.get() + offset,
	       imgBuffer->data,
	       imgBuffer->size);
	offset += imgBuffer->size;

	/*
	 * Copy bottom embedded lines to the imgData_ buffer
	 * in case the source is a RAW image
	 */
	if (isRaw && embData->bottomEmbeddedDataSize) {
		memcpy(imgData_.get() + offset,
		       embData->bottomEmbeddedData,
		       embData->bottomEmbeddedDataSize);
		offset += embData->bottomEmbeddedDataSize;
	}

	return;
}

/*
 * This function processes any user action linked with CR1.
 * At a valid detected action, the ISP statistics are copied into the CR1 memory region.
 */
void LiveControlCr::processCr1(const NxpNeoStats *stats)
{
	ASSERT(stats);

	if (cr1Ctrl_.uflags == LTEU_DO_NOTHING)
		return;

	auto ctempMemStats = stats->block<BlockStatsType::MCTemp>();
	memcpy(ispOutputData_.BLK_STAT_R,
	       ctempMemStats->ctemp_r_sum,
	       sizeof(ctempMemStats->ctemp_r_sum));
	memcpy(ispOutputData_.BLK_STAT_G,
	       ctempMemStats->ctemp_g_sum,
	       sizeof(ctempMemStats->ctemp_g_sum));
	memcpy(ispOutputData_.BLK_STAT_B,
	       ctempMemStats->ctemp_b_sum,
	       sizeof(ctempMemStats->ctemp_b_sum));
	memcpy(ispOutputData_.BLK_STAT_CNT,
	       ctempMemStats->ctemp_pix_cnt,
	       sizeof(ctempMemStats->ctemp_pix_cnt));

	auto rgbirMemStats = stats->block<BlockStatsType::MRgbIr>();
	memcpy(ispOutputData_.RGBIR_HISTOGRAM,
	       rgbirMemStats->rgbir_hist,
	       sizeof(rgbirMemStats->rgbir_hist));

	auto histMemStats = stats->block<BlockStatsType::MHist>();
	memcpy(ispOutputData_.HISTOGRAM_STATISTICS,
	       histMemStats->hist_stat,
	       sizeof(histMemStats->hist_stat));

	auto drcMemStats = stats->block<BlockStatsType::MDrc>();
	memcpy(ispOutputData_.DRC_LOCAL_SUM,
	       drcMemStats->drc_local_sum,
	       sizeof(drcMemStats->drc_local_sum));
	memcpy(ispOutputData_.DRC_GLOBAL_HIST_ROI0,
	       drcMemStats->drc_global_hist_roi0,
	       sizeof(drcMemStats->drc_global_hist_roi0));
	memcpy(ispOutputData_.DRC_GLOBAL_HIST_ROI1,
	       drcMemStats->drc_global_hist_roi1,
	       sizeof(drcMemStats->drc_global_hist_roi1));

	auto ctempRegStats = stats->block<BlockStatsType::RCTemp>();
	ispOutputData_.ctemp_gr_vs_gb_stats.pixel_count = ctempRegStats->gr_gb_cnt_cnt;
	ispOutputData_.ctemp_gr_vs_gb_stats.gr_sum = ctempRegStats->gr_sum_sum;
	ispOutputData_.ctemp_gr_vs_gb_stats.gb_sum = ctempRegStats->gb_sum_sum;
	ispOutputData_.ctemp_gr_vs_gb_stats.gr_squared_sum = ctempRegStats->gr2_sum_sum;
	ispOutputData_.ctemp_gr_vs_gb_stats.gb_squared_sum = ctempRegStats->gb2_sum_sum;
	ispOutputData_.ctemp_gr_vs_gb_stats.gr_times_gb_sum = ctempRegStats->grgb_sum_sum;

	if (cr1Ctrl_.uflags == LTEU_READ_ONCE)
		cr1Ctrl_.uflags = LTEU_DO_NOTHING;
}

/*
 * This function processes any user action linked with CR2.
 * At a valid detected action, the image data are copied into the CR2 memory region.
 */
void LiveControlCr::processCr2(const uguzzi_sensor_data_t *sensorData,
			       const EmbeddedData *embData,
			       const ImageBufferViewSet *imageBuffs)
{
	if (cr2Ctrl_.uflags == LTEU_DO_NOTHING)
		return;

	imx9x_image_hdr_t *imgHeader = reinterpret_cast<imx9x_image_hdr_t *>(imgData_.get());

	/*
	 * It is expected that the Tuning Tool has written to the imgData_
	 * buffer which image source index should be read.
	 * This is coming from the "Get Image" action from "Image Dataset"
	 */
	const ImageBufferType imgBuffType = imgHeader->image_source;

	if (!isImageAvailable(imgHeader, imageBuffs)) {
		/* Signal to the Tuning Tool that the buffer type is not available */
		imgHeader->image_source = IMAGE_BUFFER_INVALID;
		imgHeader->image_size = 0U;
		LOG(NxpNeoUguzziLiveCr, Warning)
			<< "Requested image buffer type "
			<< getImageBufferTypeName(imgBuffType)
			<< " is not available";
	} else {
		ASSERT(imgBuffType < IMAGE_BUFFER_CNT);
		copyImgData(sensorData, embData,
			    &imageBuffs->view[imgBuffType], imgBuffType);
	}

	if (cr2Ctrl_.uflags == LTEU_READ_ONCE)
		cr2Ctrl_.uflags = LTEU_DO_NOTHING;
	return;
}

void LiveControlCr::handleLiveControlCmdCr(const uguzzi_sensor_data_t *sensorData,
					   const EmbeddedData *embData,
					   const ImageBufferViewSet *imageBuffs,
					   const NxpNeoStats *ispStats)
{
	processCr1(ispStats);
	processCr2(sensorData, embData, imageBuffs);
}

} /* namespace libcamera::ipa::nxpneo */
