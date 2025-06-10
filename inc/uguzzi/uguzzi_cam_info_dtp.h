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
* @file uguzzi_cam_info_dtp.h
*
* @brief DTP describing various camera related information
*/

#ifndef UGUZZI_CAM_INFO_DTP_H
#define UGUZZI_CAM_INFO_DTP_H

#include <stdint.h>

/**
 * @brief The version of this algorithm/DTP.
 *
 * Increased when backwards incompatible changes are made.
 */
#define UGUZZI_CAM_INFO_DTP_SUBTYPE_ID (5U)
/**
 * @brief The maximum number of points allowed to describe the PWL curve.
 */
#define UGUZZI_CAM_INFO_PWL_MAX_POINTS (16U)

/**
 * @brief Represents the possible data pedestal types.
 */
typedef enum {
    /**
     * The data pedestal is added before compressing the image.
     * So it must be subtracted after decompressing the image.
     */
    UGUZZI_CAM_INFO_DP_PRE,
    /**
     * The data pedestal is added after compressing the image.
     * So it must be subtracted before decompressing the image.
     */
    UGUZZI_CAM_INFO_DP_POST
} uguzzi_cam_info_dp_t;

/**
 * @brief Represents the possible data format of the RAW image.
 */
typedef enum {
    /**
     * 8 bits-per-pixel.
     */
    UGUZZI_CAM_INFO_RAW_FORMAT_8,
    /**
     * Unpacked 10 bits in 16 bits-per-pixel.
     */
    UGUZZI_CAM_INFO_RAW_FORMAT_10_UNPCK,
    /**
     * Unpacked 12 bits in 16 bits-per-pixel.
     */
    UGUZZI_CAM_INFO_RAW_FORMAT_12_UNPCK,
    /**
     * Unpacked 14 bits in 16 bits-per-pixel.
     */
    UGUZZI_CAM_INFO_RAW_FORMAT_14_UNPCK,
    /**
     * Unpacked 16 bits in 16 bits-per-pixel.
     */
    UGUZZI_CAM_INFO_RAW_FORMAT_16_UNPCK,
    /**
     * Unpacked 20 bits in 32 bits-per-pixel.
     */
    UGUZZI_CAM_INFO_RAW_FORMAT_20_UNPCK,
    /**
     * Unpacked 32 bits in 32 bits-per-pixel.
     */
    UGUZZI_CAM_INFO_RAW_FORMAT_32_UNPCK,
    /**
     * 3 x 10-bit pixels packed into 32 bits.
     */
    UGUZZI_CAM_INFO_RAW_FORMAT_10_PCK_32,
    /**
     * 2 x 12-bit pixels packed into 24 bits. Middle byte contains bits from both pixels.
     */
    UGUZZI_CAM_INFO_RAW_FORMAT_12_PCK_24,
    /**
     * Max number of formats supported.
     */
    UGUZZI_CAM_INFO_RAW_FORMAT_MAX
} uguzzi_cam_info_raw_format_t;

/**
 * @brief Represents the possible data endianness of the RAW image.
 */
typedef enum {
    UGUZZI_CAM_INFO_RAW_ENDIANNESS_LE,
    UGUZZI_CAM_INFO_RAW_ENDIANNESS_BE
} uguzzi_cam_info_raw_endianness_t;

/**
 * @brief Represents the possible color patterns of the RAW image.
 */
typedef enum {
    UGUZZI_CAM_INFO_CFA_RGrGbB,
    UGUZZI_CAM_INFO_CFA_GrRBGb,
    UGUZZI_CAM_INFO_CFA_GbBRGr,
    UGUZZI_CAM_INFO_CFA_BGbGrR,
    UGUZZI_CAM_INFO_CFA_RGGIR_4X4,
    UGUZZI_CAM_INFO_CFA_GRIRG_4X4,
    UGUZZI_CAM_INFO_CFA_BGGIR_4X4,
    UGUZZI_CAM_INFO_CFA_GBIRG_4X4,
    UGUZZI_CAM_INFO_CFA_GIRRG_4X4,
    UGUZZI_CAM_INFO_CFA_IRGGR_4X4,
    UGUZZI_CAM_INFO_CFA_GIRBG_4X4,
    UGUZZI_CAM_INFO_CFA_IRGGB_4X4,
    UGUZZI_CAM_INFO_CFA_MONOCHROME,
    UGUZZI_CAM_INFO_CFA_IR,
    UGUZZI_CAM_INFO_CFA_RCCC,
} uguzzi_cam_info_cfa_t;

/**
 * @brief Represents the possible locations to apply WB gains.
 */
typedef enum {
    UGUZZI_CAM_INFO_WB_LOCATION_SENSOR,
    UGUZZI_CAM_INFO_WB_LOCATION_ISP,
    UGUZZI_CAM_INFO_WB_LOCATION_OTHER
} uguzzi_cam_info_wb_location_t;

/**
 * @brief PWL compression/decompression curve point.
 */
typedef struct {
    uint32_t x; /* min: 0, max: 2147483647 */
    uint32_t y; /* min: 0, max: 2147483647 */
} uguzzi_cam_info_pwl_point_t;

/**
 * @brief Sensor frame configuration.
 *
 * Description of a single sensor exposure frame.
 * This frame might be part of WDR configuration.
 */
typedef struct {
    /**
     * Whether the frame is configured for the current sensor operation mode.
     */
    uint32_t enabled; /* min: 0, max: 1 */
    /**
     * Width of the image buffer in pixels.
     */
    uint32_t width; /* min: 0, max: 65535 */
    /**
     * Height of the image buffer in rows of pixels. This includes embedded lines, if any.
     */
    uint32_t height; /* min: 0, max: 65535 */
    /**
     * Number of embedded lines at the beginning of the image buffer.
     */
    uint32_t front_emb_ln_cnt; /* min: 0, max: 65535 */
    /**
     * Number of embedded lines at the end of the image buffer.
     */
    uint32_t rear_emb_ln_cnt; /* min: 0, max: 65535 */
    /**
     * Number of bytes in an image row.
     * For an image with 2 bytes per pixel and image size of 1920x1080
     * the bytes_per_line would be 1920*2.
     */
    uint32_t bytes_per_line; /* min: 0, max: 65535 */
    /**
     * Data format of the RAW image. See uguzzi_cam_info_raw_format_t for possible values.
     */
    int32_t raw_format; /* min: 0, max: 8 */
    /**
     * Data endianness of the RAW image. See uguzzi_cam_info_raw_endianness_t for possible values.
     */
    int32_t endianness; /* min: 0, max: 1 */
    /**
     * Color pattern of the pixel data, not the image buffer.
     * The image buffer might include embedded lines in which case the actual pixel data might have
     * a different color pattern. This happens if the number of front embedded lines is odd.
     */
    int32_t cfa; /* min: 0, max: 14 */

    /**
     * The type of the data pedestal used. See uguzzi_cam_info_dp_t for possible values.
     */
    int32_t data_pedestal_type; /* min: 0, max: 1 */
    /**
     * Data pedestal value to subtract.
     */
    uint32_t data_pedestal_value; /* min: 0, max: 65535 */

    /**
     * Bits per pixel before decompression.
     */
    uint32_t decomp_in_bits; /* min: 0, max: 32 */
    /**
     * Bits per pixel after decompression.
     */
    uint32_t decomp_out_bits; /* min: 0, max: 32 */
    /**
     * Number of points used to describe the decompression PWL curve.
     */
    uint32_t num_pwl_points; /* min: 0, max: 16 */
    /**
     * Decompression PWL curve used to decompress the RAW frame.
     */
    uguzzi_cam_info_pwl_point_t decomp_pwl[UGUZZI_CAM_INFO_PWL_MAX_POINTS];
} uguzzi_cam_info_frame_cfg_t;

/**
 * @brief Main DTP tuning structure.
 */
typedef struct {
    /**
     * Configuration of the first exposure frame in multi-exposure(WDR) setup.
     */
    uguzzi_cam_info_frame_cfg_t frame1_cfg; /* Long / DCG */
    /**
     * Configuration of the second exposure frame in multi-exposure(WDR) setup.
     */
    uguzzi_cam_info_frame_cfg_t frame2_cfg; /* Short / VShort */
    /**
     * Configuration of the third exposure frame in multi-exposure(WDR) setup.
     */
    uguzzi_cam_info_frame_cfg_t frame3_cfg; /* VShort / Unused */
    /**
     * Where to apply the WB gains. See uguzzi_cam_info_wb_location_t for possible locations.
     */
    int32_t apply_wb_gains_in; /* min: 0, max: 2 */
} uguzzi_cam_info_dtp_t;

#endif /* UGUZZI_CAM_INFO_DTP_H */
