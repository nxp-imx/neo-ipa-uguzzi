/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright 2024-2026 NXP
 *
 * NXP NEO uGuzzi Live Control
 */

#include "live_control.h"

#include <errno.h>
#include <poll.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <libcamera/base/log.h>

#include <arpa/inet.h>

extern "C" {
#include "uguzzi_connect/uguzzi_connect.h"
#include "uguzzi_connect/uguzzi_live_tuning.h"
#include "uguzzi_connect/tuning_tool_commands.h"
#include "uguzzi_connect/live_tuning.h"
}

#define LIVE_CONTROL_CRC_MISMATCH_ERR (-67)

#define UGC_EOF_ERR (-66)

namespace libcamera::ipa::nxpneo {

LOG_DEFINE_CATEGORY(NxpNeoUguzziLive)

/* CRC16 communication data integrity verification */
static inline uint16_t calcCrc16CcittLut(const uint8_t *payload,
					 uint32_t payloadLength)
{
	static const uint16_t crc16CcittLut[] = {
		0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
		0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
		0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
		0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
		0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
		0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
		0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
		0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
		0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
		0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
		0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
		0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
		0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
		0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
		0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
		0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
		0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
		0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
		0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
		0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
		0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
		0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
		0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
		0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
		0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
		0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
		0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
		0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
		0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
		0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
		0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
		0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0,
	};

	uint16_t crc = 0;

	for (uint32_t i = 0; i < payloadLength; i++) {
		uint8_t lut_idx = (uint8_t)((crc >> 8) ^ payload[i]);
		crc = (uint16_t)((crc << 8) ^ (uint16_t)(crc16CcittLut[lut_idx]));
	}

	return crc;
}

ssize_t LiveControl::readBuf(uint8_t *apBuf, size_t aSizeBytes) const
{
	ssize_t ret = 0;
	size_t bytesRead = 0;

	while (bytesRead < aSizeBytes) {
		ssize_t read = recv(clientFd_,
				    (unsigned char *)(apBuf + bytesRead),
				    aSizeBytes - bytesRead,
				    0);
		if (read < 0) {
			if (errno == EINTR)
				continue;

			LOG(NxpNeoUguzziLive, Error) << "TT message reception failed.";
			ret = -errno;
			break;
		} else if (read == 0) {
			/* EOF */
			LOG(NxpNeoUguzziLive, Warning) << "TT message incomplete.";
			ret = UGC_EOF_ERR;
			break;
		}
		bytesRead += (size_t)read;
	}

	return ret;
}

int LiveControl::writeBuf(const void *apcBuf, size_t aBufSzBytes) const
{
	size_t sentSoFar = 0;
	const unsigned char *lpData =
		static_cast<const unsigned char *>(apcBuf);
	int ret = 0;

	while (sentSoFar < aBufSzBytes) {
		ssize_t sent = send(clientFd_,
				    lpData + sentSoFar,
				    aBufSzBytes - sentSoFar,
				    MSG_NOSIGNAL);
		if (sent == -1) {
			if (errno != EINTR) {
				LOG(NxpNeoUguzziLive, Error) << "Failed to send TT data.";
				ret = -errno;
				break;
			}
		} else {
			sentSoFar += (size_t)sent;
		}
	}

	return ret;
}

int LiveControl::txDataToTtCb(uguzzi_command_packet_header_t *apHeader,
			      uint8_t apData[],
			      uint32_t aDataSzBytes)
{
	LiveControl &liveCtrl = LiveControl::getInstance();
	return (liveCtrl.txDataToTt(apHeader, apData, aDataSzBytes));
}

int LiveControl::txDataToTt(uguzzi_command_packet_header_t *apHeader,
			    uint8_t apData[],
			    uint32_t aDataSzBytes)
{
	int ret = 0;

	if (aDataSzBytes > 0U) {
		/* Compute the mpPayload's CRC-16 value */
		uint16_t crc = calcCrc16CcittLut(apData, aDataSzBytes);

		/* Set the mpPayload's CRC-16 in the header (little-endian byte order) */
		apHeader->reserved[0] = (uint8_t)(crc & 0xFFU);
		apHeader->reserved[1] = (uint8_t)((crc >> 8) & 0xFFU);
	}

	/* part 1: header */
	ret = writeBuf(apHeader, sizeof(*apHeader));
	if (ret)
		return ret;

	/* part 2: payloadsize */
	ret = writeBuf(&aDataSzBytes, sizeof(aDataSzBytes));
	if (ret)
		return ret;
	if (aDataSzBytes == 0U)
		return 0;

	/* part 3: payload */
	ret = writeBuf(apData, aDataSzBytes);

	return ret;
}

int LiveControl::rxTtCmd(void)
{
	int ret = 0;
	ssize_t res;

	/* It is expected that all 3 parts of the command will arrive
	 * at the same time!
	 * So no long blocks are expected.
	 */

	/* part 1: header */
	res = readBuf((uint8_t *)&rcvHeader_, sizeof(rcvHeader_));
	if (res != 0) {
		LOG(NxpNeoUguzziLive, Warning) << "Failed to read TT CMD header.";
		ret = -EBADMSG;
		goto msg_short;
	}

	/* part 2: payload size */
	res = readBuf((uint8_t *)&rcvPayloadSz_, sizeof(rcvPayloadSz_));
	if (res != 0) {
		LOG(NxpNeoUguzziLive, Error)
			<< "Failed to read TT CMD payloadsize.";
		ret = -EBADMSG;
		goto msg_short;
	}
	if (rcvPayloadSz_ >= sizeof(rcvPayload_)) {
		LOG(NxpNeoUguzziLive, Error)
			<< "Payload too big " << rcvPayloadSz_
			<< " bytes (max " << sizeof(rcvPayload_)
			<< " bytes).";
		ret = -EBADMSG;
		goto exit;
	}

	/* part 3: payload */
	/* even payload of size 0 is considered */
	res = readBuf(rcvPayload_.cmd_payload_uint8, rcvPayloadSz_);
	if (res != 0) {
		LOG(NxpNeoUguzziLive, Error)
			<< "Failed to read TT CMD payload.";
		ret = -EBADMSG;
	}

	/* CRC check */
	if ((res == 0) && (rcvPayloadSz_ > 0U)) {
		/* Compute the payload's CRC-16 value */
		const uint16_t crcCalculated = calcCrc16CcittLut(rcvPayload_.cmd_payload_uint8,
								 rcvPayloadSz_);

		/* Get the payload's CRC-16 from the header (little-endian byte order) */
		const uint16_t crcReceived =
			(((uint16_t)rcvHeader_.reserved[1]) << 8) |
			rcvHeader_.reserved[0];
		if (crcCalculated != crcReceived) {
			LOG(NxpNeoUguzziLive, Warning) << "TT message CRC check failed.";
			ret = -EBADMSG;
			goto exit;
		}
	}

msg_short:
	/* check if message too short */
	if (res == UGC_EOF_ERR) {
		/* Ignore EOF but break out so that a reconnection is possible */
		LOG(NxpNeoUguzziLive, Warning) << "TT message too short.";
		if (close(clientFd_)) {
			LOG(NxpNeoUguzziLive, Warning) << "Failed to close client FD.";
			ret = -ENOTCONN;
		}
		/* forces reconnection */
		LOG(NxpNeoUguzziLive, Warning) << "Lost connection";
		clientFd_ = FD_INVALID;
		ret = 0;
	}
exit:
	return ret;
}

int LiveControl::pollForCmd(int timeoutMs)
{
	int ret = 0;
	int readyFds;

	struct pollfd lFdToPoll = {};
	lFdToPoll.fd = clientFd_;
	lFdToPoll.events = POLLIN;

	readyFds = poll(&lFdToPoll, 1, timeoutMs);
	if ((readyFds == -1) && (errno != EINTR)) {
		LOG(NxpNeoUguzziLive, Error) << "Wait for socket activity failed.";
		ret = -errno;
	} else if (readyFds > 0) {
		ret = rxTtCmd();
		if ((ret != 0) || (clientFd_ == FD_INVALID))
			return ret;
		const uint8_t cam = rcvHeader_.camera_index;
		const uint8_t activeCams = lt_get_active_cameras();

		if (cam >= activeCams) {
			LOG(NxpNeoUguzziLive, Warning)
				<< "Requested cam index " << (uint32_t)cam
				<< " is bigger than active cameras count "
				<< (uint32_t)activeCams;
		}
		/* Set command to process for the requested channel */
		cmdToProcess_[cam] = true;
	}

	return ret;
}

int LiveControl::waitForConnection(int timeoutMs)
{
	int err = 0;

	if (listen(serverFd_, 1) != 0) {
		LOG(NxpNeoUguzziLive, Error)
			<< "Failed to set the server socket to listen state: "
			<< strerror(errno);
		return -errno;
	}
	int readyFds;
	struct pollfd lFdToPoll = {};
	lFdToPoll.fd = serverFd_;
	lFdToPoll.events = POLLIN;

	readyFds = poll(&lFdToPoll, 1, timeoutMs);
	if (readyFds == -1) {
		if (errno != EINTR) {
			LOG(NxpNeoUguzziLive, Error)
				<< "Poll for pending connections failed.";
			err = -errno;
		}
	} else if (readyFds > 0) {
		clientFd_ = accept(serverFd_, NULL, NULL);
		if (clientFd_ == FD_INVALID) {
			if ((errno != EINTR) && (errno != ECONNABORTED)) {
				LOG(NxpNeoUguzziLive, Error)
					<< "Failed to accept connections.";
				err = -errno;
			}
		} else {
			LOG(NxpNeoUguzziLive, Info)
				<< "Live Tuning/Control connection with Tuning Tool established";
		}
	}

	return err;
}

int LiveControl::createSocket(uint16_t port)
{
	port = (port == 0U) ? LIVE_CONTROL_DEFAULT_SERVER_PORT : port;
	serverFd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverFd_ == FD_INVALID) {
		LOG(NxpNeoUguzziLive, Error) << "Failed to create server socket: "
					     << strerror(errno);
		return -errno;
	}

	const int enable = 1;
	if (setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		LOG(NxpNeoUguzziLive, Warning)
			<< "Failed to set socket with SO_REUSEADDR!";

	struct sockaddr_in srvAddr;
	memset(&srvAddr, 0, sizeof(srvAddr));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(port);
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int res = bind(serverFd_, (struct sockaddr *)&srvAddr,
		       (socklen_t)sizeof(srvAddr));
	if (res == -1) {
		/* save it in case the logging sets it too */
		int bindErrno = errno;
		LOG(NxpNeoUguzziLive, Error)
			<< "Failed to bind socket " << strerror(bindErrno);
		close(serverFd_);
		serverFd_ = FD_INVALID;
	}

	if (serverFd_ == FD_INVALID)
		return -EINVAL;

	return 0;
}

int LiveControl::init(uint32_t maxImgWidth,
		      uint32_t maxImgHeight)
{
	int ret = 0;

	if (uguzzi_live_tuning_init()) {
		LOG(NxpNeoUguzziLive, Error)
			<< "Failed to initialize live tuning.";
		ret = -EINVAL;
		goto exit;
	}

	liveCtrlCr_ = std::make_unique<LiveControlCr>();
	if (liveCtrlCr_.get()->initLiveControlCr(maxImgWidth, maxImgHeight)) {
		LOG(NxpNeoUguzziLive, Error)
			<< "Failed to initialize custom regions.";
		ret = -EINVAL;
		goto deinit;
	}

	if (uguzzi_init_communication(txDataToTtCb, &(cmdRcvCb_))) {
		LOG(NxpNeoUguzziLive, Error)
			<< "Failed to initialize communication.";
		ret = -EINVAL;
		goto deinit;
	}
	status_ = LC_INITIALIZED;

	return 0;

deinit:
	uguzzi_live_tuning_deinit();
	/* can be called even if the corresponding init has failed */
	liveCtrlCr_.get()->deinitLiveControlCr();

exit:
	return ret;
}

int LiveControl::handleCmd(const uguzzi_sensor_data_pkg_t *sensorDataPkg,
			   const EmbeddedDataPkg *embeddedDataPkg,
			   const ImageBufferViewSetPkg *imageBuffViewSetPkg,
			   const IspStatisticsPkg *ispStatPkg,
			   unsigned int activeChannel,
			   int timeoutMs)
{
	int err = 0;

	if ((status_ != LC_INITIALIZED) || (serverFd_ == FD_INVALID))
		return -EINVAL;

	if (clientFd_ == FD_INVALID)
		return waitForConnection(timeoutMs);

	/*
	 * First process any pending command for the active channel
	 * Then process any command received for the active channel
	 */
	do {
		if (cmdToProcess_[activeChannel]) {
			LOG(NxpNeoUguzziLive, Debug)
				<< "Process command for channel" << activeChannel;

			/* Process command for the active channel */
			if (cmdRcvCb_(&rcvHeader_,
				      rcvPayload_.cmd_payload_uint8,
				      rcvPayloadSz_)) {
				LOG(NxpNeoUguzziLive, Error)
					<< "Command receiving callback failed.";
				err = -EBADMSG;
			} else {
				liveCtrlCr_.get()->handleLiveControlCmdCr(
					&sensorDataPkg->channel[activeChannel],
					&embeddedDataPkg->channel[activeChannel],
					&imageBuffViewSetPkg->channel[activeChannel],
					ispStatPkg->channel[activeChannel]);
				/* Reset command to process for the active channel */
				cmdToProcess_[activeChannel] = false;
			}
		}

		err = pollForCmd(timeoutMs);
		if (err)
			return err;
	} while (cmdToProcess_[activeChannel]);

	return err;
}

int LiveControl::deinit()
{
	if (status_ != LC_INITIALIZED)
		return 0;

	if (uguzzi_live_tuning_deinit()) {
		LOG(NxpNeoUguzziLive, Error) << "Failed to deinit live tuning.";
		return -EINVAL;
	}
	liveCtrlCr_.get()->deinitLiveControlCr();

	status_ = LC_NOT_INITIALIZED;

	return 0;
}

int LiveControl::closeSocket()
{
	if (clientFd_ != FD_INVALID) {
		if (close(clientFd_)) {
			LOG(NxpNeoUguzziLive, Error)
				<< "Failed to close client socket FD: "
				<< strerror(errno);
			return -errno;
		}
		clientFd_ = FD_INVALID;
	}
	LOG(NxpNeoUguzziLive, Debug) << "Client socket is closed.";

	if (serverFd_ != FD_INVALID) {
		if (close(serverFd_)) {
			LOG(NxpNeoUguzziLive, Error)
				<< "Failed to close server socket FD: "
				<< strerror(errno);
			return -errno;
		}
		serverFd_ = FD_INVALID;
	}
	LOG(NxpNeoUguzziLive, Debug) << "Server socket is closed.";

	return 0;
}

} /* namespace libcamera::ipa::nxpneo */
