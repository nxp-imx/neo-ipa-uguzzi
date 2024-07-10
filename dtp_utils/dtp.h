/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2021, Google Inc.
 * Copyright (C) 2024, NXP
 *
 * dtp.h - Calibration Binary Data Wrapper
 *
 * This is derived work of the BinaryData class implementation from
 * libcamera IPU3 project hosted on:
 * https://git.libcamera.org/libcamera/ipu3-ipa.git/
 *
 */

#include <stdint.h>
#include <string>
#include <vector>

#ifndef __IPA_NEO_DTP__
#define __IPA_NEO_DTP__

namespace libcamera::ipa::nxpneo {

class DTP
{
public:
	DTP();

	DTP(const DTP &) = delete;
	DTP &operator=(const DTP &) = delete;
	DTP(DTP &&) = delete;
	DTP &operator=(DTP &&) = delete;

	int load(const std::string &filename);
	void *data() { return data_; }
	std::size_t size() { return size_; }

private:
	std::vector<uint8_t> dataBuff_;
	void *data_;
	std::size_t size_;
};

} /* namespace libcamera::ipa::nxpneo */

#endif
