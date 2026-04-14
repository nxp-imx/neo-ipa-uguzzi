/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright 2024-2026 NXP
 *
 * NXP NEO Image Processing Algorithms
 */

#include <sstream>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <linux/nxp_neoisp.h>

#include <libcamera/base/file.h>
#include <libcamera/base/log.h>
#include <libcamera/base/utils.h>

#include <libcamera/control_ids.h>
#include <libcamera/controls.h>
#include <libcamera/formats.h>
#include <libcamera/logging.h>
#include <libcamera/property_ids.h>

#include <libcamera/ipa/ipa_interface.h>
#include <libcamera/ipa/ipa_module_info.h>
#include <libcamera/ipa/nxpneo_ipa_interface.h>

#include "libcamera/internal/mapped_framebuffer.h"

#include "cam_helper/camera_helper.h"
#include "utils/isp_settings_converter.h"
#include "utils/neo_config.h"
#include "utils/params.h"
#include "utils/stats.h"

#include "dtp.h"
#include "version.h"

extern "C" {
#include "uguzzi/uguzzi.h"
#include "uguzzi/uguzzi_cam_info_dtp.h"
#include "uguzzi/uguzzi_config_cmd.h"
#include "uguzzi/uguzzi_ext_algs.h"
#include "uguzzi/uguzzi_isp_out.h"
}

#ifdef USE_LIVE_CONTROL
#include "live_control/live_control.h"
#endif
#ifdef UGUZZI_TESTS_ENABLED
#include "tests/run_tests.h"
#endif

using namespace std;

namespace libcamera {

LOG_DEFINE_CATEGORY(NxpNeoUguzziIPA)

using namespace libcamera::nxp;

namespace ipa::nxpneo {

#define STAT_HIST_BINS (64U)
#define HIST_CHANNELS_CNT_MAX (4U)
#define HIST_H0_ROI1_START_IDX (64U)
#define HIST_H1_ROI1_START_IDX (192U)
#define HIST_H2_ROI1_START_IDX (320U)
#define HIST_H3_ROI1_START_IDX (448U)

/*
 * This boolean is a shared variable between IPA instances when
 * they are running in non isolated mode, where they live
 * in multiple threads from the same process.
 * In that case, the boolean is used to indicate that an IPA
 * instance has already been initialized.
 * It is useful in that context to let a single IPA to initialize because:
 * - only one uGuzzi instance can run in multithread environment
 * - the live control singleton is used by the uguzzi output data
 *   packets callback to redirect to a function of the class.
 */
atomic_flag gblIpaInitialized = ATOMIC_FLAG_INIT;

struct IPAHwSettings {
	uint32_t apiVersion;
	uint32_t hwCapabilities;
	bool lensPresent;
};

struct IPASessionConfiguration {
	struct {
		Size size;
		/* bpp per ISP input */
		std::array<uint32_t, 2> bpps;
	} sensor;

	std::vector<IPACameraContext> activeContexts;
	IPAPipelineMode pipelineMode;
};


struct IPAFrameContext {
	std::map<IPACameraContext, bool> processed;
};

struct IPAContext {
	IPAHwSettings hw;
	IPASessionConfiguration configuration;
	IPAFrameContext frameContext;
};

class IPANxpNeo : public IPANxpNeoInterface
{
public:
	IPANxpNeo();
	~IPANxpNeo();
	int init(const IPASettings &settings, const InitParams &params,
		 ControlInfoMap *ipaControls,
		 SensorConfig *sensorConfig) override;
	int start() override;
	void stop() override;

	int configure(const IPAConfigInfo &ipaConfig,
		      const std::map<IPAStreamType, IPAStream> &streamConfig,
		      ControlInfoMap *ipaControls) override;
	void mapBuffers(const std::vector<IPABuffer> &buffers) override;
	void unmapBuffers(const std::vector<unsigned int> &ids) override;

	void queueRequest(const uint32_t frame,
			  const ControlList &controls) override;
	void computeParams(const uint32_t frame, const IPACameraContext context,
			   const std::map<IPABufferType,
			   uint32_t> &bufferIds) override;
	void processStats(const uint32_t frame, const IPACameraContext context,
			  const std::map<IPABufferType, uint32_t> &bufferIds,
			  const ControlList &sensorControls) override;

private:
	enum uGuzziAeMode {
		harmonized = 0,
		normal,
	};
	int initializeUguzzi(Size outputSize);
	void deinitUguzzi();
	void initUguzziProcessData();
	int configureUguzzi(
		const std::map<IPAStreamType, IPAStream> &streamConfig);
	void setSessionConfiguration(const IPAConfigInfo &ipaConfig);
	int verifySensorToInit();
	int getDTPConfig();
	int checkDTPConfig(const IPACameraSensorInfo &sensorInfo);
	int setUguzziInitialConfig();
	int setUguzziStreamConfig(
		const std::map<IPAStreamType, IPAStream> &streamConfig);
	int configUguzziAeMode();
	uint32_t getTuningId(IPACameraContext context);
	int getUguzziInitialSettings();
	int getCamInfoFromDTP();
	int getWbLocationFromDTP();

	void prepareUguzziSensorData(uint32_t frame, unsigned int channel);

	void convertCtempRegsToAwbBbc(const neoisp_ctemp_reg_stats_s *ctempRegs,
				      imx9x_isp_ctemp_bbc_output_t *awbBbc);
	void convertCtempRegsToAwbCrois(
		const neoisp_ctemp_reg_stats_s *ctempRegs,
		imx9x_isp_ctemp_color_rois_output_t *awbCrois);

	void overrideParams(imx9x_isp_cfg_prms_t *cfgParams,
			    NxpNeoParams *params);

	void prepareUguzziAecHistograms(
		unsigned int channel,
		const neoisp_rgbir_mem_stats_s *rgbirHist,
		const neoisp_hist_mem_stats_s *hist);
	void prepareUguzziAwbStats(unsigned int channel,
				   const neoisp_ctemp_reg_stats_s *ctempRegs,
				   const neoisp_ctemp_mem_stats_s *ctempMems);
	void prepareUguzziAfStats(unsigned int channel,
				  const neoisp_af_reg_stats_s *afRegs,
				  const neoisp_drc_mem_stats_s *drcMems);
	void prepareUguzziStats(unsigned int channel, const NxpNeoStats *stats);

	void updateAwbStatType(unsigned int channel);
	int processUguzzi(unsigned int channel);

	void setInitialControls();
	void setControls(uint32_t frame);
	bool libcameraCfa2UguzziBayerPattern(uint32_t cfa,
					     uguzzi_cam_info_cfa_t *pattern);
	std::string controlListToString(const ControlList *ctrls) const;
	std::string logSensorParams(const uint32_t frame,
				    const ControlList *ctrlsApplied,
				    const ControlList *ctrlsToApply) const;

	void metaDataToSensorData(const ControlList *mdCtrls,
				  uguzzi_sensor_data_t *sensorData) const;

	bool isYuvFormat(const PixelFormat &format) const;
	bool isMonochrome(const PixelFormat &format) const;
#ifdef USE_LIVE_CONTROL
	void processLiveControl(unsigned int channel, const NxpNeoStats *stats);
#endif
	int loadConfigFile(const std::string &filename);

	/* Constant value 1 in UQx.8 and UQx.16 fixed point formats */
	static constexpr unsigned int UQ8_1 = (1 << 8);
	static constexpr unsigned int UQ16_1 = (1 << 16);

	bool uguzziInitialized_{ false };
	bool metaDataValid_{ false };

	/* IPA context */
	struct IPAContext context_;

	uint32_t rawImage0BufferId_;
	uint32_t rawImage1BufferId_;

	std::string sensorModel_;
	std::string sensorEntity_;

	/*
	 * This flag indicates if IPA init has been enabled or skipped.
	 * IPA init can be skipped in the context of non-isolated mode.
	 */
	bool enabled_{ false };
	DTP dtp_;

	const uguzzi_cam_info_dtp_t *camInfoDtp_[UGUZZI_CAMERA_CNT];
	/* Where to apply the WB gains for each camera */
	uguzzi_cam_info_wb_location_t wbLocation_[UGUZZI_CAMERA_CNT];

	/*
	* uguzzi_config() was called and external algos might want to configure
	* themselves to account for changes in static params
	*/
	bool mConfigChanged{ false };

	/* Long, short and vshort histogram buffers for each uGuzzi channel. */
	uint32_t longHistStats_[UGUZZI_CAMERA_CNT][STAT_HIST_BINS];
	uint32_t shortHistStats_[UGUZZI_CAMERA_CNT][STAT_HIST_BINS];
	uint32_t vShortHistStats_[UGUZZI_CAMERA_CNT][STAT_HIST_BINS];

	/* uguzzi_process() inputs */
	uguzzi_sensor_data_pkg_t sensorDataPkg_;
	uguzzi_stats_data_pkg_t statsDataPkg_;
	ae_histogram_data_hw_t aeHistData_[UGUZZI_CAMERA_CNT];
	ae_statistics_data_hw_t agblbceStats_[UGUZZI_CAMERA_CNT];
	awb_statistics_data_hw_t awbStatsData_[UGUZZI_CAMERA_CNT];
	imx9x_isp_awb_stats_type_t awbStatType_[UGUZZI_CAMERA_CNT];
	af_statistics_data_hw_t afStatsData_[UGUZZI_CAMERA_CNT];

	/* uguzzi_process outputs */
	uguzzi_sensor_settings_pkg_t sensorSettingsPkg_;
	uguzzi_sensor_settings_t sensorSettings_[UGUZZI_CAMERA_CNT];
	uguzzi_isp_settings_pkg_t ispSettingsPkg_;
	vpipe_settings_hw_t ispSettings_[UGUZZI_CAMERA_CNT];

	/* Live tuning buffers */
	ImageBufferViewSetPkg imgBuffViewSetPkg_;
	IspStatisticsPkg ispStatPkg_;
	/* \todo Use it when embedded data is not part of the RAW image */
	EmbeddedDataPkg embDataPkg_;

	std::map<unsigned int, MappedFrameBuffer> buffers_;

	/* Interface to the Camera Helper */
	std::unique_ptr<CameraHelper> camHelper_;

	ControlInfoMap sensorControls_;
	ControlInfoMap lensControls_;

	std::optional<int32_t> lensHwPosition_;

	ControlList mdControls_;

	/*
	 * this array maintains the frame id for each camera
	 * and can be shared for testing purposes.
	 */
	std::array<uint32_t, UGUZZI_CAMERA_CNT> camFrames;

	/* IPA config file instance */
	IPAFileConfig config_;
	/* Tuning info from configuration file */
	const TuningInfo *tuningInfo_;
	/* Directory path containing the configuration files for the IPA */
	std::string dataDir_;

	/* Map between the pipeline context and the uGuzzi channel. */
	std::map<const IPACameraContext, unsigned int> uguzziChannelMap_;
};

namespace {

/* Map between the pipeline mode and the cameraHelper stream mode. */
const std::map<const IPAPipelineMode, SensorStreamModes> kSensorStreamModeMap = {
	{ IPAPipelineMode::Standard, SensorStreamStandard },
	{ IPAPipelineMode::HdrMerge, SensorStreamHdr },
	{ IPAPipelineMode::RgbIr, SensorStreamRgbIr },
	{ IPAPipelineMode::RgbIrDual, SensorStreamDualContext },
};

/*
 * For RgbIr dual mode, the tuningId vector is stored as
 * { tuningIdRgb, tuningIdIr }
 */
const std::map<const IPACameraContext, unsigned int> kTuningIdRgbIrMap = {
	{ IPACameraContext::Rgb, 0 },
	{ IPACameraContext::Ir, 1 },
};

/* List of controls handled by the NeoNxp IPA */
const ControlInfoMap::Map nxpneoControls{};

} /* namespace */

IPANxpNeo::IPANxpNeo()
	: camFrames{}
{
}

IPANxpNeo::~IPANxpNeo()
{
#ifdef USE_LIVE_CONTROL
	int ret = 0;
	LiveControl &liveCtrl = LiveControl::getInstance();
	ret = liveCtrl.deinit();
	if (ret)
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to deinitialize Live Control!";
	ret = liveCtrl.closeSocket();
	if (ret)
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to close socket!";
#endif
	deinitUguzzi();
	enabled_ = false;
}

int IPANxpNeo::initializeUguzzi(Size outputSize)
{
	/* Initialize uguzzi instance. */
	uguzzi_init_configuration_t initCfg{};
	uint16_t cameraCnt = 0;

	for (const auto &[context, channel] : uguzziChannelMap_) {
		initCfg.channel_config[channel].sensor_id = getTuningId(context);
		initCfg.channel_config[channel].sensor_mode = tuningInfo_->tuningMode;
		initCfg.channel_config[channel].image_W = outputSize.width;
		initCfg.channel_config[channel].image_H = outputSize.height;
		/* ISP always adjusts the pattern to RGGB on entry */
		initCfg.channel_config[channel].pattern = IMX9X_ISP_BAYER_RGrGbB;
		initCfg.channel_config[channel].isp_active = 1;
		cameraCnt++;
	}
	ASSERT(cameraCnt <= UGUZZI_CAMERA_CNT);

	initCfg.camera_cnt = cameraCnt;
	initCfg.dtp_database = dtp_.data();
	initCfg.dtp_database_size = dtp_.size();

	int err = uguzzi_init(&initCfg);
	if (err) {
		uguzziInitialized_ = false;
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to initialize uGuzzi library";
		return err;
	}

	uguzziInitialized_ = true;

	return 0;
}

void IPANxpNeo::deinitUguzzi()
{
	if (uguzziInitialized_) {
		uguzzi_delete();
		uguzziInitialized_ = false;
	}
}

/**
 * \brief Initialize uguzzi_process() inputs and outputs
 */
void IPANxpNeo::initUguzziProcessData()
{
	for (const auto &[context, channel] : uguzziChannelMap_) {
		/* Initialize input aeHistData_ */
		aeHistData_[channel].Long = longHistStats_[channel];
		aeHistData_[channel].Short = shortHistStats_[channel];
		aeHistData_[channel].VShort = vShortHistStats_[channel];
		aeHistData_[channel].bins = STAT_HIST_BINS;

		/* Initialize input statsDataPkg_ */
		statsDataPkg_.channel[channel].p_ae_stats =
			&agblbceStats_[channel];
		statsDataPkg_.channel[channel].p_ae_hist =
			&aeHistData_[channel];
		statsDataPkg_.channel[channel].p_awb_stats =
			&awbStatsData_[channel];
		statsDataPkg_.channel[channel].p_awb_sw_stats = nullptr;
		statsDataPkg_.channel[channel].p_af_stats =
			&afStatsData_[channel];

		/* Initialize output sensorSettingsPkg_ */
		sensorSettingsPkg_.channel[channel] = &sensorSettings_[channel];

		/* Initialize output ispSettingsPkg_ */
		ispSettingsPkg_.isp_config[channel] = &ispSettings_[channel];
	}
}

int IPANxpNeo::configureUguzzi(
	const std::map<IPAStreamType, IPAStream> &streamConfig)
{
	int ret = 0;

	ret = setUguzziInitialConfig();
	if (ret) {
		LOG(NxpNeoUguzziIPA, Error) << "Failed uGuzzi initial config";
		return ret;
	}
	LOG(NxpNeoUguzziIPA, Debug) << "Successful uGuzzi initial config!";

	ret = setUguzziStreamConfig(streamConfig);
	if (ret) {
		LOG(NxpNeoUguzziIPA, Error) << "Failed uGuzzi stream config";
		return ret;
	}
	LOG(NxpNeoUguzziIPA, Debug) << "Successful uGuzzi stream config!";

	return 0;
}

void IPANxpNeo::setSessionConfiguration(const IPAConfigInfo &ipaConfig)
{
	uint32_t bpp0 = ipaConfig.sensorInfo.bitsPerPixel;
	uint32_t bpp1 = ipaConfig.bitsPerPixelAuxiliary;
	context_.configuration.sensor.bpps = { bpp0, bpp1 };
	context_.configuration.sensor.size = ipaConfig.sensorInfo.outputSize;
	context_.configuration.pipelineMode = ipaConfig.mode;

	/* Initialize active RGB/Ir contexts. */
	context_.configuration.activeContexts =
		context_.configuration.pipelineMode == IPAPipelineMode::RgbIrDual ?
		std::vector<IPACameraContext> { IPACameraContext::Rgb, IPACameraContext::Ir } :
		std::vector<IPACameraContext> { IPACameraContext::Rgb };
}

/**
 * \brief Verify conditions to initialize IPA/sensor
 *
 * The conditions to be fulfilled so that the IPA instance can be started are:
 * - Non-isolated mode: no other IPA instance was previously initialized
 *   in the same process and sensor filtering not defined in the configuration
 *   file.
 * - Sensor filtering: if defined in the configuration file, the actual sensor
 *   entity name should match the definition.
 *
 * \return 0 if IPA/sensor is to be initialized, a negative error code otherwise
 */
int IPANxpNeo::verifySensorToInit()
{
	bool initialize = true;
	const char *disableIsolation = getenv("LIBCAMERA_IPA_DISABLE_ISOLATION");
	bool nonIsolated = (disableIsolation && disableIsolation[0] != '\0');
	bool sensorFilter = false;

	if (config_.sensorFilter().has_value() &&
	    config_.sensorFilter().value() != "") {
		sensorFilter = true;
		LOG(NxpNeoUguzziIPA, Debug)
			<< "Sensor filter enabled for: "
			<< config_.sensorFilter().value();
	}

	if (nonIsolated && !sensorFilter && gblIpaInitialized.test_and_set())
		initialize = false;

	if (sensorFilter && config_.sensorFilter().value() != sensorEntity_)
		initialize = false;

	if (!initialize) {
		LOG(NxpNeoUguzziIPA, Info)
			<< "Sensor " << sensorEntity_
			<< " doesn't verify conditions to be initialized.";
		return -EINVAL;
	}
	return 0;
}

int IPANxpNeo::getDTPConfig()
{
	int ret = 0;

	ret = getCamInfoFromDTP();
	if (ret) {
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to read camera info from DTP";
		return ret;
	}
	LOG(NxpNeoUguzziIPA, Debug) << "Successful read camera info from DTP!";

	ret = getWbLocationFromDTP();
	if (ret) {
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to configure the WB gains location";
		return ret;
	}
	LOG(NxpNeoUguzziIPA, Debug)
		<< "Successful WB gains location configuration!";

	return 0;
}

/**
 * \brief Check the tuning info
 *
 * This function checks if the parameters used for tuning are aligned with the
 * sensor information.
 * It checks the width, height and embedded top lines.
 * The CFA pattern is not checked since the libcamera ColorFilterArrangement
 * definition doesn't cover the RGBIr format.
 *
 * \param[in] sensorInfo The sensor information
 */
int IPANxpNeo::checkDTPConfig(const IPACameraSensorInfo &sensorInfo)
{
	/*
	 * Checking the parameters used for tuning can be performed for
	 * channel 0 only. Indeed the parameters used for tuning are assumed to
	 * be the same among the uguzzi channels.
	 */
	const uint32_t channel = 0;
	const uguzzi_cam_info_cfa_t camInfoPattern =
		static_cast<uguzzi_cam_info_cfa_t>(
			camInfoDtp_[channel]->frame1_cfg.cfa);

	uint32_t sensorTopLines = camHelper_->attributes()->mdParams.topLines;
	/* outputSize from sensorInfo is cropped to remove the embedded lines */
	Size sensorOutputSize =
		{ sensorInfo.outputSize.width,
		  sensorInfo.outputSize.height + sensorTopLines };
	uint32_t dtpTopLines =
		camInfoDtp_[channel]->frame1_cfg.front_emb_ln_cnt;
	Size dtpOutputSize = { camInfoDtp_[channel]->frame1_cfg.width,
			       camInfoDtp_[channel]->frame1_cfg.height };
	const bool sensorConfigDiffers =
		sensorOutputSize != dtpOutputSize ||
		(sensorTopLines && sensorTopLines != dtpTopLines);

	LOG(NxpNeoUguzziIPA, Debug) << "DTP CFA pattern: " << camInfoPattern;
	if (sensorConfigDiffers)
		LOG(NxpNeoUguzziIPA, Warning)
			<< "Sensor frame and DTP frame configuration differs "
			<< "[Size, nb_emb_ln] = ["
			<< sensorOutputSize << ", " << sensorTopLines
			<< "] versus ["
			<< dtpOutputSize << ", " << dtpTopLines << "]";

	return 0;
}

int IPANxpNeo::setUguzziInitialConfig()
{
	int err = 0;
	uguzzi_configuration_t cfg{};

	for (const auto &[context, channel] : uguzziChannelMap_) {
		cfg.channel_id = channel;

		cfg.config_id = CMD_AE_FLICKER_MODE;
		cfg.config_val = UGUZZI_MAINS_FREQ_UNKNOWN;
		err |= uguzzi_config(&cfg);

		cfg.config_id = CMD_AE_LFM_MODE;
		cfg.config_val = UGUZZI_LFM_MODE_OFF;
		err |= uguzzi_config(&cfg);

		/* Disable the uGuzzi AF processing (0: normal, 1: disabled) */
		cfg.config_id = CMD_AF_PROCESSING_MODE;
		cfg.config_val = context_.hw.lensPresent ? 0 : 1;
		err |= uguzzi_config(&cfg);
	}

	/*
	 * Raise the flag even if none of the above configs succeeded.
	 * A config that changes the value of a static parameter might
	 * fail because an uGuzzi algorithm couldn't find a new tuning
	 * matching that new value. But in that case the value of the
	 * static parameter did change and thus external algorithms must
	 * be notified because their tuning might depend on that static
	 * parameter.
	 */
	mConfigChanged = true;

	return err;
}

int IPANxpNeo::setUguzziStreamConfig(
	const std::map<IPAStreamType, IPAStream> &streamConfig)
{
	/**
	 * Switch tuning based on the color format of the main ISP output stream.
	 * The DTP MUST have appropriate tuning for RGB stream at Scene 0
	 * and appropriate tuning for YUV stream at Scene 1 for this to work.
	 * From the GCM standpoint, a monochrome format needs the YUV profile.
	 * For RGBIR sensor, the IR stream is not processed by GCM, hence the
	 * RGB profile should be used.
	 * Also, a raw-stream only configuration can use any of the RGB/YUV
	 * profile since the IPA is not used in that case.
	 */
	bool rgbFormat = false;
	for (const auto &[index, stream] : streamConfig) {
		PixelFormat format(stream.pixelFormat);
		if (!isYuvFormat(format) && !isMonochrome(format))
			rgbFormat = true;
	}

	/* This is performed only for RGB context. */
	uguzzi_configuration_t cfg{};
	cfg.channel_id = uguzziChannelMap_.at(IPACameraContext::Rgb);
	cfg.config_id = CMD_SCENE_MODE;
	cfg.config_val = rgbFormat ? 0 : 1;
	int cfgError = uguzzi_config(&cfg);
	if (cfgError) {
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to configure Color format tuning!";
		return -EINVAL;
	}

	mConfigChanged = true;

	return 0;
}

/**
 * \brief Configure the uGuzzi AE mode
 *
 * This functions sets the normal (non harmonized) uGuzzi AEC mode.
 * /todo: This setting can be adapted using a configuration parameter.
 *
 * \return 0 on success, or a negative error code otherwise
 */
int IPANxpNeo::configUguzziAeMode()
{
	uguzzi_configuration_t cfg{};
	int err = 0;

	for (const auto &[context, channel] : uguzziChannelMap_) {
		cfg.channel_id = channel;
		cfg.config_id = CMD_AE_MODE;
		cfg.config_val = uGuzziAeMode::normal;
		err |= uguzzi_config(&cfg);
	}

	mConfigChanged = true;

	return err;
}

/**
 * \brief Get tuning id based on the camera context type
 *
 * For RgbIr dual mode, the index used to store the tuningId is retrieved
 * with kTuningIdRgbIrMap mapping the index with the camera context type.
 * For other pipeline mode, this index is 0.
 *
 * \param[in] context The camera context type (RGB or Ir)
 */
uint32_t IPANxpNeo::getTuningId(IPACameraContext context)
{
	uint8_t index = 0;
	if (context_.configuration.pipelineMode == IPAPipelineMode::RgbIrDual)
		index = kTuningIdRgbIrMap.at(context);

	return tuningInfo_->tuningId[index];
}

/**
 * \brief Get initial sensor and ISP settings
 *
 * This function gets initial sensor and ISP settings by calling
 * uguzzi_process() with NULL pointers for input sensor data and statistics.
 * However it is required to call first uguzzi_config() with any valid config
 * to make sure that the ON_CONFIG settings will be also reported. For this,
 * the config with the CMD_AE_MODE applies.
 *
 * \return 0 on success, or a negative error code otherwise
 */
int IPANxpNeo::getUguzziInitialSettings()
{
	int err = configUguzziAeMode();

	for (const auto &[context, channel] : uguzziChannelMap_) {
		/* Update uGuzzi structures with the static buffers. */
		sensorSettingsPkg_.channel[channel] = &sensorSettings_[channel];
		ispSettingsPkg_.isp_config[channel] = &ispSettings_[channel];
	}

	err |= uguzzi_process(NULL, NULL,
			      &sensorSettingsPkg_, &ispSettingsPkg_);

	for (const auto &[context, channel] : uguzziChannelMap_)
		updateAwbStatType(channel);

	if (err)
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to get uGuzzi initial settings";

	return err;
}

int IPANxpNeo::getCamInfoFromDTP()
{
	for (const auto &[context, channel] : uguzziChannelMap_) {
		uguzzi_dtp_output_t dtpOutput;
		int dtpError = uguzzi_ext_algs_dtp_query(
			channel,
			UGUZZI_DTP_ID_EXT_ALGS_CAMERA_INFO,
			UGUZZI_CAM_INFO_DTP_SUBTYPE_ID,
			&dtpOutput);
		if (dtpError) {
			LOG(NxpNeoUguzziIPA, Error)
				<< "Failed to read camera info DTP for channel "
				<< channel;
			return dtpError;
		}

		if (dtpOutput.type != UGUZZI_DTP_DATA_TYPE_FIXED) {
			LOG(NxpNeoUguzziIPA, Error)
				<< "Wrong DTP type for channel "
				<< channel
				<< "! Only Fixed type is supported";
			return -EINVAL;
		}

		camInfoDtp_[channel] =
			static_cast<const uguzzi_cam_info_dtp_t *>(
				dtpOutput.x1y1);
	}

	return 0;
}

/* Must be called only after successful call to IPANxpNeo::getCamInfoFromDTP() */
int IPANxpNeo::getWbLocationFromDTP()
{
	for (const auto &[context, channel] : uguzziChannelMap_) {
		switch (camInfoDtp_[channel]->apply_wb_gains_in) {
		case UGUZZI_CAM_INFO_WB_LOCATION_SENSOR:
			LOG(NxpNeoUguzziIPA, Debug)
				<< "WB gains apply in sensor for channel "
				<< channel;
			break;
		case UGUZZI_CAM_INFO_WB_LOCATION_ISP:
			LOG(NxpNeoUguzziIPA, Debug)
				<< "WB gains apply in ISP for channel "
				<< channel;
			break;
		case UGUZZI_CAM_INFO_WB_LOCATION_OTHER:
			LOG(NxpNeoUguzziIPA, Error)
				<< "Unsupported WB gains location for channel "
				<< channel;
			return -EINVAL;
		default:
			LOG(NxpNeoUguzziIPA, Error)
				<< "Unknown WB gains location for channel "
				<< channel;
			return -EINVAL;
		}

		wbLocation_[channel] =
			static_cast<uguzzi_cam_info_wb_location_t>(
				camInfoDtp_[channel]->apply_wb_gains_in);
	}

	return 0;
}

/**
 * The sensor data for each camera must contain the total WB gains applied on
 * the image until the AWB block statistics are made. This includes digital
 * gains applied through OBWB and BNR, and other blocks that can apply per
 * channel gain before the AWB block statistics are generated.
 * The per channel gains applied after AWB block statistics are NOT to be
 * included.
 */
void IPANxpNeo::prepareUguzziSensorData(uint32_t frame, unsigned int channel)
{
	if (!sensorDataPkg_.channel[channel].valid) {
		/* Skip invalid sensor data. */
		return;
	}

	const imx9x_isp_cfg_prms_t *ispCfg =
		&ispSettings_[channel].isp_cfg_params[0];
	uguzzi_awb_gains_t *sensorWbGains =
		&sensorDataPkg_.channel[channel].wb.wb_gains;

	uint64_t redGain = sensorWbGains->red;
	uint64_t greenGain = sensorWbGains->green;
	uint64_t blueGain = sensorWbGains->blue;

	redGain *= ispCfg->obwb_wb_gains_dcg.r;
	redGain *= ispCfg->obwb_wb_gains_hdr.r;
	redGain *= ispCfg->bnr.output_gain;
	redGain >>= 24U;

	greenGain *= (static_cast<uint32_t>(ispCfg->obwb_wb_gains_dcg.gr) +
		      ispCfg->obwb_wb_gains_dcg.gb) / 2U;
	greenGain *= (static_cast<uint32_t>(ispCfg->obwb_wb_gains_hdr.gr) +
		      ispCfg->obwb_wb_gains_hdr.gb) / 2U;
	greenGain *= ispCfg->bnr.output_gain;
	greenGain >>= 24U;

	blueGain *= ispCfg->obwb_wb_gains_dcg.b;
	blueGain *= ispCfg->obwb_wb_gains_hdr.b;
	blueGain *= ispCfg->bnr.output_gain;
	blueGain >>= 24U;

	uint64_t maxGain = (redGain > greenGain) ? redGain : greenGain;
	maxGain = (maxGain > blueGain) ? maxGain : blueGain;
	if (maxGain > UINT16_MAX) {
		/*
		 * Scale down the gains. This is to preserve the color ratios
		 * and as much of the digital gain as possible.
		 * 1.0 = 65536 and round it up to make sure the gains will be
		 * below the max.
		 */
		const uint64_t scale = (maxGain * 65536U + UINT16_MAX) / UINT16_MAX;
		redGain = (redGain * 65536U) / scale;
		greenGain = (greenGain * 65536U) / scale;
		blueGain = (blueGain * 65536U) / scale;

		LOG(NxpNeoUguzziIPA, Warning)
			<< "Too high total WB gains applied for channel "
			<< channel
			<< "Quality of AWB algorithm might be reduced!";
	}

	sensorWbGains->red = static_cast<uint16_t>(redGain);
	sensorWbGains->green = static_cast<uint16_t>(greenGain);
	sensorWbGains->blue = static_cast<uint16_t>(blueGain);

	const bool wbGainsValid = sensorWbGains->red >= 256U &&
				  sensorWbGains->green >= 256U &&
				  sensorWbGains->blue >= 256U;
	sensorDataPkg_.channel[channel].valid = wbGainsValid ? 1U : 0U;

	/* \todo populate frame number from the metadata when available. */
	sensorDataPkg_.frame_num = static_cast<uint64_t>(frame);
}

void IPANxpNeo::convertCtempRegsToAwbBbc(
	const neoisp_ctemp_reg_stats_s *ctempRegs,
	imx9x_isp_ctemp_bbc_output_t *awbBbc)
{
	awbBbc->white_pixel_count = ctempRegs->cnt_white_white;
	awbBbc->red_sum =
		((uint64_t)ctempRegs->sumr_sum_h << 32) | ctempRegs->sumr_sum_l;
	awbBbc->green_sum =
		((uint64_t)ctempRegs->sumg_sum_h << 32) | ctempRegs->sumg_sum_l;
	awbBbc->blue_sum =
		((uint64_t)ctempRegs->sumb_sum_h << 32) | ctempRegs->sumb_sum_l;
	/*
	 * These are sums of u1.7 values so high register will always be zero
	 * for up to 8Mpix sensors.
	 */
	awbBbc->r_over_g_sum = ctempRegs->sumrg_sum_l;
	awbBbc->b_over_g_sum = ctempRegs->sumbg_sum_l;
}

void IPANxpNeo::convertCtempRegsToAwbCrois(
	const neoisp_ctemp_reg_stats_s *ctempRegs,
	imx9x_isp_ctemp_color_rois_output_t *awbCrois)
{
	for (int roi = 0; roi < NEO_CTEMP_REG_STATS_CROIS_CNT; roi++) {
		awbCrois->rois[roi].pixel_count =
			ctempRegs->crois[roi].pixcnt_pixcnt;
		awbCrois->rois[roi].red_sum = ctempRegs->crois[roi].sumred_sum;
		awbCrois->rois[roi].green_sum =
			ctempRegs->crois[roi].sumgreen_sum;
		awbCrois->rois[roi].blue_sum =
			ctempRegs->crois[roi].sumblue_sum;
	}
}

/**
 * \brief Override the ISP params
 *
 * This function overrides some ISP params according to hw configuration
 * or to user configuration defined in configuration file.
 * For now, only the INALIGN from the PIPE_CONF is overriden.
 *
 * \param[in] cfgParams The uGuzzi output params
 * \param[out] params The ISP params to override
 */
void IPANxpNeo::overrideParams(imx9x_isp_cfg_prms_t *cfgParams,
			       NxpNeoParams *params)
{
	if (cfgParams->update[PIPE_CONF_CFG] && config_.overrideInAlign()) {
		uint8_t inAlign =
			(context_.hw.hwCapabilities & NEO_CAP_ALIGNMENT_MSB) ?
			1 : 0;

		auto config = params->block<BlockParamsType::PipeConf>();
		/*
		 * Call to setUpdate(true) is already performed by the uGuzzi
		 * pipeconf configuration.
		 */

		config->img_conf_inalign0 = inAlign;
		config->img_conf_inalign1 = inAlign;
	}
}

void IPANxpNeo::prepareUguzziAecHistograms(
	unsigned int channel,
	const neoisp_rgbir_mem_stats_s *rgbirHist,
	const neoisp_hist_mem_stats_s *hist)
{
	const imx9x_isp_stat_cfg_t &statCfg =
		ispSettings_[channel].isp_cfg_params[0].stat;
	const imx9x_isp_rgbir_stat_cfg_t &rgbirStatCfg =
		ispSettings_[channel].isp_cfg_params[0].rgbir_stat;

	uint32_t statRoiWidth = statCfg.background.width;
	uint32_t statRoiHeight = statCfg.background.height;
	uint32_t rgbirRoiWidth = rgbirStatCfg.background.width;
	uint32_t rgbirRoiHeight = rgbirStatCfg.background.height;

	bool mismatchingRois = false;
	/* Check the ROI configuration of the histograms */
	if (statRoiWidth != rgbirRoiWidth || statRoiHeight != rgbirRoiHeight) {
		/*
		 * Data corruption or misconfiguration.
		 * Duplicate the STAT histograms.
		 */
		LOG(NxpNeoUguzziIPA, Warning)
			<< "Mismatching RGBIR and STAT histograms ROI configuration. "
			<< "Discard RGBIR histograms and use duplicate STAT histograms";
		mismatchingRois = true;
	}

	const uint32_t *hist0;
	const uint32_t *hist1;
	const uint32_t *hist2;
	const uint32_t *hist3;
	const uint32_t *hist4;
	const uint32_t *hist5;
	if (mismatchingRois) {
		/* Duplicate the histograms from STATS module */
		hist0 = &hist->hist_stat[HIST_H0_ROI1_START_IDX];
		hist1 = &hist->hist_stat[HIST_H1_ROI1_START_IDX];
		hist2 = &hist->hist_stat[HIST_H2_ROI1_START_IDX];
		hist3 = &hist->hist_stat[HIST_H0_ROI1_START_IDX];
		hist4 = &hist->hist_stat[HIST_H1_ROI1_START_IDX];
		hist5 = &hist->hist_stat[HIST_H2_ROI1_START_IDX];
	} else {
		/* The histograms are configured correctly, use all of them */
		hist0 = &hist->hist_stat[HIST_H0_ROI1_START_IDX];
		hist1 = &hist->hist_stat[HIST_H1_ROI1_START_IDX];
		hist2 = &hist->hist_stat[HIST_H2_ROI1_START_IDX];
		hist3 = &hist->hist_stat[HIST_H3_ROI1_START_IDX];
		hist4 = &rgbirHist->rgbir_hist[HIST_H0_ROI1_START_IDX];
		hist5 = &rgbirHist->rgbir_hist[HIST_H1_ROI1_START_IDX];
	}

	ae_histogram_data_hw_t &histData = aeHistData_[channel];
	for (uint32_t bin = 0U; bin < STAT_HIST_BINS; bin++) {
		histData.Long[bin] = *(hist0 + bin) + *(hist3 + bin);
		histData.Short[bin] = *(hist1 + bin) + *(hist4 + bin);
		histData.VShort[bin] = *(hist2 + bin) + *(hist5 + bin);
	}

	/*
	 * Maximum combined channels used for each uGuzzi histograms.
	 * Each uGuzzi histogram is a combination of 2 ISP histograms which
	 * can be configured with up to 2 bayer channels.
	 */
	const unsigned int maxUguzziHistChannels = 4;
	histData.sum = (statRoiWidth * statRoiHeight / HIST_CHANNELS_CNT_MAX) *
		       maxUguzziHistChannels;
}

void IPANxpNeo::prepareUguzziAwbStats(unsigned int channel,
				      const neoisp_ctemp_reg_stats_s *ctempRegs,
				      const neoisp_ctemp_mem_stats_s *ctempMems)
{
	const imx9x_isp_awb_stats_type_t statType = awbStatType_[channel];
	awb_statistics_data_hw_t &outAwbData = awbStatsData_[channel];

	switch (statType) {
	case IMX9X_ISP_AWB_STATS_PAXELS:
		outAwbData.type = IMX9X_ISP_AWB_STATS_PAXELS;
		outAwbData.paxels_data.pixel_counts = ctempMems->ctemp_pix_cnt;
		outAwbData.paxels_data.red_sums = ctempMems->ctemp_r_sum;
		outAwbData.paxels_data.green_sums = ctempMems->ctemp_g_sum;
		outAwbData.paxels_data.blue_sums = ctempMems->ctemp_b_sum;
		break;

	case IMX9X_ISP_AWB_STATS_CROIS:
		outAwbData.type = IMX9X_ISP_AWB_STATS_CROIS;
		convertCtempRegsToAwbCrois(ctempRegs, &outAwbData.crois);
		break;

	case IMX9X_ISP_AWB_STATS_BBC:
		outAwbData.type = IMX9X_ISP_AWB_STATS_BBC;
		convertCtempRegsToAwbBbc(ctempRegs, &outAwbData.bbc);
		break;

	default:
		LOG(NxpNeoUguzziIPA, Error)
			<< "Unsupported AWB statistics type " << statType
			<< ". Using Block Statistics instead!";
		outAwbData.type = IMX9X_ISP_AWB_STATS_PAXELS;
		outAwbData.paxels_data.pixel_counts = ctempMems->ctemp_pix_cnt;
		outAwbData.paxels_data.red_sums = ctempMems->ctemp_r_sum;
		outAwbData.paxels_data.green_sums = ctempMems->ctemp_g_sum;
		outAwbData.paxels_data.blue_sums = ctempMems->ctemp_b_sum;
		break;
	}
}

void IPANxpNeo::prepareUguzziAfStats(unsigned int channel,
				     const neoisp_af_reg_stats_s *afRegs,
				     const neoisp_drc_mem_stats_s *drcMems)
{
	af_statistics_data_hw_t &outAfData = afStatsData_[channel];

	/* Populate CDAF statistics. */
	uint32_t *filter0Sums = &outAfData.af_rois_stat.filter0_sums[0];
	uint32_t *filter1Sums = &outAfData.af_rois_stat.filter1_sums[0];
	for (unsigned int i = 0; i < AUTOFOCUS_ROI_CNT; i++) {
		filter0Sums[i] = afRegs->rois[i].sum0;
		filter1Sums[i] = afRegs->rois[i].sum1;
	}

	/* Report AF ROI definitions. */
	const imx9x_isp_autofocus_roi_cfg_t *roisConfig =
		&ispSettings_[channel].isp_cfg_params[0].autofocus.rois_config[0];
	imx9x_isp_autofocus_roi_cfg_t *roisOut = &outAfData.rois_config[0];
	std::copy(roisConfig, roisConfig + AUTOFOCUS_ROI_CNT, roisOut);

	/* Populate DRC local 32x32 grid configuration and statistics. */
	outAfData.block_stats = drcMems->drc_local_sum;
	const imx9x_isp_drc_local_tonemap_ctrl_cfg_t *localDrcConfig =
		&ispSettings_[channel].isp_cfg_params[0].drc_local_tonemap_ctrl;
	outAfData.block_width = localDrcConfig->block_size_x;
	outAfData.block_height = localDrcConfig->block_size_y;
	outAfData.block_cnt_horz = 32;
	outAfData.block_cnt_vert = 32;
}

void IPANxpNeo::prepareUguzziStats(unsigned int channel, const NxpNeoStats *stats)
{
	auto rgbirStats = stats->block<BlockStatsType::MRgbIr>();
	auto histStats = stats->block<BlockStatsType::MHist>();
	prepareUguzziAecHistograms(channel,
				   rgbirStats.stats(),
				   histStats.stats());

	auto ctempRegStats = stats->block<BlockStatsType::RCTemp>();
	auto ctempMemStats = stats->block<BlockStatsType::MCTemp>();
	prepareUguzziAwbStats(channel,
			      ctempRegStats.stats(),
			      ctempMemStats.stats());

	auto drcMemStats = stats->block<BlockStatsType::MDrc>();
	agblbceStats_[channel].global_hist_roi0 =
		drcMemStats->drc_global_hist_roi0;
	agblbceStats_[channel].global_hist_roi1 =
		drcMemStats->drc_global_hist_roi1;
	agblbceStats_[channel].local_stats =
		drcMemStats->drc_local_sum;

	auto afStats = stats->block<BlockStatsType::RAf>();
	prepareUguzziAfStats(channel, afStats.stats(),
			     drcMemStats.stats());

	statsDataPkg_.channel[channel].stats_type = HW_STATS;
	statsDataPkg_.channel[channel].valid = 1;
}

void IPANxpNeo::updateAwbStatType(unsigned int channel)
{
	/*
	 * Since the CTemp configuration can be updated dynamically, the type
	 * of AWB statistic (BBC, CTemp ROIs or Block statistic) should be
	 * stored to provide the proper AWB statistics.
	 */
	const imx9x_isp_cfg_prms_t *ispCfg =
		&ispSettings_[channel].isp_cfg_params[0];

	if (ispCfg->update[CTEMP_CFG]) {
		awbStatType_[channel] = static_cast<imx9x_isp_awb_stats_type_t>(
			ispCfg->ctemp.ctrl.awb_stat_type);
	}
}

int IPANxpNeo::processUguzzi(unsigned int channel)
{
	vpipe_settings_hw_t invalidIspSettings;
	uguzzi_sensor_settings_t invalidSensorSettings;

	/*
	 * For the active channel, update uGuzzi structures with the static
	 * buffers.
	 */
	ispSettingsPkg_.isp_config[channel] = &ispSettings_[channel];
	sensorSettingsPkg_.channel[channel] = &sensorSettings_[channel];

	for (const auto &it : uguzziChannelMap_) {
		if (channel == it.second)
			continue;
		uint8_t nonActiveChannel = it.second;
		sensorDataPkg_.channel[nonActiveChannel].valid = 0;
		statsDataPkg_.channel[nonActiveChannel].valid = 0;
		/*
		 * Provide unused buffers to uGuzzi for the non active channels
		 * since uGuzzi is still updating those buffers, such as the
		 * update flag of the ISP settings and the sensor settings value.
		 * This ensures to preserve the latest valid uGuzzi output for
		 * these non active channels which can be retrieved from the
		 * map of buffers.
		 */
		ispSettingsPkg_.isp_config[nonActiveChannel] =
			&invalidIspSettings;
		sensorSettingsPkg_.channel[nonActiveChannel] =
			&invalidSensorSettings;
	}

	int err = uguzzi_process(&sensorDataPkg_, &statsDataPkg_,
				 &sensorSettingsPkg_, &ispSettingsPkg_);
	if (err)
		return err;

	updateAwbStatType(channel);

	return err;
}

#ifdef USE_LIVE_CONTROL
void IPANxpNeo::processLiveControl(unsigned int channel, const NxpNeoStats *stats)
{
	LiveControl &liveCtrl = LiveControl::getInstance();

	if (buffers_.count(rawImage0BufferId_)) {
		const MappedBuffer::Plane &rawBufferPlane =
			buffers_.at(rawImage0BufferId_).planes()[0];

		imgBuffViewSetPkg_.channel[channel].view[IMAGE_BUFFER_DCG].data =
			static_cast<const void *>(rawBufferPlane.data());
		imgBuffViewSetPkg_.channel[channel].view[IMAGE_BUFFER_DCG].size =
			static_cast<uint32_t>(rawBufferPlane.size_bytes());
	}

	if (buffers_.count(rawImage1BufferId_)) {
		const MappedBuffer::Plane &rawBufferPlane =
			buffers_.at(rawImage1BufferId_).planes()[0];
		const ImageBufferType type =
			context_.configuration.pipelineMode == IPAPipelineMode::RgbIrDual ?
			IMAGE_BUFFER_DCG : IMAGE_BUFFER_VS;

		imgBuffViewSetPkg_.channel[channel].view[type].data =
			static_cast<const void *>(rawBufferPlane.data());
		imgBuffViewSetPkg_.channel[channel].view[type].size =
			static_cast<uint32_t>(rawBufferPlane.size_bytes());
	}

	ispStatPkg_.channel[channel] = stats;

	int err = liveCtrl.handleCmd(&sensorDataPkg_,
				     &embDataPkg_,
				     &imgBuffViewSetPkg_,
				     &ispStatPkg_,
				     channel,
				     LIVE_CONTROL_CMD_HANDLE_TIMEOUT_MS);
	if (err)
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to handle Live Control command - err=["
			<< err << "]";

	return;
}
#endif

void IPANxpNeo::setInitialControls()
{
	setControls(0);
}

void IPANxpNeo::setControls(uint32_t frame)
{
	/*
	 * Send controls if:
	 * - For any active contexts and algo are processed for all active
	 *   contexts.
	 * - For initial controls (before 1st frame start), no active frame
	 *   contexts yet.
	 */
	for (const auto &processedIt : context_.frameContext.processed)
		if (!processedIt.second)
			return;

	ControlList ctrls(sensorControls_);

	std::vector<Duration> exposures;
	std::vector<double> gains;
	for (const auto &[context, channel] : uguzziChannelMap_) {
		uguzzi_sensor_settings_t &settings = sensorSettings_[channel];

		uint32_t aGainQ16 = settings.exp_n.exp.again;
		uint32_t dGainQ16 = settings.exp_n.exp.dgain;
		uint64_t gainQ16 = static_cast<uint64_t>(
			aGainQ16) * dGainQ16 / UQ16_1;
		double gain = static_cast<double>(gainQ16) / UQ16_1;

		Duration exposure = settings.exp_n.exp.exposure * 1.0us;

		exposures.push_back(exposure);
		gains.push_back(gain);
	}
	camHelper_->controlListSetAGC(&ctrls,
				      Span<Duration>(exposures),
				      Span<double>(gains));

	/* WB gains only apply to RGB stream. */
	unsigned int channelRgb = uguzziChannelMap_.at(IPACameraContext::Rgb);
	uguzzi_sensor_settings_t &settingsRgb = sensorSettings_[channelRgb];
	if (wbLocation_[channelRgb] == UGUZZI_CAM_INFO_WB_LOCATION_SENSOR) {
		/* WB in sensor */
		std::array<double, 4> wbGains;
		uguzzi_awb_gains_t &uguzziWbGains = settingsRgb.wb.wb_gains;
		/* R, Gr, Gb, B */
		wbGains[0] = static_cast<double>(uguzziWbGains.red) / UQ8_1;
		wbGains[1] = static_cast<double>(uguzziWbGains.green) / UQ8_1;
		wbGains[2] = wbGains[1];
		wbGains[3] = static_cast<double>(uguzziWbGains.blue) / UQ8_1;

		camHelper_->controlListSetAWB(&ctrls, Span<const double, 4>(wbGains));
	}

	LOG(NxpNeoUguzziIPA, Debug) << logSensorParams(frame, &mdControls_, &ctrls);

	setSensorControls.emit(frame, ctrls);

	if (!context_.hw.lensPresent)
		return;

	/* For now, autofocus settings are used from the RGB context. */
	if (!lensHwPosition_ || lensHwPosition_.value() != settingsRgb.lens_pos) {
		lensHwPosition_ = settingsRgb.lens_pos;
		ControlList lensControls(lensControls_);
		ControlValue value(lensHwPosition_.value());
		lensControls.set(V4L2_CID_FOCUS_ABSOLUTE, value);
		setLensControls.emit(lensControls);
	}
}

bool IPANxpNeo::libcameraCfa2UguzziBayerPattern(
	uint32_t cfa,
	uguzzi_cam_info_cfa_t *pattern)
{
	bool result = true;

	switch (cfa) {
	case libcamera::properties::draft::ColorFilterArrangementEnum::RGGB:
		*pattern = UGUZZI_CAM_INFO_CFA_RGrGbB;
		break;
	case libcamera::properties::draft::ColorFilterArrangementEnum::GRBG:
		*pattern = UGUZZI_CAM_INFO_CFA_GrRBGb;
		break;
	case libcamera::properties::draft::ColorFilterArrangementEnum::GBRG:
		*pattern = UGUZZI_CAM_INFO_CFA_GbBRGr;
		break;
	case libcamera::properties::draft::ColorFilterArrangementEnum::BGGR:
		*pattern = UGUZZI_CAM_INFO_CFA_BGbGrR;
		break;
	default:
		result = false;
		LOG(NxpNeoUguzziIPA, Error)
			<< "This IPA does not support non-Bayer sensors!";
		break;
	}

	return result;
}

int IPANxpNeo::init(const IPASettings &settings, const InitParams &params,
		    ControlInfoMap *ipaControls,
		    SensorConfig *sensorConfig)
{
	sensorModel_ = settings.sensorModel;
	sensorEntity_ = params.sensorEntity;
	context_.hw.apiVersion = params.apiVersion;
	context_.hw.hwCapabilities = params.hwCapabilities;
	context_.hw.lensPresent = params.lensPresent;

	dataDir_ = utils::dirname(settings.configurationFile) + "/uguzzi";

	/* Load IPA configuration */
	if (loadConfigFile(dataDir_ + "/config_ipa_uguzzi.yaml"))
		return -EINVAL;

	const char *dir = getenv("LIBCAMERA_IPA_UGUZZI_LOG_DIR");
	if (dir) {
		std::string logPath = std::string(dir) + "/" +
				      std::to_string(getpid()) + ".log";
		logSetFile(logPath.c_str());
	}

	if (verifySensorToInit())
		return 0;

	LOG(NxpNeoUguzziIPA, Info) << "IPANxpNeo "
				   << NEO_IPA_UGUZZI_VERSION;
	LOG(NxpNeoUguzziIPA, Debug) << "Initializing for " << sensorModel_
				    << " on hardware revision "
				    << params.hwRevision
				    << " - Sensor: " << sensorEntity_;

	camHelper_ = CameraHelperFactoryBase::create(sensorModel_);
	if (!camHelper_) {
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to create camera helper for "
			<< sensorModel_;
		return -ENODEV;
	}

	/* \note no user controls are supported */
	ControlInfoMap::Map ctrlMap = nxpneoControls;
	*ipaControls = ControlInfoMap(std::move(ctrlMap), controls::controls);

	/* Initialize SensorConfig parameters */
	const CameraHelper::Attributes *attributes = camHelper_->attributes();
	const std::map<int32_t, std::pair<uint32_t, bool>> &camHelperDelayParams =
		attributes->delayedControlParams;

	ControlList ctrls(params.sensorControls);
	auto idMap = ctrls.idMap();
	std::map<int32_t, ipa::nxpneo::DelayedControlsParams> &ipaDelayParams =
		sensorConfig->delayedControlsParams;
	for (const auto &kv : camHelperDelayParams) {
		auto k = kv.first;
		auto v = kv.second;
		if (idMap->find(kv.first) != idMap->end())
			ipaDelayParams.emplace(std::piecewise_construct,
					       std::forward_as_tuple(k),
					       std::forward_as_tuple(v.first, v.second));
		else
			LOG(NxpNeoUguzziIPA, Warning)
				<< "The sensor control list doesn't support the control ID "
				<< utils::hex(kv.first);
	}

	sensorConfig->embeddedTopLines = attributes->mdParams.topLines;

#ifdef USE_LIVE_CONTROL
	LiveControl &liveCtrl = LiveControl::getInstance();
	const uint16_t socketPort = config_.socketPort(sensorModel_,
						       sensorEntity_);
	int ret = liveCtrl.createSocket(socketPort);
	if (ret)
		LOG(NxpNeoUguzziIPA, Error)
			<< "Live Tuning socket is not created!";
	else
		LOG(NxpNeoUguzziIPA, Debug) << "Live Tuning socket is created!";
#else
	LOG(NxpNeoUguzziIPA, Debug)
		<< "Live Tuning/Control is not enabled at compile time!";
#endif

	/* Update the camera helper with sensor control values. */
	camHelper_->sensorControlList(&params.sensorControlList);

	/* Set the IPA initialization state flag to enabled */
	enabled_ = true;

	return 0;
}

int IPANxpNeo::start()
{
	int err = getUguzziInitialSettings();
	if (err)
		return err;

	setInitialControls();

	return 0;
}

void IPANxpNeo::stop()
{
}

int IPANxpNeo::configure(const IPAConfigInfo &ipaConfig,
			 const std::map<IPAStreamType, IPAStream> &streamConfig,
			 ControlInfoMap *ipaControls)
{
	int ret = 0;
	const IPACameraSensorInfo *sensorInfo = &ipaConfig.sensorInfo;
	(void)ipaControls;

	if (!enabled_) {
		LOG(NxpNeoUguzziIPA, Error) << "IPA for sensor "
					    << sensorEntity_
					    << " is not initialized";
		return -ENODEV;
	}

	/* Deinit any previous configure */
#ifdef USE_LIVE_CONTROL
	LiveControl &liveCtrl = LiveControl::getInstance();
	int err = liveCtrl.deinit();
	if (err)
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to deinitialize Live Control!";
#endif
	deinitUguzzi();

	setSessionConfiguration(ipaConfig);

	/* Assign the uguzzi channel with the camera context type. */
	uguzziChannelMap_[IPACameraContext::Rgb] = 0;
	if (ipaConfig.mode == IPAPipelineMode::RgbIrDual)
		uguzziChannelMap_[IPACameraContext::Ir] = 1;

	/* Get the tuning info according to the sensor entity and resolution */
	tuningInfo_ = config_.tuningInfo(sensorModel_, sensorEntity_,
					 sensorInfo->outputSize,
					 sensorInfo->bitsPerPixel,
					 ipaConfig.mode);
	if (!tuningInfo_) {
		LOG(NxpNeoUguzziIPA, Warning) << "No tuningInfo for ["
					      << sensorModel_ << "; "
					      << sensorEntity_ << "; "
					      << sensorInfo->outputSize << "; "
					      << sensorInfo->bitsPerPixel
					      << "bpp; mode:"
					      << static_cast<int>(ipaConfig.mode)
					      << "]";
		return -EINVAL;
	}

	/* Load the DTP binary */
	std::string dtpBin = dataDir_ + "/" + tuningInfo_->dtpFile;
	if (dtp_.load(dtpBin) != 0) {
		LOG(NxpNeoUguzziIPA, Error) << "Failed to load DTP " << dtpBin;
		return -EINVAL;
	}

	/* Initialize uGuzzi with output size including embedded lines */
	Size outputSize = sensorInfo->outputSize;
	outputSize.height += camHelper_->attributes()->mdParams.topLines;
	ret = initializeUguzzi(outputSize);
	if (ret) {
		return ret;
	}
	LOG(NxpNeoUguzziIPA, Debug) << "Successful uGuzzi lib initialization!";

	/* Initialize uguzzi_process() inputs and outputs. */
	initUguzziProcessData();

#ifdef UGUZZI_TESTS_ENABLED
	/*
	 * When adapting the IPA for supporting multicameras with 1 single
	 * uGuzzi instance, this initializeTestsUguzzi() should be called
	 * part of the proxy operating the single instance.
	 */
	if (initializeTestsUguzzi(Span<uint32_t>(IPANxpNeo::camFrames),
				  uguzziChannelMap_.size(),
				  sensorModel_))
		LOG(NxpNeoUguzziIPA, Error) << "Failed to initialize tests!";
#endif
	/* Configure uGuzzi */
	ret = configureUguzzi(streamConfig);
	if (ret) {
		return ret;
	}

	/* Get DTP configuration */
	ret = getDTPConfig();
	if (ret) {
		return ret;
	}

	/* Check DTP configuration */
	ret = checkDTPConfig(*sensorInfo);
	if (ret) {
		return ret;
	}

#ifdef USE_LIVE_CONTROL
	/* Initialize Live Control */
	/*
	 * Live Connect gets attached to the initialized uGuzzi,
	 * hence, live connect should be initialized with
	 * uguzzi_live_tuning_init() once uGuzzi is initialized.
	 * Use uGuzzi channel 0 to get width and height.
	 */
	ret = liveCtrl.init(camInfoDtp_[0]->frame1_cfg.width,
			    camInfoDtp_[0]->frame1_cfg.height);
	if (ret)
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to initialize Live Control!";
	else
		LOG(NxpNeoUguzziIPA, Debug)
			<< "Live Tuning/Control is initialized!";
#endif
	CameraMode cameraMode;
	cameraMode.pixelRate = sensorInfo->pixelRate;
	cameraMode.bitdepth = sensorInfo->bitsPerPixel;
	cameraMode.width = sensorInfo->outputSize.width;
	cameraMode.height = sensorInfo->outputSize.height;
	cameraMode.hblank = ipaConfig.sensorControlList.get(V4L2_CID_HBLANK).get<int32_t>();
	cameraMode.vblank = ipaConfig.sensorControlList.get(V4L2_CID_VBLANK).get<int32_t>();
	auto iter = kSensorStreamModeMap.find(ipaConfig.mode);
	if (iter != kSensorStreamModeMap.end()) {
		cameraMode.streamMode = iter->second;
	} else {
		cameraMode.streamMode = SensorStreamStandard;
		LOG(NxpNeoUguzziIPA, Warning)
			<< "No sensor stream mode found for pipeline mode: "
			<< static_cast<int>(ipaConfig.mode)
			<< " - Default mode is used: " << cameraMode.streamMode;
	}
	camHelper_->setCameraMode(cameraMode);

	sensorControls_ = ipaConfig.sensorControls;
	lensControls_ = ipaConfig.lensControls;

	/* Clear the Live tuning buffers */
	imgBuffViewSetPkg_ = {};
	ispStatPkg_ = {};
	embDataPkg_ = {};

	return 0;
}

void IPANxpNeo::mapBuffers(const std::vector<IPABuffer> &buffers)
{
	for (const IPABuffer &buffer : buffers) {
		const FrameBuffer fb(buffer.planes);
		buffers_.emplace(buffer.id,
				 MappedFrameBuffer(
					 &fb,
					 MappedFrameBuffer::MapFlag::ReadWrite));
	}
}

void IPANxpNeo::unmapBuffers(const std::vector<unsigned int> &ids)
{
	for (unsigned int id : ids) {
		auto it = buffers_.find(id);
		if (it == buffers_.end())
			continue;

		buffers_.erase(it);
	}
}

void IPANxpNeo::queueRequest(const uint32_t frame, const ControlList &controls)
{
	(void)frame;
	(void)controls;

	/* Clear the IPA frame context before processing a new frame. */
	context_.frameContext = {};

	for (const auto &ctxt : context_.configuration.activeContexts)
		context_.frameContext.processed[ctxt] = false;
}

void IPANxpNeo::computeParams(const uint32_t frame, const IPACameraContext context,
			      const std::map<IPABufferType, uint32_t> &bufferIds)
{
	ControlList &controls = mdControls_;
	controls = ControlList(md::controlIdMap);

	unsigned int channel = uguzziChannelMap_.at(context);
	/* Update the frame id. */
	camFrames[channel] = frame;

	uint8_t *metaData = nullptr;
	size_t metaSize = 0;
	metaDataValid_ = false;

	/* Give access to raw buffers for live tuning */
	auto input0It = bufferIds.find(IPABufferType::Image0);
	rawImage0BufferId_ = input0It != bufferIds.end() ? input0It->second : 0;
	auto input1It = bufferIds.find(IPABufferType::Image1);
	rawImage1BufferId_ = input1It != bufferIds.end() ? input1It->second : 0;

	/*
	 * Look for metadata availability, either from the camera embedded data
	 * stream or from the pixel data top lines.
	 */
	auto eDataIt = bufferIds.find(IPABufferType::EData);
	unsigned int eDataBufferId =
		eDataIt != bufferIds.end() ? eDataIt->second : 0;
	if (eDataBufferId && buffers_.count(eDataBufferId)) {
		const MappedBuffer::Plane &plane =
			buffers_.at(eDataBufferId).planes()[0];
		metaData = plane.data();
		metaSize = plane.size_bytes();
	} else if (rawImage0BufferId_ && buffers_.count(rawImage0BufferId_)) {
		/*
		 * For now, embedded data is only supported from the raw Image0.
		 */
		const MappedBuffer::Plane &plane =
			buffers_.at(rawImage0BufferId_).planes()[0];
		metaData = plane.data();
		uint32_t topLines = camHelper_->attributes()->mdParams.topLines;
		std::array<uint32_t, 2> &bpps =
			context_.configuration.sensor.bpps;
		size_t bytepp =
			bpps[0] <= 8 ? sizeof(uint8_t) : sizeof(uint16_t);
		unsigned int width =
			context_.configuration.sensor.size.width;
		metaSize = topLines * width * bytepp;
	}

	if (metaSize) {
		Span<uint8_t> mdBuffer(metaData, metaSize);
		if (!camHelper_->parseEmbedded(mdBuffer, &controls))
			metaDataValid_ = true;
	}

	auto paramsIter = bufferIds.find(IPABufferType::Params);
	unsigned int paramsBufferId =
		paramsIter != bufferIds.end() ? paramsIter->second : 0;
	if (!buffers_.count(paramsBufferId)) {
		LOG(NxpNeoUguzziIPA, Error)
			<< "Parameters buffer " << paramsBufferId
			<< " not mapped for frame " << frame;
		return;
	}
	NxpNeoParams params(context_.hw.apiVersion,
			    buffers_.at(paramsBufferId).planes()[0]);

	imx9x_isp_cfg_prms_t &cfgParams =
		ispSettings_[channel].isp_cfg_params[0];
	convertUguzziIspCfg2IspDrvCfg(&cfgParams,
				      sensorDataPkg_.channel[channel].l2vs_ratio,
				      &params);
	overrideParams(&cfgParams, &params);

	paramsComputed.emit(frame, context, params.size());
}

void IPANxpNeo::processStats(const uint32_t frame, const IPACameraContext context,
			     const std::map<IPABufferType, uint32_t> &bufferIds,
			     const ControlList &sensorControls)
{
	auto statsIter = bufferIds.find(IPABufferType::Stats);
	unsigned int statsBufferId =
		statsIter != bufferIds.end() ? statsIter->second : 0;
	if (!buffers_.count(statsBufferId)) {
		LOG(NxpNeoUguzziIPA, Error)
			<< "Statistics buffer " << statsBufferId
			<< " not mapped for frame " << frame;
		return;
	}

	const NxpNeoStats stats(context_.hw.apiVersion,
				buffers_.at(statsBufferId).planes()[0]);

	ControlList &controls = mdControls_;

	/**
	 * No image metadata are available so use the delay line maintained
	 * by the pipeline handler.
	 */
	if (!metaDataValid_) {
		if (!frame)
			LOG(NxpNeoUguzziIPA, Info)
				<< "Metadata not available, delay lines are used instead!";
		controls = ControlList(md::controlIdMap);
		camHelper_->sensorControlsToMetaData(&sensorControls, &controls);
	}

	unsigned int channel = uguzziChannelMap_.at(context);
	uguzzi_sensor_data_t *sensorData = &sensorDataPkg_.channel[channel];
	metaDataToSensorData(&controls, sensorData);

	prepareUguzziSensorData(frame, channel);

	prepareUguzziStats(channel, &stats);

	int err = processUguzzi(channel);
	if (err)
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to process ISP statistics";

	ControlList metadata(controls::controls);
	if (context == IPACameraContext::Rgb) {
		/*
		 * Metadata are only filled in RGB context.
		 * This is to avoid overwritten the same control in Ir context.
		 */
		unsigned int channelRGB = uguzziChannelMap_.at(context);
		metadata.set(controls::Lux,
			     static_cast<float>(ispSettingsPkg_.uguzzi_metadata[channelRGB].aec_info[22]));
		metadata.set(controls::ColourTemperature,
			     sensorSettings_[channelRGB].wb.colour_temp);
		/* add more as needed */
	}
	/* Set processed flag for this context. */
	context_.frameContext.processed.at(context) = true;

	setControls(frame);

#ifdef USE_LIVE_CONTROL
	processLiveControl(channel, &stats);
#endif
	metadataReady.emit(frame, context, metadata);
}

/**
 * \brief Convert metadata control list to uguzzi sensor data structure
 *
 * This function converts a metadata control list into a uguzzi sensor data
 * structure, doing the necessary sanity checks and format conversions.
 * Metadata for analog gain, digital gain and exposure are expected either
 * a single value for the main (long) capture, or up to 3 captures in the
 * following order: long, short then very short.
 *
 * \param[in] mdCtrls The metadata control list
 * \param[out] sensorData The uguzzi sensor data structure to populate
 */
void IPANxpNeo::metaDataToSensorData(
	const ControlList *mdCtrls, uguzzi_sensor_data_t *sensorData) const
{
	bool mdValid = true;
	bool mdMultiCapture = false;

	std::array<float, UGUZZI_WDR3_ENTRY_MAX> aGainArray =
		{ 1.0f, 1.0f, 1.0f };
	Span<const float> aGain = Span<float>(aGainArray);
	if (mdCtrls->contains(md::AnalogueGain.id())) {
		const ControlValue &aGainCtrl =
			mdCtrls->get(md::AnalogueGain.id());
		Span<const float> aGainValue =
			aGainCtrl.get<Span<const float>>();
		assert(aGainValue.size() <= UGUZZI_WDR3_ENTRY_MAX);
		std::copy(aGainValue.begin(), aGainValue.end(),
			  aGainArray.begin());
	} else {
		LOG(NxpNeoUguzziIPA, Warning) << "No analog gain metadata";
		mdValid = false;
	}

	std::array<float, UGUZZI_WDR3_ENTRY_MAX> dGainArray =
		{ 1.0f, 1.0f, 1.0f };
	Span<const float> dGain = Span<float>(dGainArray);
	if (mdCtrls->contains(md::DigitalGain.id())) {
		const ControlValue &dGainCtrl =
			mdCtrls->get(md::DigitalGain.id());
		Span<const float> dGainValue =
			dGainCtrl.get<Span<const float>>();
		assert(dGainValue.size() <= UGUZZI_WDR3_ENTRY_MAX);
		std::copy(dGainValue.begin(), dGainValue.end(),
			  dGainArray.begin());
	} else {
		LOG(NxpNeoUguzziIPA, Warning) << "No digital gain metadata";
		mdValid = false;
	}

	std::array<float, UGUZZI_WDR3_ENTRY_MAX> exposureArray =
		{ 1.0f, 0.0f, 0.0f };
	Span<const float> exposure = Span<float>(exposureArray);
	if (mdCtrls->contains(md::Exposure.id())) {
		const ControlValue &exposureCtrl =
			mdCtrls->get(md::Exposure.id());
		Span<const float> exposureValue =
			exposureCtrl.get<Span<const float>>();
		assert(exposureValue.size() <= UGUZZI_WDR3_ENTRY_MAX);
		std::copy(exposureValue.begin(), exposureValue.end(),
			  exposureArray.begin());
		mdMultiCapture = (exposureValue.size() == UGUZZI_WDR3_ENTRY_MAX);
	} else {
		LOG(NxpNeoUguzziIPA, Warning) << "No exposure metadata";
		mdValid = false;
	}

	std::array<float, 4> wbGainArray = { 1.0f, 1.0f, 1.0f, 1.0f };
	Span<const float> wbGain = Span<float>(wbGainArray);
	if (mdCtrls->contains(md::WhiteBalanceGain.id())) {
		const ControlValue &wbGainCtrl =
			mdCtrls->get(md::WhiteBalanceGain.id());
		Span<const float> wbGainValue =
			wbGainCtrl.get<Span<const float>>();
		assert(wbGainValue.size() == 4);
		std::copy(wbGainValue.begin(), wbGainValue.end(),
			  wbGainArray.begin());
	} else {
		LOG(NxpNeoUguzziIPA, Warning) << "No wb gain metadata";
		mdValid = false;
	}

	float temperature = 25.0;
	if (mdCtrls->contains(md::Temperature.id())) {
		const ControlValue &tempGainCtrl =
			mdCtrls->get(md::Temperature.id());
		temperature = tempGainCtrl.get<float>();
	} else {
		LOG(NxpNeoUguzziIPA, Warning) << "No temperature metadata";
	}

	/*
	 * Exposure and gain - units:
	 * - exposure: seconds for metadata, micro seconds for sensor data
	 * - again: raw gain in metadata, UQ16.16 for sensor data
	 * - dgain: raw gain in metadata, UQ16.16 for sensor data
	 */
	uguzzi_exposure_t *exposureL =
		&sensorData->exp_gain[UGUZZI_WDR3_ENTRY_LONG];
	exposureL->exposure =
		static_cast<uint32_t>(exposure[UGUZZI_WDR3_ENTRY_LONG] * 1.0e6f);
	exposureL->again =
		static_cast<uint32_t>(aGain[UGUZZI_WDR3_ENTRY_LONG] *
				      dGain[UGUZZI_WDR3_ENTRY_LONG] * UQ16_1);
	exposureL->dgain = UQ16_1;

	uguzzi_exposure_t *exposureS =
		&sensorData->exp_gain[UGUZZI_WDR3_ENTRY_SHORT];
	exposureS->exposure = exposureL->exposure;
	exposureS->again =
		static_cast<uint32_t>(aGain[UGUZZI_WDR3_ENTRY_SHORT] *
				      dGain[UGUZZI_WDR3_ENTRY_SHORT] * UQ16_1);
	exposureS->dgain = UQ16_1;

	uguzzi_exposure_t *exposureVS =
		&sensorData->exp_gain[UGUZZI_WDR3_ENTRY_VERY_SHORT];
	exposureVS->exposure =
		static_cast<uint32_t>(exposure[UGUZZI_WDR3_ENTRY_VERY_SHORT] * 1.0e6f);
	exposureVS->again =
		static_cast<uint32_t>(aGain[UGUZZI_WDR3_ENTRY_VERY_SHORT] *
				      dGain[UGUZZI_WDR3_ENTRY_VERY_SHORT] * UQ16_1);
	exposureVS->dgain = UQ16_1;

	/*
	 * Capture ratios L2S and L2VS - unit UQ24.8
	 */
	if (mdMultiCapture) {
		uint64_t totalL =
			static_cast<uint64_t>(exposureL->exposure) *
			static_cast<uint64_t>(exposureL->again);

		uint64_t totalS =
			static_cast<uint64_t>(exposureS->exposure) *
			static_cast<uint64_t>(exposureS->again);
		if (!totalS) {
			totalS = 1U;
			mdValid = false;
			LOG(NxpNeoUguzziIPA, Warning)
				<< "Short total exposure is 0";
		}

		uint64_t totalVS =
			static_cast<uint64_t>(exposureVS->exposure) *
			static_cast<uint64_t>(exposureVS->again);
		if (!totalVS) {
			totalVS = 1U;
			mdValid = false;
			LOG(NxpNeoUguzziIPA, Warning)
				<< "Very Short total exposure is 0";
		}

		sensorData->l2s_ratio =
			static_cast<uint32_t>((totalL * UQ8_1) / totalS);
		sensorData->l2vs_ratio =
			static_cast<uint32_t>((totalL * UQ8_1) / totalVS);
	} else {
		sensorData->l2s_ratio = UQ8_1;
		sensorData->l2vs_ratio = UQ8_1;
	}

	/* WB gains - metadata is raw gain, sensor data UQ8.8 */
	sensorData->wb.wb_gains.red =
		static_cast<uint16_t>(wbGain[0] * UQ8_1);
	sensorData->wb.wb_gains.green =
		static_cast<uint16_t>((wbGain[1] + wbGain[2]) / 2 * UQ8_1);
	sensorData->wb.wb_gains.blue =
		static_cast<uint16_t>(wbGain[3] * UQ8_1);
	sensorData->wb.offsets.red = 0;
	sensorData->wb.offsets.green = 0;
	sensorData->wb.offsets.blue = 0;

	/*
	 * Temperature in degrees C
	 * raw in metadata, UQ8.8 (positive) in sensor data
	 */
	if (temperature < 0.0)
		temperature = 0.0f;

	sensorData->sensor_temperature_c =
		static_cast<uint16_t>((temperature * UQ8_1));

	/*
	 * There is no sensor feedback from the camera about the lens position
	 * so simply return the controlled value when it is available.
	 */
	sensorData->applied_lens_pos = lensHwPosition_ ?
		lensHwPosition_.value() : 0;

	sensorData->valid = mdValid;
}

bool IPANxpNeo::isYuvFormat(const PixelFormat &format) const
{
	/* Keep in sync with NeoDevice::frameFormats() */
	static const PixelFormat formats[] = {
		formats::YUYV,
		formats::UYVY,
		formats::XVUY8888,
		formats::NV12,
		formats::NV21,
		formats::NV16,
		formats::NV61,
	};

	auto it = std::find(std::begin(formats), std::end(formats), format);
	return it != std::end(formats);
}

bool IPANxpNeo::isMonochrome(const PixelFormat &format) const
{
	/* Keep in sync with NeoDevice::frameFormats() */
	static const PixelFormat formats[] = {
		formats::R8,
		formats::R10,
		formats::R12,
		formats::R16,
	};

	auto it = std::find(std::begin(formats), std::end(formats), format);
	return it != std::end(formats);
}

std::string IPANxpNeo::controlListToString(const ControlList *ctrls) const
{
	std::stringstream log;
	for (auto it = ctrls->begin(); it != ctrls->end(); ++it) {
		ControlValue value = it->second;
		if (it != ctrls->begin())
			log << "\n";
		log << it->first << ": val=" << value.toString();
	}

	return log.str();
}

std::string IPANxpNeo::logSensorParams(const uint32_t frame,
				       const ControlList *ctrlsApplied,
				       const ControlList *ctrlsToApply) const
{
	std::stringstream log;

	log << "\n--- frame [" << frame << "] meta data:\n"
	    << controlListToString(ctrlsApplied)
	    << "\nupdate:\n"
	    << controlListToString(ctrlsToApply);

	return log.str();
}

/**
 * \brief Load the IPA configuration file
 * \param[in] filename The path to configuration file
 * \return 0 on success, or a negative error code otherwise
 */
int IPANxpNeo::loadConfigFile(const std::string &filename)
{
	int ret = config_.load(filename);
	if (ret)
		LOG(NxpNeoUguzziIPA, Error) << "Failed to load config file "
					    << filename;

	return ret;
}

} // namespace ipa::nxpneo

/*
 * External IPA module interface
 */

extern "C" {
const struct IPAModuleInfo ipaModuleInfo = {
	IPA_MODULE_API_VERSION,
	1,
	"nxp/neo",
	"nxp/neo",
};

IPAInterface *ipaCreate()
{
	return new ipa::nxpneo::IPANxpNeo();
}
}

} /* namespace libcamera */
