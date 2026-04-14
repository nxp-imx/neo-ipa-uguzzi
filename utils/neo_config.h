/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright 2025-2026 NXP
 *
 * Configuration helpers for IPA
 */

#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include <libcamera/geometry.h>

#include <libcamera/ipa/nxpneo_ipa_interface.h>

#include "libcamera/internal/yaml_parser.h"

namespace libcamera::ipa::nxpneo {

struct TuningInfo {
	Size resolution;
	unsigned int bitDepth;
	IPAPipelineMode mode;
	std::string dtpFile;
	std::vector<uint32_t> tuningId;
	uint32_t tuningMode;
};

using SensorMap = std::map<std::string, std::vector<TuningInfo>>;
using SocketMap = std::map<std::string, uint16_t>;

class IPAFileConfig
{
public:
	IPAFileConfig() {};
	virtual ~IPAFileConfig() {};
	int load(const std::string &filename);
	const TuningInfo *tuningInfo(const std::string &model,
				     const std::string &entity,
				     Size resolution,
				     unsigned int bitDepth,
				     IPAPipelineMode mode) const;
	uint16_t socketPort(const std::string &model,
			    const std::string &entity) const;
	const std::optional<std::string> &sensorFilter() const { return sensorFilter_; }
	bool overrideInAlign() const { return overrideInAlign_; }

private:
	static constexpr uint16_t kSocketPort = 50000;
	static constexpr unsigned int kBitDepth = 16;
	static constexpr uint32_t kTuningIdRgb = 2010;
	static constexpr uint32_t kTuningIdIr = 2011;
	static constexpr uint32_t kTuningMode = 0;

	int parseSensors(const YamlObject &sensors);
	int parseSensorProfiles(const YamlObject &profiles,
				const std::string &sensor);
	int parseEntityFilter(const YamlObject &entity);
	int parseOverrideInAlign(const YamlObject &overrideInAlign);

	SensorMap sensorMap_;
	std::optional<std::string> sensorFilter_;
	bool overrideInAlign_ = true;
	SocketMap socketMap_;
	/*
	 * Map between the pipeline mode name and its associated IPA pipeline
	 * mode enum value.
	 */
	static const std::map<std::string, IPAPipelineMode> kPipelineModeNameMap;
};

} /* namespace libcamera::ipa::nxpneo */
