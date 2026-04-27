/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright 2024-2026 NXP
 *
 * Uguzzi external log function definition
 */

#include <stdarg.h>
#include <libcamera/base/log.h>

namespace libcamera {

LOG_DEFINE_CATEGORY(NxpNeoUguzziLib)

extern "C" {

/*
 * Function used by uGuzzi library for logging purposes
 *
 * Error log message starts with "[E]"
 */
void mms_ext_printf(const char *format, ...)
{
	va_list args;
	std::string logMsg;
	std::string strError("[E]");
	char *cstr = nullptr;

	va_start(args, format);

	int ret = vasprintf(&cstr, format, args);
	if (ret >= 0) {
		logMsg = std::string(cstr);
		if (logMsg.find(strError) == 0)
			LOG(NxpNeoUguzziLib, Error) << logMsg;
		else
			LOG(NxpNeoUguzziLib, Debug) << logMsg;
	}

	free(cstr);
	va_end(args);
}

} /* extern "C" */

} /* namespace libcamera */
