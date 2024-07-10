/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * run_tests.cpp - NXP NEO uGuzzi IPA tests
 * Copyright 2024 NXP
 */

#include "run_tests.h"

#include <errno.h>
#include <map>

#include <libcamera/base/log.h>

extern "C" {
#include "uguzzi/uguzzi.h"
}

namespace libcamera::ipa::nxpneo {

LOG_DEFINE_CATEGORY(NxpNeoUguzziIPATests)

#define AEC_TESTS_NUM (3u)
#define AEC_TESTS_STEP (2000)
/*
 * This scaling factor is applied to the reported gain value
 * to be in the same range value as the exposure values.
 */
#define AEC_TESTS_GAIN_SCALE (1000)
/* \todo move into context params */
#define AEC_TESTS_RUNS_CNT (64u)
#define AEC_TESTS_RUNS_CNT_ALL (AEC_TESTS_RUNS_CNT * AEC_TESTS_NUM)

struct aecTestsParams {
	uint32_t exposure;
	uint32_t analogGain;
};

struct testsContext {
	uint8_t cameraCnt;
	Span<uint32_t> frameId;
	aecTestsParams *paramsAec;
	/*
	 * \todo: following params could be configured
	 * from user using a config file
	 */
	bool enableTests;
	bool enableLogs;
};

static const aecTestsParams ox03c10AecParams[AEC_TESTS_NUM] = {
	/* test0: gain and exposure variation, total exposure is constant */
	{ 28000, 32000 },
	/* test1: gain constant, exposure varies with slope */
	{ 4000, 32000 },
	/* test2: exposure constant, gain varies with slope */
	{ 10000, 30000 },
};

static testsContext context;

static const std::map<std::string, const aecTestsParams *> sensorParams = {
	{ "mx95mbcam", ox03c10AecParams },
};

static void initAecTestParams(uint32_t *expTime, uint32_t *analogGain);
static void setAecVerificationTest(uguzzi_sensor_settings_pkg_t *sensor_settings_pkg,
				   uguzzi_isp_settings_pkg_t *isp_settings_pkg);
static void logUguzziInput(uint32_t channel,
			   uguzzi_sensor_data_pkg_t *sensor_data_pkg,
			   uguzzi_stats_data_pkg_t *stats_data_pkg);
static void logUguzziOutput(uint32_t channel,
			    uguzzi_sensor_settings_pkg_t *sensor_settings_pkg,
			    uguzzi_isp_settings_pkg_t *isp_settings_pkg);
static void logAecUguzziInput(uint32_t channel,
			      uguzzi_sensor_data_pkg_t *sensor_data_pkg);
static void logAecUguzziOutput(uint32_t channel, uint32_t expTime,
			       uint32_t analogGain, uint32_t brightness);
static int uguzziProcBeginCb(uguzzi_sensor_data_pkg_t *sensor_data_pkg,
			     uguzzi_stats_data_pkg_t *stats_data_pkg);
static int uguzziProcEndCb(uguzzi_sensor_settings_pkg_t *sensor_settings_pkg,
			   uguzzi_isp_settings_pkg_t *isp_settings_pkg);
static int initTestsParams(const std::string &sensorModel);

static void initAecTestParams(uint32_t *expTime, uint32_t *analogGain)
{
	static uint8_t testCount = 0;
	static int8_t slopeDir = 1;
	static uint32_t runs = 1;
	uint32_t runAecTestNo = ((runs - 1) / AEC_TESTS_RUNS_CNT);
	aecTestsParams *params = context.paramsAec;

	LOG(NxpNeoUguzziIPATests, Info) << "aecTestNo[" << runAecTestNo
					<< "] - runs[" << runs << "]";
	if (!((runs - 1) % AEC_TESTS_RUNS_CNT)) {
		/* reset values when starting a new serie of runs */
		testCount = 0;
		slopeDir = 1;
	}
	switch (runAecTestNo) {
	case 0:
		*expTime = params[runAecTestNo].exposure >> testCount;
		*analogGain = params[runAecTestNo].analogGain << testCount;
		testCount = (testCount < 2) ? (testCount + 1) : 0;
		break;

	case 1:
		if (testCount >= 12)
			slopeDir = -1;
		else if (testCount == 1)
			slopeDir = 1;
		testCount += slopeDir;
		*analogGain = params[runAecTestNo].analogGain;
		*expTime = params[runAecTestNo].exposure + testCount * AEC_TESTS_STEP;
		break;

	case 2:
		if (testCount >= 12)
			slopeDir = -1;
		else if (testCount == 1)
			slopeDir = 1;
		testCount += slopeDir;
		*expTime = params[runAecTestNo].exposure;
		*analogGain = params[runAecTestNo].analogGain + testCount * AEC_TESTS_STEP;
		break;
	}
	/* increment runs */
	runs = (runs < AEC_TESTS_RUNS_CNT_ALL) ? (runs + 1) : 1;
}

static void setAecVerificationTest(uguzzi_sensor_settings_pkg_t *sensor_settings_pkg,
				   uguzzi_isp_settings_pkg_t *isp_settings_pkg)
{
	uint32_t expTime = 0;
	uint32_t analogGain = 0;

	initAecTestParams(&expTime, &analogGain);

	for (uint32_t chnlIdx = 0U; chnlIdx < context.cameraCnt; chnlIdx++) {
		sensor_settings_pkg->channel[chnlIdx]->exp_n.exp.exposure = expTime;
		sensor_settings_pkg->channel[chnlIdx]->exp_n.exp.again =
			(uint32_t)(((uint64_t)analogGain * 65536) / AEC_TESTS_GAIN_SCALE);
		logAecUguzziOutput(chnlIdx, expTime, analogGain,
				   isp_settings_pkg->uguzzi_metadata[chnlIdx].aec_info[0]);
	}
}

static void logUguzziInput(uint32_t channel,
			   uguzzi_sensor_data_pkg_t *sensor_data_pkg,
			   uguzzi_stats_data_pkg_t *stats_data_pkg)
{
	(void)stats_data_pkg;
	if (!sensor_data_pkg) {
		LOG(NxpNeoUguzziIPATests, Error) << "sensor_data_pkg is NULL";
		return;
	}

	uguzzi_exposure_t *exp_gain = sensor_data_pkg->channel[channel].exp_gain;
	LOG(NxpNeoUguzziIPATests, Info) << "channel[" << channel
					<< "] frame[" << context.frameId[channel]
					<< "] uGuzziSensorData exposure/again/dgain[L/S/VS]=["
					<< exp_gain[UGUZZI_WDR3_ENTRY_LONG].exposure
					<< ", "
					<< exp_gain[UGUZZI_WDR3_ENTRY_SHORT].exposure
					<< ", "
					<< exp_gain[UGUZZI_WDR3_ENTRY_VERY_SHORT].exposure
					<< "]/["
					<< exp_gain[UGUZZI_WDR3_ENTRY_LONG].again
					<< ", "
					<< exp_gain[UGUZZI_WDR3_ENTRY_SHORT].again
					<< ", "
					<< exp_gain[UGUZZI_WDR3_ENTRY_VERY_SHORT].again
					<< "]/["
					<< exp_gain[UGUZZI_WDR3_ENTRY_LONG].dgain
					<< ", "
					<< exp_gain[UGUZZI_WDR3_ENTRY_SHORT].dgain
					<< ", "
					<< exp_gain[UGUZZI_WDR3_ENTRY_VERY_SHORT].dgain
					<< "]";
	return;
}

static void logUguzziOutput(uint32_t channel,
			    uguzzi_sensor_settings_pkg_t *sensor_settings_pkg,
			    uguzzi_isp_settings_pkg_t *isp_settings_pkg)
{
	(void)isp_settings_pkg;

	uguzzi_sensor_settings_t *sensorSettings = sensor_settings_pkg->channel[channel];
	LOG(NxpNeoUguzziIPATests, Info) << "channel[" << channel
					<< "] frame[" << context.frameId[channel]
					<< "] uGuzziSensorSettings [exposure/dgain/again]=["
					<< sensorSettings->exp_n.exp.exposure << ", "
					<< sensorSettings->exp_n.exp.dgain << ", "
					<< sensorSettings->exp_n.exp.again
					<< "] - [Ylvl]=["
					<< isp_settings_pkg->uguzzi_metadata[0].aec_info[0]
					<< "]";

	return;
}

static void logAecUguzziInput(uint32_t channel,
			      uguzzi_sensor_data_pkg_t *sensor_data_pkg)
{
	uguzzi_exposure_t *exp_gain = sensor_data_pkg->channel[channel].exp_gain;
	uint32_t sensorExposure = exp_gain[UGUZZI_WDR3_ENTRY_LONG].exposure;
	uint32_t sensorGain = (uint32_t)(((uint64_t)exp_gain[UGUZZI_WDR3_ENTRY_LONG].again * AEC_TESTS_GAIN_SCALE) / 65536);
	if (!sensor_data_pkg) {
		LOG(NxpNeoUguzziIPATests, Error) << "sensor_data_pkg is NULL";
		return;
	}
	LOG(NxpNeoUguzziIPATests, Info) << "channel[" << channel
					<< "] frame[" << context.frameId[channel]
					<< "] sensor exposure/again=["
					<< sensorExposure
					<< ", "
					<< sensorGain
					<< "] totalExposure=["
					<< sensorExposure * sensorGain
					<< "]";
	return;
}

static void logAecUguzziOutput(uint32_t channel, uint32_t expTime,
			       uint32_t analogGain, uint32_t brightness)
{
	LOG(NxpNeoUguzziIPATests, Info) << "channel[" << channel
					<< "] frame[" << context.frameId[channel]
					<< "] settings exposure/again=["
					<< expTime << ", "
					<< analogGain
					<< "] totalExposure=["
					<< (expTime * analogGain)
					<< "] Ylvl=["
					<< brightness
					<< "]";

	return;
}

static int uguzziProcBeginCb(uguzzi_sensor_data_pkg_t *sensor_data_pkg,
			     uguzzi_stats_data_pkg_t *stats_data_pkg)
{
	(void)stats_data_pkg;
	if (!sensor_data_pkg) {
		LOG(NxpNeoUguzziIPATests, Error) << "sensor_data_pkg is NULL";
		return 0;
	}

	if (context.enableTests) {
		for (uint32_t chnlIdx = 0U; chnlIdx < context.cameraCnt; chnlIdx++) {
			logAecUguzziInput(chnlIdx, sensor_data_pkg);
		}
	}

	if (context.enableLogs) {
		for (uint32_t chnlIdx = 0U; chnlIdx < context.cameraCnt; chnlIdx++) {
			logUguzziInput(chnlIdx, sensor_data_pkg, stats_data_pkg);
		}
	}

	return 0;
}

static int uguzziProcEndCb(uguzzi_sensor_settings_pkg_t *sensor_settings_pkg,
			   uguzzi_isp_settings_pkg_t *isp_settings_pkg)
{
	(void)isp_settings_pkg;

	if (context.enableTests) {
		/* AEC verification tests */
		setAecVerificationTest(sensor_settings_pkg, isp_settings_pkg);
		/* \todo: add uGuzzi converter tests */
	}

	if (context.enableLogs) {
		for (uint32_t chnlIdx = 0U; chnlIdx < context.cameraCnt; chnlIdx++) {
			logUguzziOutput(chnlIdx,
					sensor_settings_pkg, isp_settings_pkg);
		}
	}

	return 0;
}

static int initTestsParams(const std::string &sensorModel)
{
	auto it = sensorParams.find(sensorModel);
	if (it != sensorParams.end()) {
		LOG(NxpNeoUguzziIPATests, Info) << "Initialize tests for sensor ["
						<< it->first << "]";
		context.paramsAec = const_cast<aecTestsParams *>(it->second);
	} else {
		LOG(NxpNeoUguzziIPATests, Error) << "Sensor ["
						 << sensorModel
						 << "] is not supported for test";
		return -EINVAL;
	}

	return 0;
}

int initializeTestsUguzzi(Span<uint32_t> camFrames,
			  uint8_t cameraCnt,
			  std::string sensorModel)
{
	int ret;

	/* Initialize tests context */
	context.frameId = Span<uint32_t>(camFrames);
	context.cameraCnt = cameraCnt;
	/* \todo: consider to set enableTests and enableLogs from a config file */
	context.enableTests = true;
	context.enableLogs = false;

	/* Initialize tests params */
	ret = initTestsParams(sensorModel);

	/*
	 * Register the uGuzzi callbacks invoked at the beginning and
	 * at the end of uguzzi process
	 */
	uguzzi_set_process_begin_callback(uguzziProcBeginCb);
	uguzzi_set_process_end_callback(uguzziProcEndCb);

	return ret;
}

} /* namespace libcamera::ipa::nxpneo */
