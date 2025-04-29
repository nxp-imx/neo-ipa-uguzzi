/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * neo-config.cpp - Configuration helpers for IPA
 * Copyright 2025 NXP
 */

#include "neo_config.h"

#include <libcamera/base/file.h>
#include <libcamera/base/log.h>

namespace libcamera::ipa::nxpneo {

LOG_DEFINE_CATEGORY(NxpNeoUguzziConfig)

/**
 * \brief Load the IPA configuration from a configuration file
 * \param[in] filename The path to configuration file
 * \return 0 on success or a negative error code otherwise
 */
int IPAFileConfig::load(const std::string &filename)
{
	File file(filename);
	int ret = 0;

	if (!file.open(File::OpenModeFlag::ReadOnly)) {
		ret = file.error();
		LOG(NxpNeoUguzziConfig, Error)
			<< "Failed to open YAML configuration file : "
			<< strerror(-ret);
		return ret;
	}

	std::unique_ptr<libcamera::YamlObject> root = YamlParser::parse(file);
	if (!root) {
		LOG(NxpNeoUguzziConfig, Error) << "Failed to parse configuration file";
		return -EINVAL;
	}

	unsigned int version = (*root)["version"].get<uint32_t>(0);
	if (version != 1) {
		LOG(NxpNeoUguzziConfig, Error)
			<< "Invalid configuration file version " << version;
		return -EINVAL;
	}

	/* Parse the sensors section */
	if (!root->contains("sensors")) {
		LOG(NxpNeoUguzziConfig, Error)
			<< "Configuration file doesn't contain any sensors section";
		return -EINVAL;
	}
	const YamlObject &sensors = (*root)["sensors"];
	ret = parseSensors(sensors);
	if (ret)
		LOG(NxpNeoUguzziConfig, Warning)
			<< "Invalid sensors section in config file";

	/* Parse the optional entity-filter section */
	const YamlObject &entityFilter = (*root)["entity-filter"];
	ret = parseEntityFilter(entityFilter);
	if (ret)
		LOG(NxpNeoUguzziConfig, Warning)
			<< "Invalid entity filter section in config file";

	return ret;
}

/**
 * \brief Parse the sensors section in the yaml configuration file
 * \param[in] sensors The sensors node in yaml file
 * \return 0 if no error was detected, a negative error code otherwise
 */
int IPAFileConfig::parseSensors(const YamlObject &sensors)
{
	const auto &list = sensors.asList();
	for (const auto &[i, sensorName] : utils::enumerate(list)) {
		if (!sensorName.isDictionary()) {
			LOG(NxpNeoUguzziConfig, Error)
				<< "Invalid YAML syntax for sensors " << i;
			return -EINVAL;
		}
		const auto &entity = sensorName["entity"].get<std::string>();
		if (!entity.has_value()) {
			LOG(NxpNeoUguzziConfig, Error) << "Missing camera entity name";
			return -EINVAL;
		}
		/* Parse the socket port */
		const YamlObject &portObj = sensorName["socket-port"];
		socketMap_[entity.value()] =
			portObj.get<uint16_t>().value_or(kSocketPort);

		/* Parse the profiles info */
		const YamlObject &profiles = sensorName["profiles"];
		int ret = parseSensorProfiles(profiles, entity.value());
		if (ret) {
			LOG(NxpNeoUguzziConfig, Warning)
				<< "Invalid profiles section in config file";
			return ret;
		}
	}

	return 0;
}

/**
 * \brief Parse the sensor profiles section in the yaml configuration file
 * \param[in] profiles The profiles node in yaml file
 * \param[in] entity The sensor entity for which profile is parsed
 * \return 0 if no error was detected, a negative error code otherwise
 */
int IPAFileConfig::parseSensorProfiles(const YamlObject &profiles,
				       const std::string &entity)
{
	std::vector<TuningInfo> tuningInfos;

	/* Parse the list of profiles */
	const auto &listProfile = profiles.asList();
	for (const auto &[i, profile] : utils::enumerate(listProfile)) {
		if (!profile.isDictionary()) {
			LOG(NxpNeoUguzziConfig, Error)
				<< "Invalid YAML syntax for profiles " << i;
			return -EINVAL;
		}
		const auto &resolution = profile["resolution"].get<Size>();
		if (!resolution.has_value()) {
			LOG(NxpNeoUguzziConfig, Error)
				<< "Missing camera resolution name";
			return -EINVAL;
		}
		TuningInfo tuningInfo;
		const YamlObject &bppObj = profile["bit-depth"];
		tuningInfo.bitDepth =
			bppObj.get<uint16_t>().value_or(kBitDepth);

		const YamlObject &dtpObj = profile["dtp-file"];
		tuningInfo.dtpFile = dtpObj.get<std::string>().value_or("");

		const YamlObject &tuningIdObj = profile["tuning-id"];
		tuningInfo.tuningId =
			tuningIdObj.get<uint16_t>().value_or(kTuningId);

		const YamlObject &tuningModeObj = profile["tuning-mode"];
		tuningInfo.tuningMode =
			tuningModeObj.get<uint16_t>().value_or(kTuningMode);

		tuningInfo.resolution = resolution.value();

		tuningInfos.push_back(tuningInfo);
	}

	sensorMap_[entity] = tuningInfos;

	return 0;
}

/**
 * \brief Parse the sensor filter section in the yaml configuration file
 * \param[in] entity The sensor filter node in yaml file
 * \return 0 if no error was detected, a negative error code otherwise
 */
int IPAFileConfig::parseEntityFilter(const YamlObject &entity)
{
	if (entity.isValue())
		sensorFilter_ = entity.get<std::string>().value_or("");

	return 0;
}

/**
 * \brief Report the tuning info associated to a camera entity,
 *        to a resolution and to a bit depth.
 * \param[in] name The name of the camera media device entity
 * \param[in] resolution The resolution of the camera stream
 * \param[in] bitDepth The bits per pixel of the camera stream
 *
 * The tuningInfo structure contains information related to the tuning
 * of the sensor.
 *
 * \return 0 on success or a negative error code otherwise
 */
const TuningInfo *IPAFileConfig::tuningInfo(const std::string &name,
					    Size resolution,
					    unsigned int bitDepth) const
{
	auto iter = sensorMap_.find(name);

	if (iter != sensorMap_.end()) {
		/* Report the tuning infos set matching the sensor entity */
		const std::vector<TuningInfo> *tuningInfos = &(iter->second);

		/*
		 * Search for the tuning infos matching the resolution and
		 * the bit depth.
		 */
		auto iter_res = std::find_if(
			tuningInfos->begin(), tuningInfos->end(),
			[&](auto &info) {
				return ((info.resolution == resolution) &&
					(info.bitDepth == bitDepth));
			});
		if (iter_res != tuningInfos->end()) {
			const TuningInfo *tuningInfo = &(*iter_res);
			LOG(NxpNeoUguzziConfig, Debug) << "TuningInfo parsed for ["
						       << name << "; "
						       << resolution << "; "
						       << bitDepth << "bpp]: ["
						       << tuningInfo->dtpFile << ", "
						       << tuningInfo->tuningId << ", "
						       << tuningInfo->tuningMode << "]";
			return tuningInfo;
		}
	}
	LOG(NxpNeoUguzziConfig, Error) << "No tuning Info found for ["
				       << name << "; " << resolution << "; "
				       << bitDepth << "bpp]";
	return nullptr;
}

/**
 * \brief Report the socket port associated to a camera entity
 * \param[in] name The name of the camera media device entity
 *
 * \return The socket port if it exists, 0 otherwise
 */
uint16_t IPAFileConfig::socketPort(const std::string &name) const
{
	auto iter = socketMap_.find(name);

	if (iter != socketMap_.end()) {
		LOG(NxpNeoUguzziConfig, Debug) << "Socket port parsed for "
					       << name << ": ["
					       << iter->second << "]";
		return iter->second;
	} else {
		return 0;
	}
}

} /* namespace libcamera::ipa::nxpneo */
