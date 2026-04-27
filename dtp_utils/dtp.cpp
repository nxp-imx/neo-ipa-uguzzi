/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2024-2026 NXP
 *
 * Calibration Binary Data Wrapper
 *
 * Copyright (C) 2021, Google Inc.
 *
 * This is derived work of the BinaryData class implementation from
 * libcamera IPU3 project hosted on:
 * https://git.libcamera.org/libcamera/ipu3-ipa.git/
 */

#include <dtp.h>

#include <libcamera/base/file.h>
#include <libcamera/base/log.h>
#include <libcamera/base/utils.h>
#include <libcamera/logging.h>

namespace libcamera {

LOG_DEFINE_CATEGORY(NxpNeoUguzziDTP)

namespace ipa::nxpneo {

DTP::DTP()
	: dataBuff_{}, data_{ nullptr }, size_{ 0 }
{
}

int DTP::load(const std::string &filename)
{
	File dtpBinary(filename);

	if (!dtpBinary.exists()) {
		LOG(NxpNeoUguzziDTP, Error) << "Failed to find DTP file " << filename;
		return -ENOENT;
	}

	if (!dtpBinary.open(File::OpenModeFlag::ReadOnly)) {
		LOG(NxpNeoUguzziDTP, Error) << "Failed to open DTP file" << filename;
		return -EINVAL;
	}

	ssize_t fileSize = dtpBinary.size();
	if (fileSize < 0) {
		LOG(NxpNeoUguzziDTP, Error) << "Failed to determine the size of DTP file " << filename;
		return -ENODATA;
	}

	dataBuff_.resize(fileSize);

	if (dtpBinary.read(dataBuff_) != fileSize) {
		LOG(NxpNeoUguzziDTP, Error) << "Failed to read DTP file " << filename;
		return -EIO;
	}

	data_ = dataBuff_.data();
	size_ = fileSize;

	LOG(NxpNeoUguzziDTP, Debug) << "DTP file successfully loaded " << filename;

	return 0;
}

} /* namespace ipa::nxpneo */

} /* namespace libcamera */
