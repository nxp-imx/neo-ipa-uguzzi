/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * run_tests.h - NXP NEO uGuzzi IPA tests
 * Copyright 2024 NXP
 */

#include <stdint.h>

#include <libcamera/base/span.h>

namespace libcamera::ipa::nxpneo {

int initializeTestsUguzzi(Span<uint32_t> camFrames,
			  uint8_t cameraCnt,
			  std::string sensorModel);

} /* namespace libcamera::ipa::nxpneo */
