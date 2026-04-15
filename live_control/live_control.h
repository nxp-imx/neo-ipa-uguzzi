/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright 2024-2026 NXP
 *
 * NXP NEO uGuzzi Live Control
 */

#pragma once

#include <stdint.h>
#include <sys/types.h>

#include <linux/nxp_neoisp.h>

#include "live_control_cr.h"

extern "C" {
#include "uguzzi/uguzzi.h"
#include "uguzzi_connect/uguzzi_commands.h"
}

#define LIVE_CONTROL_DEFAULT_SERVER_PORT (50000)

#define LIVE_CONTROL_CMD_HANDLE_TIMEOUT_MS (1)

namespace libcamera::ipa::nxpneo {

enum LiveControlStatus {
	LC_NOT_INITIALIZED,
	LC_INITIALIZED,
};

#define FD_INVALID (-1)

struct EmbeddedDataPkg {
	EmbeddedData channel[UGUZZI_CAMERA_CNT];
};

struct ImageBufferViewSetPkg {
	ImageBufferViewSet channel[UGUZZI_CAMERA_CNT];
};

struct IspStatisticsPkg {
	const NxpNeoStats *channel[UGUZZI_CAMERA_CNT];
};

class LiveControl
{
public:
	static LiveControl &getInstance()
	{
		static LiveControl liveCtrl;
		return liveCtrl;
	}
	LiveControl(LiveControl const &) = delete;
	void operator=(LiveControl const &) = delete;

	int createSocket(uint16_t port);
	int init(uint32_t maxImgWidth,
		 uint32_t maxImgHeight);
	int closeSocket();

	int handleCmd(const uguzzi_sensor_data_pkg_t *sensorDataPkg,
		      const EmbeddedDataPkg *embeddedDataPkg,
		      const ImageBufferViewSetPkg *imageBuffViewSetPkg,
		      const IspStatisticsPkg *ispStatPkg,
		      unsigned int activeChannel,
		      int timeoutMs);

	int deinit();

	/* the callback to register to uguzzi should be static */
	static int txDataToTtCb(uguzzi_command_packet_header_t *apHeader,
				uint8_t apData[],
				uint32_t aDataSzBytes);
private:
	LiveControl()
		: rcvHeader_({}), rcvPayload_({})
	{
		status_ = LC_NOT_INITIALIZED;
		clientFd_ = FD_INVALID;
		serverFd_ = FD_INVALID;
		rcvPayloadSz_ = 0;
	}

	int waitForConnection(int timeoutMs);
	int pollForCmd(int timeoutMs);
	int rxTtCmd();
	ssize_t readBuf(uint8_t *apBuf, size_t aSizeBytes) const;
	int writeBuf(const void *apcBuf, size_t aBufSzBytes) const;
	int txDataToTt(uguzzi_command_packet_header_t *apHeader,
		       uint8_t apData[],
		       uint32_t aDataSzBytes);

	std::array<bool, UGUZZI_CAMERA_CNT> cmdToProcess_{};
	LiveControlStatus status_;
	int serverFd_;
	int clientFd_;
	uguzzi_command_packet_header_t rcvHeader_;
	uint32_t rcvPayloadSz_;
	uguzzi_command_payload_t rcvPayload_;
	uguzzi_command_receive_cb_t cmdRcvCb_;
	std::unique_ptr<LiveControlCr> liveCtrlCr_;
};

} /* namespace libcamera::ipa::nxpneo */
