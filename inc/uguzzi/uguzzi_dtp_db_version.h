/* =============================================================================
* Copyright 2006-2025 MM Solutions EAD (MMS)
*
* MMS Proprietary. This software is owned or controlled by MMS and may only be
* used strictly in accordance with the applicable license terms. By expressly
* accepting such terms or by downloading, installing, activating and/or
* otherwise using the software, you are agreeing that you have read, and that
* you agree to comply with and are bound by, such license terms. If you do not
* agree to be bound by the applicable license terms, then you may not retain,
* install, activate or otherwise use the software.
* =========================================================================== */
/**
* @file uguzzi_dtp_db_version.h
*
* @brief This file defines the structure for DTP version in dtp database.
*/

#ifndef UGUZZI_DTP_DB_VERSION_H__
#define UGUZZI_DTP_DB_VERSION_H__

#define SHA_CHECKSUM_LEN (40)

#define DTP_DB_VER_SUB_ID (3)

typedef struct {
    uint32_t tuning_version_major;
    uint32_t tuning_version_minor;
	uint32_t tuning_version_build;
    uint32_t tuning_tool_app_version;
    uint32_t data_year;
    uint32_t data_month;
    uint32_t data_day;
    uint32_t time_hour;
    uint32_t time_minute;
    uint32_t time_second;
    uint8_t  sha[SHA_CHECKSUM_LEN];
} uguzzi_dtp_db_version_t;

int uguzzi_get_dtp_version(uguzzi_dtp_db_version_t *version);

#endif // UGUZZI_DTP_DB_VERSION_H__
