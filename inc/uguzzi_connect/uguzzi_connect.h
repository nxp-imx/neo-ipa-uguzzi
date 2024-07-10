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
* @file uguzzi_connect.h
*
* @brief Contains common definitions for all modules included in Connect
*/

#ifndef CONNECT_H__
#define CONNECT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t con_status_t;

#define CONNECT_SUCCESS      (  0 )
#define CONNECT_ERROR        ( -1 ) /*< Unspecified error */
#define CONNECT_E_PARAM      ( -2 ) /*< Invalid parameter */
#define CONNECT_E_SIZE       ( -3 ) /*< Invalid size */
#define CONNECT_E_CHECKSUM   ( -4 ) /*< Verification fail */
#define CONNECT_E_ACCESS     ( -5 ) /*< Invalid access type */
#define CONNECT_E_ALIGNMENT  ( -6 ) /*< Invalid alignment */
#define CONNECT_E_ASSERT     ( -7 ) /*< Critical error */

#ifdef __cplusplus
}
#endif

#endif /*CONNECT_H__ */
