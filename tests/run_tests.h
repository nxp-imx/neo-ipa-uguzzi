/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright 2024-2026 NXP
 *
 * NXP NEO uGuzzi IPA tests
 */

#include <stdint.h>

#include <libcamera/base/span.h>

namespace libcamera::ipa::nxpneo {

int initializeTestsUguzzi(Span<uint32_t> camFrames,
			  uint8_t cameraCnt,
			  std::string sensorModel);

} /* namespace libcamera::ipa::nxpneo */
