/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * neo.cpp - NXP NEO Image Processing Algorithms
 * Copyright 2024-2025 NXP
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
		      const std::map<uint32_t, IPAStream> &streamConfig,
		      ControlInfoMap *ipaControls) override;
	void mapBuffers(const std::vector<IPABuffer> &buffers) override;
	void unmapBuffers(const std::vector<unsigned int> &ids) override;

	void queueRequest(const uint32_t frame, const ControlList &controls) override;
	void computeParams(const uint32_t frame, const IPAContextType context,
			   const std::map<uint32_t, uint32_t> &bufferIds) override;
	void processStats(const uint32_t frame, const IPAContextType context,
			  const std::map<uint32_t, uint32_t> &bufferIds,
			  const ControlList &sensorControls) override;

private:
	int initializeUguzzi(Size outputSize);
	void deinitUguzzi();
	int configureUguzzi(const std::map<uint32_t, IPAStream> &streamConfig);
	int verifySensorToInit();
	int getDTPConfig();
	int checkDTPConfig(const IPACameraSensorInfo &sensorInfo);
	int setUguzziInitialConfig();
	int setUguzziStreamConfig(const std::map<uint32_t, IPAStream> &streamConfig);
	int getUguzziInitialSettings();
	int getCamInfoFromDTP();
	int getWbLocationFromDTP();

	void prepareUguzziSensorData();

	void convertCtempRegsToAwbBbc(const neoisp_ctemp_reg_stats_s *ctempRegs,
				      imx9x_isp_ctemp_bbc_output_t *awbBbc);
	void convertCtempRegsToAwbCrois(const neoisp_ctemp_reg_stats_s *ctempRegs,
					imx9x_isp_ctemp_color_rois_output_t *awbCrois);

	void prepareUguzziAecHistograms(const vpipe_settings_hw_t *cfg,
					const neoisp_rgbir_mem_stats_s *rgbirHist,
					const neoisp_hist_mem_stats_s *hist,
					ae_histogram_data_hw_t *outHistData);
	void prepareUguzziAwbStats(imx9x_isp_awb_stats_type_t statType,
				   const neoisp_ctemp_reg_stats_s *ctempRegs,
				   const neoisp_ctemp_mem_stats_s *ctempMems,
				   awb_statistics_data_hw_t *outAwbData);
	void prepareUguzziAfStats(const vpipe_settings_hw_t *cfg,
				  const neoisp_af_reg_stats_s *afRegs,
				  const neoisp_drc_mem_stats_s *drcMems,
				  af_statistics_data_hw_t *outAfData);
	void prepareUguzziStats(const NxpNeoStats *stats);

	int processUguzzi(uguzzi_sensor_data_pkg_t *sensorDataPkg,
			  uguzzi_stats_data_pkg_t *statsDataPkg,
			  uguzzi_sensor_settings_pkg_t *sensorSettingsPkg,
			  uguzzi_isp_settings_pkg_t *ispSettingsPkg);

	void setControls(unsigned int frame, IPAContextType context);
	bool libcameraCfa2UguzziBayerPattern(uint32_t cfa,
					     uguzzi_cam_info_cfa_t *pattern);
	std::string controlListToString(const ControlList *ctrls) const;
	std::string logSensorParams(const unsigned int frame,
				    const ControlList *ctrlsApplied,
				    const ControlList *ctrlsToApply) const;

	void metaDataToSensorData(const ControlList *mdCtrls,
				  uguzzi_sensor_data_t *sensorData) const;

	bool isYuvFormat(const PixelFormat &format) const;
	bool isMonochrome(const PixelFormat &format) const;
#ifdef USE_LIVE_CONTROL
	void processLiveControl(const NxpNeoStats *stats);
#endif
	int loadConfigFile(const std::string &filename);

	/* Constant value 1 in UQx.8 and UQx.16 fixed point formats */
	static constexpr unsigned int UQ8_1 = (1 << 8);
	static constexpr unsigned int UQ16_1 = (1 << 16);

	bool uguzziInitialized_{ false };
	bool metaDataValid_{ false };
	IPACameraSensorInfo sensorInfo_;

	uint32_t rawImage0BufferId_;
	uint32_t rawImage1BufferId_;

	uint16_t cameraCnt_;
	uint8_t channel_;
	std::string sensorModel_;
	std::string sensorEntity_;

	/* apiVersion for metadata accesses */
	uint32_t apiVersion_;

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
	uint32_t sResultLongHist[STAT_HIST_BINS];
	uint32_t sResultShortHist[STAT_HIST_BINS];
	uint32_t sResultVshortHist[STAT_HIST_BINS];

	/* uguzzi_process() inputs */
	uguzzi_sensor_data_pkg_t mSensorDataPkg;
	uguzzi_stats_data_pkg_t mStatsDataPkg;
	ae_histogram_data_hw_t mAeHistData[UGUZZI_CAMERA_CNT];
	ae_statistics_data_hw_t mAgblbceStats[UGUZZI_CAMERA_CNT];
	awb_statistics_data_hw_t mAwbStatsData[UGUZZI_CAMERA_CNT];
	imx9x_isp_awb_stats_type_t mAwbStatType[UGUZZI_CAMERA_CNT];
	af_statistics_data_hw_t mAfStatsData[UGUZZI_CAMERA_CNT];

	/* uguzzi_process outputs */
	uguzzi_sensor_settings_pkg_t mSensorSettingsPkg;
	uguzzi_sensor_settings_t mSensorSettings[UGUZZI_CAMERA_CNT];
	uguzzi_isp_settings_pkg_t mIspSettingsPkg;
	vpipe_settings_hw_t mIspSettings[UGUZZI_CAMERA_CNT];

	std::map<unsigned int, MappedFrameBuffer> buffers_;

	/* Interface to the Camera Helper */
	std::unique_ptr<CameraHelper> camHelper_;

	ControlInfoMap sensorControls_;
	ControlInfoMap lensControls_;

	bool lensPresent_ = false;
	std::optional<int32_t> lensHwPosition_;

	ControlList mdControls_;

	IPAModeType pipelineMode_;

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

	/* Map between the IPA stream mode and the cameraHelper stream mode. */
	static const std::map<const IPAModeType, SensorStreamModes> kSensorStreamModeMap;
	static const std::map<const IPAContextType, SensorContextTypes> kSensorContextMap;
};

const std::map<const IPAModeType, SensorStreamModes> IPANxpNeo::kSensorStreamModeMap = {
	{ IPAModeTypeStandard, SensorStreamStandard },
	{ IPAModeTypeHdrMerge, SensorStreamHdr },
	{ IPAModeTypeRgbIr, SensorStreamRgbIr },
	{ IPAModeTypeRgbIrDual, SensorStreamDualContext },
};

const std::map<const IPAContextType, SensorContextTypes> IPANxpNeo::kSensorContextMap = {
	{ IPAContextTypeRgb, SensorContextRgb },
	{ IPAContextTypeIr, SensorContextIr },
};

namespace {

/* List of controls handled by the NeoNxp IPA */
const ControlInfoMap::Map nxpneoControls{};

} /* namespace */

IPANxpNeo::IPANxpNeo()
	: camFrames{}
{
	/*
	 * Only 1 camera per uGuzzi instance (per IPA process) is
	 * supported for now. The channel 0 from the uGuzzi instance is used.
	 *
	 * However to support later surround view, all cameras will
	 * be processed by one single uGuzzi instance. Each IPA will access the
	 * uGuzzi structures using a different uGuzzi channel corresponding
	 * to their camera.
	 * IPA would not call uguzzi lib directly but via a proxy operating
	 * the single library instance.
	 * The cameraCnt_ will be initialized by the proxy according to the
	 * number of registered cameras.
	 */
	cameraCnt_ = 1;
	channel_ = 0;
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
	uguzzi_init_configuration_t initCfg{};
	initCfg.camera_cnt = cameraCnt_;
	initCfg.dtp_database = dtp_.data();
	initCfg.dtp_database_size = dtp_.size();

	initCfg.channel_config[channel_].sensor_id = tuningInfo_->tuningId;
	initCfg.channel_config[channel_].sensor_mode = tuningInfo_->tuningMode;
	initCfg.channel_config[channel_].image_W = outputSize.width;
	initCfg.channel_config[channel_].image_H = outputSize.height;
	/* ISP always adjusts the pattern to RGGB on entry */
	initCfg.channel_config[channel_].pattern = IMX9X_ISP_BAYER_RGrGbB;
	initCfg.channel_config[channel_].isp_active = 1;

	int err = uguzzi_init(&initCfg);
	if (err) {
		uguzziInitialized_ = false;
		LOG(NxpNeoUguzziIPA, Error) << "Failed to initialize uGuzzi library";
		return err;
	}

	uguzziInitialized_ = true;

	mStatsDataPkg.channel[channel_].p_ae_stats = &mAgblbceStats[channel_];
	mStatsDataPkg.channel[channel_].p_ae_hist = &mAeHistData[channel_];
	mStatsDataPkg.channel[channel_].p_awb_stats = &mAwbStatsData[channel_];
	mStatsDataPkg.channel[channel_].p_awb_sw_stats = nullptr;
	mStatsDataPkg.channel[channel_].p_af_stats = &mAfStatsData[channel_];
	mSensorSettingsPkg.channel[channel_] = &mSensorSettings[channel_];
	mIspSettingsPkg.isp_config[channel_] = &mIspSettings[channel_];

	return 0;
}

void IPANxpNeo::deinitUguzzi()
{
	if (uguzziInitialized_) {
		uguzzi_delete();
		uguzziInitialized_ = false;
	}
}

int IPANxpNeo::configureUguzzi(const std::map<uint32_t, IPAStream> &streamConfig)
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

/**
 * \brief Verify conditions to initialize IPA/sensor
 *
 * The conditions to be fulfilled so that the IPA instance can be started are:
 * - Non-isolated mode: no other IPA instance was previously initialized
     in the same process and sensor filtering not defined in the configuration file.
 * - Sensor filtering: if defined in the configuration file, the actual sensor entity
 *   name should match the definition.
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
		LOG(NxpNeoUguzziIPA, Error) << "Failed to read camera info from DTP";
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
 * This function checks if the parameters used for tuning are correct with the
 * sensor information.
 * It is checking the width, height and embedded top lines.
 * The CFA pattern is not checked since the libcamera ColorFilterArrangement
 * definition doesn't cover the RGBIr format.
 *
 * \param[in] sensorInfo The sensor information
 */
int IPANxpNeo::checkDTPConfig(const IPACameraSensorInfo &sensorInfo)
{
	const uguzzi_cam_info_cfa_t camInfoPattern =
		static_cast<uguzzi_cam_info_cfa_t>(
			camInfoDtp_[channel_]->frame1_cfg.cfa);

	uint32_t topLines = camHelper_->attributes()->mdParams.topLines;
	/* outputSize from sensorInfo is cropped to remove the embedded lines */
	uint32_t sensorOutputWidth = sensorInfo.outputSize.width;
	uint32_t sensorOutputHeight = sensorInfo.outputSize.height + topLines;
	const bool sensorConfigDiffers =
		sensorOutputWidth != camInfoDtp_[channel_]->frame1_cfg.width ||
		sensorOutputHeight != camInfoDtp_[channel_]->frame1_cfg.height ||
		(topLines &&
		 topLines != camInfoDtp_[channel_]->frame1_cfg.front_emb_ln_cnt);

	LOG(NxpNeoUguzziIPA, Debug) << "DTP CFA pattern: " << camInfoPattern;
	if (sensorConfigDiffers)
		LOG(NxpNeoUguzziIPA, Warning)
			<< "Sensor frame and DTP frame configuration differs "
			<< "[W, H, nb_emb_ln] = ["
			<< sensorOutputWidth
			<< ", " << sensorOutputHeight
			<< ", " << topLines << "] versus ["
			<< camInfoDtp_[channel_]->frame1_cfg.width
			<< ", " << camInfoDtp_[channel_]->frame1_cfg.height
			<< ", "
			<< camInfoDtp_[channel_]->frame1_cfg.front_emb_ln_cnt
			<< "]";

	return 0;
}

int IPANxpNeo::setUguzziInitialConfig()
{
	int err = 0;

	uguzzi_configuration_t cfg{};
	cfg.channel_id = channel_;
	cfg.config_id = CMD_AE_MODE;
	/*
	 * Use harmonized AEC by default because it is the most likely use case
	 * for the IPA handling multiple sensors
	 */
	cfg.config_val = 0;
	err |= uguzzi_config(&cfg);

	cfg.config_id = CMD_AE_FLICKER_MODE;
	cfg.config_val = UGUZZI_MAINS_FREQ_UNKNOWN;
	err |= uguzzi_config(&cfg);

	cfg.config_id = CMD_AE_LFM_MODE;
	cfg.config_val = UGUZZI_LFM_MODE_OFF;
	err |= uguzzi_config(&cfg);

	/* Disable the uGuzzi AF processing (0: normal, 1: disabled) */
	cfg.config_id = CMD_AF_PROCESSING_MODE;
	cfg.config_val = lensPresent_ ? 0 : 1;
	err |= uguzzi_config(&cfg);

	/*
	 * Raise the flag even if none of the above configs succeeded.
	 * A config that changes the value of a static parameter might
	 * fail because an uGuzzi algorithm couldn't find a new tuning
	 * matching that new value. But in that case the value of the
	 * static parameter did change and thus external algorithms must
	 * be notified because their tuning might depend on that static parameter.
	 */
	mConfigChanged = true;

	return err;
}

int IPANxpNeo::setUguzziStreamConfig(const std::map<uint32_t, IPAStream> &streamConfig)
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

	uguzzi_configuration_t cfg{};
	cfg.channel_id = 0U;
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
	int err = 0;

	uguzzi_configuration_t cfg{};
	cfg.channel_id = channel_;
	cfg.config_id = CMD_AE_MODE;
	cfg.config_val = 0;
	err = uguzzi_config(&cfg);

	err |= processUguzzi(NULL, NULL, &mSensorSettingsPkg, &mIspSettingsPkg);
	if (err)
		LOG(NxpNeoUguzziIPA, Error) << "Failed to get uGuzzi initial settings";

	return err;
}

int IPANxpNeo::getCamInfoFromDTP()
{
	uguzzi_dtp_output_t dtpOutput;
	int dtpError = uguzzi_ext_algs_dtp_query(channel_,
						 UGUZZI_DTP_ID_EXT_ALGS_CAMERA_INFO,
						 UGUZZI_CAM_INFO_DTP_SUBTYPE_ID,
						 &dtpOutput);
	if (dtpError) {
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to read camera info DTP for camera " << channel_;
		return dtpError;
	}

	if (dtpOutput.type != UGUZZI_DTP_DATA_TYPE_FIXED) {
		LOG(NxpNeoUguzziIPA, Error) << "Wrong DTP type for camera "
					    << channel_
					    << "! Only Fixed type is supported";
		return -EINVAL;
	}

	camInfoDtp_[channel_] =
		static_cast<const uguzzi_cam_info_dtp_t *>(dtpOutput.x1y1);

	return 0;
}

/* Must be called only after successful call to IPANxpNeo::getCamInfoFromDTP() */
int IPANxpNeo::getWbLocationFromDTP()
{
	switch (camInfoDtp_[channel_]->apply_wb_gains_in) {
	case UGUZZI_CAM_INFO_WB_LOCATION_SENSOR:
		LOG(NxpNeoUguzziIPA, Debug)
			<< "WB gains will be applied in the sensor for camera "
			<< channel_;
		break;
	case UGUZZI_CAM_INFO_WB_LOCATION_ISP:
		LOG(NxpNeoUguzziIPA, Debug)
			<< "WB gains will be applied in the ISP for camera "
			<< channel_;
		break;
	case UGUZZI_CAM_INFO_WB_LOCATION_OTHER:
		LOG(NxpNeoUguzziIPA, Error)
			<< "Unsupported location for applying WB gains for camera "
			<< channel_;
		return -EINVAL;
	default:
		LOG(NxpNeoUguzziIPA, Error)
			<< "Unknown location for applying WB gains for camera "
			<< channel_;
		return -EINVAL;
	}

	wbLocation_[channel_] =
		static_cast<uguzzi_cam_info_wb_location_t>(camInfoDtp_[channel_]->apply_wb_gains_in);

	return 0;
}

/**
 * The sensor data for each camera must contain the total WB gains applied on the image
 * until the AWB block statistics are made. This includes digital gains applied
 * through OBWB and BNR, and other blocks that can apply per channel gain before the
 * AWB block statistics are generated.
 * The per channel gains applied after AWB block statistics are NOT to be included.
 */
void IPANxpNeo::prepareUguzziSensorData()
{
	if (!mSensorDataPkg.channel[channel_].valid) {
		/* Skip invalid sensor data. */
		return;
	}

	const imx9x_isp_cfg_prms_t *ispCfg =
		&mIspSettingsPkg.isp_config[channel_]->isp_cfg_params[0];
	uguzzi_awb_gains_t *sensorWbGains =
		&mSensorDataPkg.channel[channel_].wb.wb_gains;

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
		/* Scale down the gains. This is to preserve the color ratios and
		 * as much of the digital gain as possible.
		 */
		/* 1.0 = 65536 and round it up to make sure the gains will be below the max */
		const uint64_t scale = (maxGain * 65536U + UINT16_MAX) / UINT16_MAX;
		redGain = (redGain * 65536U) / scale;
		greenGain = (greenGain * 65536U) / scale;
		blueGain = (blueGain * 65536U) / scale;

		LOG(NxpNeoUguzziIPA, Warning)
			<< "Too high total WB gains applied on the image for camera "
			<< channel_ << "Quality of AWB algorithm might be reduced!";
	}

	sensorWbGains->red = static_cast<uint16_t>(redGain);
	sensorWbGains->green = static_cast<uint16_t>(greenGain);
	sensorWbGains->blue = static_cast<uint16_t>(blueGain);

	const bool wbGainsValid = sensorWbGains->red >= 256U &&
				  sensorWbGains->green >= 256U &&
				  sensorWbGains->blue >= 256U;
	mSensorDataPkg.channel[channel_].valid = wbGainsValid ? 1U : 0U;
}

void IPANxpNeo::convertCtempRegsToAwbBbc(const neoisp_ctemp_reg_stats_s *ctempRegs,
					 imx9x_isp_ctemp_bbc_output_t *awbBbc)
{
	awbBbc->white_pixel_count = ctempRegs->cnt_white_white;
	awbBbc->red_sum = ((uint64_t)ctempRegs->sumr_sum_h << 32) | ctempRegs->sumr_sum_l;
	awbBbc->green_sum = ((uint64_t)ctempRegs->sumg_sum_h << 32) | ctempRegs->sumg_sum_l;
	awbBbc->blue_sum = ((uint64_t)ctempRegs->sumb_sum_h << 32) | ctempRegs->sumb_sum_l;
	/* these are sums of u1.7 values so high register will always be zero for up to 8Mpix sensors */
	awbBbc->r_over_g_sum = ctempRegs->sumrg_sum_l;
	awbBbc->b_over_g_sum = ctempRegs->sumbg_sum_l;
}

void IPANxpNeo::convertCtempRegsToAwbCrois(const neoisp_ctemp_reg_stats_s *ctempRegs,
					   imx9x_isp_ctemp_color_rois_output_t *awbCrois)
{
	for (int roi = 0; roi < NEO_CTEMP_REG_STATS_CROIS_CNT; roi++) {
		awbCrois->rois[roi].pixel_count = ctempRegs->crois[roi].pixcnt_pixcnt;
		awbCrois->rois[roi].red_sum = ctempRegs->crois[roi].sumred_sum;
		awbCrois->rois[roi].green_sum = ctempRegs->crois[roi].sumgreen_sum;
		awbCrois->rois[roi].blue_sum = ctempRegs->crois[roi].sumblue_sum;
	}
}

void IPANxpNeo::prepareUguzziAecHistograms(const vpipe_settings_hw_t *cfg,
					   const neoisp_rgbir_mem_stats_s *rgbirHist,
					   const neoisp_hist_mem_stats_s *hist,
					   ae_histogram_data_hw_t *outHistData)
{
	const imx9x_isp_stat_cfg_t *statCfg = &cfg->isp_cfg_params[0].stat;
	const imx9x_isp_rgbir_stat_cfg_t *rgbirStatCfg = &cfg->isp_cfg_params[0].rgbir_stat;

	uint32_t statRoiWidth = statCfg->background.width;
	uint32_t statRoiHeight = statCfg->background.height;
	uint32_t rgbirRoiWidth = rgbirStatCfg->background.width;
	uint32_t rgbirRoiHeight = rgbirStatCfg->background.height;

	uint32_t statRoiChannels = statCfg->hists[0].channel_selection;
	uint32_t roiChannelsCnt = 0;
	uint32_t sumHistLong = 0, sumHistShort = 0, sumHistVeryShort = 0;
	uint32_t sumHistExpected;

	bool histChannelsCorrect =
		statCfg->hists[0].channel_selection != statCfg->hists[1].channel_selection ||
		statCfg->hists[1].channel_selection != statCfg->hists[2].channel_selection ||
		statCfg->hists[3].channel_selection != rgbirStatCfg->hists[0].channel_selection ||
		rgbirStatCfg->hists[0].channel_selection != rgbirStatCfg->hists[1].channel_selection;
	/* Check the channel configuration of the histograms */
	if (histChannelsCorrect) {
		/* Should not be happening, data corruption or misconfiguration occured. */
		LOG(NxpNeoUguzziIPA, Warning)
			<< "Mismatching histogram channel configuration";
	}

	bool mismatchingRois = false;
	/* Check the ROI configuration of the histograms */
	if (statRoiWidth != rgbirRoiWidth || statRoiHeight != rgbirRoiHeight) {
		/* Should not be happening, data corruption or misconfiguration occured.
		Print a warning and duplicate the STAT histograms. */
		LOG(NxpNeoUguzziIPA, Warning)
			<< "Mismatching RGBIR and STAT histograms ROI configuration. "
			<< "Discarding RGBIR histograms and using duplicate STAT histograms";
		mismatchingRois = true;
	}

	for (uint32_t i = 0; i < HIST_CHANNELS_CNT_MAX; i++)
		roiChannelsCnt += ((statRoiChannels >> i) & 1U);

	/* Accounting for RGBIR histograms */
	roiChannelsCnt *= 2U;

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

	for (uint32_t bin = 0U; bin < STAT_HIST_BINS; bin++) {
		sResultLongHist[bin] = *(hist0 + bin) + *(hist3 + bin);
		sResultShortHist[bin] = *(hist1 + bin) + *(hist4 + bin);
		sResultVshortHist[bin] = *(hist2 + bin) + *(hist5 + bin);

		sumHistLong += sResultLongHist[bin];
		sumHistShort += sResultShortHist[bin];
		sumHistVeryShort += sResultVshortHist[bin];
	}

	/*
	 * NOTE: Assuming that the stats ROI's width and height do not exceed
	 * the frame's width and height. (E.g for 1280x720 frame, the stats ROI's size
	 * should be at most 1280x720. And if X or Y offset is specified, the ROI's
	 * width and height will be decremented accordingly to fit the frame's size
	 */
	sumHistExpected = (statRoiWidth * statRoiHeight / HIST_CHANNELS_CNT_MAX) * roiChannelsCnt;

	if (sumHistLong != sumHistExpected) {
		LOG(NxpNeoUguzziIPA, Warning) << "Mismatching Long histogram sum. "
					      << "Expected: " << sumHistExpected
					      << ", current: " << sumHistLong;
	}
	if (sumHistShort != sumHistExpected) {
		LOG(NxpNeoUguzziIPA, Warning) << "Mismatching Short histogram sum. "
					      << "Expected: " << sumHistExpected
					      << ", current: " << sumHistShort;
	}
	if (sumHistVeryShort != sumHistExpected) {
		LOG(NxpNeoUguzziIPA, Warning) << "Mismatching VShort histogram sum. "
					      << "Expected: " << sumHistExpected
					      << ", current: " << sumHistVeryShort;
	}

	outHistData->Long = sResultLongHist;
	outHistData->Short = sResultShortHist;
	outHistData->VShort = sResultVshortHist;
	outHistData->bins = STAT_HIST_BINS;
	outHistData->sum = sumHistExpected;
}

void IPANxpNeo::prepareUguzziAwbStats(imx9x_isp_awb_stats_type_t statType,
				      const neoisp_ctemp_reg_stats_s *ctempRegs,
				      const neoisp_ctemp_mem_stats_s *ctempMems,
				      awb_statistics_data_hw_t *outAwbData)
{
	switch (statType) {
	case IMX9X_ISP_AWB_STATS_PAXELS:
		outAwbData->type = IMX9X_ISP_AWB_STATS_PAXELS;
		outAwbData->paxels_data.pixel_counts = ctempMems->ctemp_pix_cnt;
		outAwbData->paxels_data.red_sums = ctempMems->ctemp_r_sum;
		outAwbData->paxels_data.green_sums = ctempMems->ctemp_g_sum;
		outAwbData->paxels_data.blue_sums = ctempMems->ctemp_b_sum;
		break;

	case IMX9X_ISP_AWB_STATS_CROIS:
		outAwbData->type = IMX9X_ISP_AWB_STATS_CROIS;
		convertCtempRegsToAwbCrois(ctempRegs, &outAwbData->crois);
		break;

	case IMX9X_ISP_AWB_STATS_BBC:
		outAwbData->type = IMX9X_ISP_AWB_STATS_BBC;
		convertCtempRegsToAwbBbc(ctempRegs, &outAwbData->bbc);
		break;

	default:
		LOG(NxpNeoUguzziIPA, Error)
			<< "Unsupported AWB statistics type " << statType
			<< ". Using Block Statistics instead!";
		outAwbData->type = IMX9X_ISP_AWB_STATS_PAXELS;
		outAwbData->paxels_data.pixel_counts = ctempMems->ctemp_pix_cnt;
		outAwbData->paxels_data.red_sums = ctempMems->ctemp_r_sum;
		outAwbData->paxels_data.green_sums = ctempMems->ctemp_g_sum;
		outAwbData->paxels_data.blue_sums = ctempMems->ctemp_b_sum;
		break;
	}
}

void IPANxpNeo::prepareUguzziAfStats(const vpipe_settings_hw_t *cfg,
				     const neoisp_af_reg_stats_s *afRegs,
				     const neoisp_drc_mem_stats_s *drcMems,
				     af_statistics_data_hw_t *outAfData)
{
	/* Populate CDAF statistics. */
	uint32_t *filter0Sums = &outAfData->af_rois_stat.filter0_sums[0];
	uint32_t *filter1Sums = &outAfData->af_rois_stat.filter1_sums[0];
	for (unsigned int i = 0; i < AUTOFOCUS_ROI_CNT; i++) {
		filter0Sums[i] = afRegs->rois[i].sum0;
		filter1Sums[i] = afRegs->rois[i].sum1;
	}

	/* Report AF ROI definitions. */
	const imx9x_isp_autofocus_roi_cfg_t *roisConfig =
		&cfg->isp_cfg_params[0].autofocus.rois_config[0];
	imx9x_isp_autofocus_roi_cfg_t *roisOut = &outAfData->rois_config[0];
	std::copy(roisConfig, roisConfig + AUTOFOCUS_ROI_CNT, roisOut);

	/* Populate DRC local 32x32 grid configuration and statistics. */
	outAfData->block_stats = drcMems->drc_local_sum;
	const imx9x_isp_drc_local_tonemap_ctrl_cfg_t *localDrcConfig =
		 &cfg->isp_cfg_params[0].drc_local_tonemap_ctrl;
	outAfData->block_width = localDrcConfig->block_size_x;
	outAfData->block_height = localDrcConfig->block_size_y;
	outAfData->block_cnt_horz = 32;
	outAfData->block_cnt_vert = 32;
}

void IPANxpNeo::prepareUguzziStats(const NxpNeoStats *stats)
{
	auto rgbirStats = stats->block<BlockStatsType::MRgbIr>();
	auto histStats = stats->block<BlockStatsType::MHist>();
	prepareUguzziAecHistograms(&mIspSettings[channel_],
				   rgbirStats.stats(),
				   histStats.stats(),
				   &mAeHistData[channel_]);

	auto ctempRegStats = stats->block<BlockStatsType::RCTemp>();
	auto ctempMemStats = stats->block<BlockStatsType::MCTemp>();
	prepareUguzziAwbStats(mAwbStatType[channel_],
			      ctempRegStats.stats(),
			      ctempMemStats.stats(),
			      &mAwbStatsData[channel_]);

	auto drcMemStats = stats->block<BlockStatsType::MDrc>();
	mAgblbceStats[channel_].global_hist_roi0 = drcMemStats->drc_global_hist_roi0;
	mAgblbceStats[channel_].global_hist_roi1 = drcMemStats->drc_global_hist_roi1;
	mAgblbceStats[channel_].local_stats = drcMemStats->drc_local_sum;

	auto afStats = stats->block<BlockStatsType::RAf>();
	prepareUguzziAfStats(&mIspSettings[channel_],
			     afStats.stats(),
			     drcMemStats.stats(),
			     &mAfStatsData[channel_]);

	mStatsDataPkg.channel[channel_].stats_type = HW_STATS;
	mStatsDataPkg.channel[channel_].valid = 1;
}

int IPANxpNeo::processUguzzi(uguzzi_sensor_data_pkg_t *sensorDataPkg,
			     uguzzi_stats_data_pkg_t *statsDataPkg,
			     uguzzi_sensor_settings_pkg_t *sensorSettingsPkg,
			     uguzzi_isp_settings_pkg_t *ispSettingsPkg)
{
	int err = uguzzi_process(sensorDataPkg, statsDataPkg, sensorSettingsPkg, ispSettingsPkg);
	if (!err) {
		const imx9x_isp_cfg_prms_t *ispCfg =
			&ispSettingsPkg->isp_config[channel_]->isp_cfg_params[0];

		if (ispCfg->update[CTEMP_CFG]) {
			mAwbStatType[channel_] = static_cast<imx9x_isp_awb_stats_type_t>(
				ispCfg->ctemp.ctrl.awb_stat_type);
		}
	}

	return err;
}

#ifdef USE_LIVE_CONTROL
void IPANxpNeo::processLiveControl(const NxpNeoStats *stats)
{
	/* \todo Use it when embedded data is not part of the RAW image */
	EmbeddedDataPkg embDataPkg = {};
	ImageBufferViewSetPkg imgBuffViewSetPkg = {};
	IspStatisticsPkg ispStatPkg = {};
	LiveControl &liveCtrl = LiveControl::getInstance();

	if (buffers_.count(rawImage0BufferId_)) {
		const MappedBuffer::Plane &rawBufferPlane =
			buffers_.at(rawImage0BufferId_).planes()[0];

		imgBuffViewSetPkg.channel[channel_].view[IMAGE_BUFFER_DCG].data =
			static_cast<const void *>(rawBufferPlane.data());
		imgBuffViewSetPkg.channel[channel_].view[IMAGE_BUFFER_DCG].size =
			static_cast<uint32_t>(rawBufferPlane.size_bytes());
	}

	if (buffers_.count(rawImage1BufferId_)) {
		const MappedBuffer::Plane &rawBufferPlane =
			buffers_.at(rawImage1BufferId_).planes()[0];

		imgBuffViewSetPkg.channel[channel_].view[IMAGE_BUFFER_VS].data =
			static_cast<const void *>(rawBufferPlane.data());
		imgBuffViewSetPkg.channel[channel_].view[IMAGE_BUFFER_VS].size =
			static_cast<uint32_t>(rawBufferPlane.size_bytes());
	}

	ispStatPkg.channel[channel_] = stats;

	int err = liveCtrl.handleCmd(&mSensorDataPkg,
				     &embDataPkg,
				     &imgBuffViewSetPkg,
				     &ispStatPkg,
				     LIVE_CONTROL_CMD_HANDLE_TIMEOUT_MS);
	if (err)
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to handle Live Control command - err=["
			<< err << "]";

	return;
}
#endif

void IPANxpNeo::setControls(unsigned int frame, IPAContextType context)
{
	ControlList ctrls(sensorControls_);

	/* \todo what about harmonized cameras? How to set controls per camera? */
	/* \todo how to obtain conversion gain from OTP sensor registers? */
	uguzzi_sensor_settings_t *settings = mSensorSettingsPkg.channel[channel_];

	uint32_t aGainQ16 = settings->exp_n.exp.again;
	uint32_t dGainQ16 = settings->exp_n.exp.dgain;
	uint64_t gainQ16 = static_cast<uint64_t>(aGainQ16) * dGainQ16 / UQ16_1;
	double gain = static_cast<double>(gainQ16) / UQ16_1;

	Duration exposure = settings->exp_n.exp.exposure * 1.0us;
	camHelper_->controlListSetAGC(&ctrls, kSensorContextMap.at(context),
				      exposure, gain);

	if ((wbLocation_[channel_] == UGUZZI_CAM_INFO_WB_LOCATION_ISP) && (!frame)) {
		/* WB in ISP, set sensor WB gains to 1.0 only for 1st frame */
		/*
		 * uGuzzi issue is that for UGUZZI_CAM_INFO_WB_LOCATION_ISP,
		 * settings->wb.wb_gains has incorrect value,
		 * hence hard coded values 1.0 are used instead.
		 */
		std::array<double, 4> wbGains = { 1.0, 1.0, 1.0, 1.0 };
		camHelper_->controlListSetAWB(&ctrls, Span<const double, 4>(wbGains));

	} else if (wbLocation_[channel_] == UGUZZI_CAM_INFO_WB_LOCATION_SENSOR) {
		/* WB in sensor */
		std::array<double, 4> wbGains;
		/* R, Gr, Gb, B */
		wbGains[0] = static_cast<double>(settings->wb.wb_gains.red) / UQ8_1;
		wbGains[1] = static_cast<double>(settings->wb.wb_gains.green) / UQ8_1;
		wbGains[2] = wbGains[1];
		wbGains[3] = static_cast<double>(settings->wb.wb_gains.blue) / UQ8_1;

		camHelper_->controlListSetAWB(&ctrls, Span<const double, 4>(wbGains));
	}

	LOG(NxpNeoUguzziIPA, Debug) << logSensorParams(frame, &mdControls_, &ctrls);

	/*
	 * In RGBIr dual mode, the controls should be sent for one context only:
	 * - the RGB context should be used as long as the single-capture
	 *   controls are used from the CameraHelper in RGBIr dual mode.
	 */
	if (pipelineMode_ != IPAModeTypeRgbIrDual ||
	    (pipelineMode_ == IPAModeTypeRgbIrDual &&
	     context == IPAContextTypeRgb)) {
		setSensorControls.emit(frame, ctrls);
	}

	if (!lensPresent_)
		return;

	if (!lensHwPosition_ || lensHwPosition_.value() != settings->lens_pos) {
		lensHwPosition_ = settings->lens_pos;
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
	apiVersion_ = params.apiVersion;

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
				    << " on hardware revision " << params.hwRevision
				    << " - Sensor: " << sensorEntity_;

	camHelper_ = CameraHelperFactoryBase::create(sensorModel_);
	if (!camHelper_) {
		LOG(NxpNeoUguzziIPA, Error)
			<< "Failed to create camera helper for "
			<< sensorModel_;
		return -ENODEV;
	}

	/* \note in case multiple harmonized cameras are to be supported in the future */
	if (cameraCnt_ > UGUZZI_CAMERA_CNT) {
		LOG(NxpNeoUguzziIPA, Error)
			<< cameraCnt_ << " cameras requested but only "
			<< UGUZZI_CAMERA_CNT << " are supported";
		return -EINVAL;
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
	sensorConfig->rgbIr = attributes->rgbIr;

#ifdef USE_LIVE_CONTROL
	LiveControl &liveCtrl = LiveControl::getInstance();
	const uint16_t socketPort = config_.socketPort(sensorModel_, sensorEntity_);
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

	/* Set the camera helper with sensor control values. */
	camHelper_->setControls(&params.sensorControlList);

	lensPresent_ = params.lensPresent;

	/* Set the IPA initialization state flag to enabled */
	enabled_ = true;

	return 0;
}

int IPANxpNeo::start()
{
	int err = getUguzziInitialSettings();
	if (err)
		return err;

	setControls(0, IPAContextTypeRgb);

	return 0;
}

void IPANxpNeo::stop()
{
}

int IPANxpNeo::configure(const IPAConfigInfo &ipaConfig,
			 const std::map<uint32_t, IPAStream> &streamConfig,
			 ControlInfoMap *ipaControls)
{
	int ret = 0;
	const IPACameraSensorInfo *sensorInfo = &ipaConfig.sensorInfo;
	(void)ipaControls;

	if (!enabled_) {
		LOG(NxpNeoUguzziIPA, Error) << "IPA for sensor "
					    << sensorEntity_ << " is not initialized";
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

	pipelineMode_ = ipaConfig.mode;

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
					      << sensorInfo->bitsPerPixel << "bpp; mode:"
					      << ipaConfig.mode << "]";
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

#ifdef UGUZZI_TESTS_ENABLED
	/*
	 * When adapting the IPA for supporting multicameras with 1 single
	 * uGuzzi instance, this initializeTestsUguzzi() should be called
	 * part of the proxy operating the single instance.
	 */
	if (initializeTestsUguzzi(Span<uint32_t>(IPANxpNeo::camFrames),
				  cameraCnt_,
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
	 * hence, live connect should be initialized with uguzzi_live_tuning_init()
	 * once uGuzzi is initialized.
	 */
	ret = liveCtrl.init(camInfoDtp_[channel_]->frame1_cfg.width,
			    camInfoDtp_[channel_]->frame1_cfg.height);
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
			<< ipaConfig.mode
			<< " - Default mode is used: " << cameraMode.streamMode;
	}
	camHelper_->setCameraMode(cameraMode);

	sensorControls_ = ipaConfig.sensorControls;
	lensControls_ = ipaConfig.lensControls;
	sensorInfo_ = ipaConfig.sensorInfo;

	return 0;
}

void IPANxpNeo::mapBuffers(const std::vector<IPABuffer> &buffers)
{
	for (const IPABuffer &buffer : buffers) {
		const FrameBuffer fb(buffer.planes);
		buffers_.emplace(buffer.id,
				 MappedFrameBuffer(&fb, MappedFrameBuffer::MapFlag::ReadWrite));
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
}

void IPANxpNeo::computeParams(const uint32_t frame, const IPAContextType context,
			      const std::map<uint32_t, uint32_t> &bufferIds)
{
	ControlList &controls = mdControls_;
	controls = ControlList(md::controlIdMap);

	/* update the frame id (only cam=0 is supported for now) */
	camFrames[channel_] = frame;

	uint8_t *metaData = nullptr;
	size_t metaSize = 0;
	metaDataValid_ = false;

	/* Give access to raw buffers for live tuning */
	auto input0It = bufferIds.find(IPABufferTypeImage0);
	rawImage0BufferId_ = input0It != bufferIds.end() ? input0It->second : 0;
	auto input1It = bufferIds.find(IPABufferTypeImage1);
	rawImage1BufferId_ = input1It != bufferIds.end() ? input1It->second : 0;

	/*
	 * Look for metadata availability, either from the camera embedded data
	 * stream or from the pixel data top lines.
	 */
	auto eDataIt = bufferIds.find(IPABufferTypeEData);
	unsigned int eDataBufferId =
		eDataIt != bufferIds.end() ? eDataIt->second : 0;
	if (eDataBufferId && buffers_.count(eDataBufferId)) {
		const MappedBuffer::Plane &plane =
			buffers_.at(eDataBufferId).planes()[0];
		metaData = plane.data();
		metaSize = plane.size_bytes();
	} else if (rawImage0BufferId_ && buffers_.count(rawImage0BufferId_)) {
		/* For now, embedded data is only supported from the raw Image0. */
		const MappedBuffer::Plane &plane =
			buffers_.at(rawImage0BufferId_).planes()[0];
		metaData = plane.data();
		uint32_t topLines = camHelper_->attributes()->mdParams.topLines;
		unsigned int bpp = sensorInfo_.bitsPerPixel;
		size_t bytepp = bpp <= 8 ? sizeof(uint8_t) : sizeof(uint16_t);
		unsigned int width = sensorInfo_.outputSize.width;
		metaSize = topLines * width * bytepp;
	}

	if (metaSize) {
		Span<uint8_t> mdBuffer(metaData, metaSize);
		if (!camHelper_->parseEmbedded(mdBuffer, &controls))
			metaDataValid_ = true;
	}

	auto paramsIter = bufferIds.find(IPABufferTypeParams);
	unsigned int paramsBufferId =
		paramsIter != bufferIds.end() ? paramsIter->second : 0;
	ASSERT(buffers_.count(paramsBufferId));
	NxpNeoParams params(apiVersion_,
			    buffers_.at(paramsBufferId).planes()[0]);

	convertUguzziIspCfg2IspDrvCfg(&mIspSettingsPkg.isp_config[channel_]->isp_cfg_params[0],
				      mSensorDataPkg.channel[channel_].l2vs_ratio,
				      &params);

	paramsComputed.emit(frame, context, params.size());
}

void IPANxpNeo::processStats(const uint32_t frame, const IPAContextType context,
			     const std::map<uint32_t, uint32_t> &bufferIds,
			     const ControlList &sensorControls)
{
	auto statsIter = bufferIds.find(IPABufferTypeStats);
	unsigned int statsBufferId =
		statsIter != bufferIds.end() ? statsIter->second : 0;
	ASSERT(buffers_.count(statsBufferId));

	const NxpNeoStats stats(apiVersion_,
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

	uguzzi_sensor_data_t *sensorData = &mSensorDataPkg.channel[channel_];
	metaDataToSensorData(&controls, sensorData);

	prepareUguzziSensorData();

	prepareUguzziStats(&stats);

	int err = processUguzzi(&mSensorDataPkg,
				&mStatsDataPkg,
				&mSensorSettingsPkg,
				&mIspSettingsPkg);
	if (err)
		LOG(NxpNeoUguzziIPA, Error) << "Failed to process ISP statistics";

	ControlList metadata(controls::controls);
	if (context == IPAContextTypeRgb) {
		/*
		 * Metadata are only filled in RGB context.
		 * This is to avoid overwritten the same control in Ir context.
		 */
		metadata.set(controls::Lux,
			     static_cast<float>(mIspSettingsPkg.uguzzi_metadata[channel_].aec_info[22]));
		metadata.set(controls::ColourTemperature,
			     mSensorSettingsPkg.channel[channel_]->wb.colour_temp);
		/* add more as needed */
	}

	setControls(frame, context);

#ifdef USE_LIVE_CONTROL
	processLiveControl(&stats);
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

	std::array<float, UGUZZI_WDR3_ENTRY_MAX> aGainArray = { 1.0f, 1.0f, 1.0f };
	Span<const float> aGain = Span<float>(aGainArray);
	if (mdCtrls->contains(md::AnalogueGain.id())) {
		const ControlValue &aGainCtrl = mdCtrls->get(md::AnalogueGain.id());
		Span<const float> aGainValue = aGainCtrl.get<Span<const float>>();
		assert(aGainValue.size() <= UGUZZI_WDR3_ENTRY_MAX);
		std::copy(aGainValue.begin(), aGainValue.end(), aGainArray.begin());
	} else {
		LOG(NxpNeoUguzziIPA, Warning) << "No analog gain metadata";
		mdValid = false;
	}

	std::array<float, UGUZZI_WDR3_ENTRY_MAX> dGainArray = { 1.0f, 1.0f, 1.0f };
	Span<const float> dGain = Span<float>(dGainArray);
	if (mdCtrls->contains(md::DigitalGain.id())) {
		const ControlValue &dGainCtrl = mdCtrls->get(md::DigitalGain.id());
		Span<const float> dGainValue = dGainCtrl.get<Span<const float>>();
		assert(dGainValue.size() <= UGUZZI_WDR3_ENTRY_MAX);
		std::copy(dGainValue.begin(), dGainValue.end(), dGainArray.begin());
	} else {
		LOG(NxpNeoUguzziIPA, Warning) << "No digital gain metadata";
		mdValid = false;
	}

	std::array<float, UGUZZI_WDR3_ENTRY_MAX> exposureArray = { 1.0f, 0.0f, 0.0f };
	Span<const float> exposure = Span<float>(exposureArray);
	if (mdCtrls->contains(md::Exposure.id())) {
		const ControlValue &exposureCtrl = mdCtrls->get(md::Exposure.id());
		Span<const float> exposureValue = exposureCtrl.get<Span<const float>>();
		assert(exposureValue.size() <= UGUZZI_WDR3_ENTRY_MAX);
		std::copy(exposureValue.begin(), exposureValue.end(), exposureArray.begin());
		mdMultiCapture = (exposureValue.size() == UGUZZI_WDR3_ENTRY_MAX);
	} else {
		LOG(NxpNeoUguzziIPA, Warning) << "No exposure metadata";
		mdValid = false;
	}

	std::array<float, 4> wbGainArray = { 1.0f, 1.0f, 1.0f, 1.0f };
	Span<const float> wbGain = Span<float>(wbGainArray);
	if (mdCtrls->contains(md::WhiteBalanceGain.id())) {
		const ControlValue &wbGainCtrl = mdCtrls->get(md::WhiteBalanceGain.id());
		Span<const float> wbGainValue = wbGainCtrl.get<Span<const float>>();
		assert(wbGainValue.size() == 4);
		std::copy(wbGainValue.begin(), wbGainValue.end(), wbGainArray.begin());
	} else {
		LOG(NxpNeoUguzziIPA, Warning) << "No wb gain metadata";
		mdValid = false;
	}

	float temperature = 25.0;
	if (mdCtrls->contains(md::Temperature.id())) {
		const ControlValue &tempGainCtrl = mdCtrls->get(md::Temperature.id());
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
			LOG(NxpNeoUguzziIPA, Warning) << "Short total exposure is 0";
		}

		uint64_t totalVS =
			static_cast<uint64_t>(exposureVS->exposure) *
			static_cast<uint64_t>(exposureVS->again);
		if (!totalVS) {
			totalVS = 1U;
			mdValid = false;
			LOG(NxpNeoUguzziIPA, Warning) << "Very Short total exposure is 0";
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

std::string IPANxpNeo::logSensorParams(const unsigned int frame,
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
