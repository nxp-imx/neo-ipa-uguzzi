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
* @file uguzzi_ext_algs.h
*
* @brief Types and API for external algorithm creation and use with uGuzzi
*/

#ifndef UGUZZI_EXT_ALGS_H
#define UGUZZI_EXT_ALGS_H

#include <stdint.h>

#define UGUZZI_EXT_ALGS_SUCCESS (0) /** No errors. The function call was successful. */
#define UGUZZI_EXT_ALGS_FAILURE (-1) /** The function call failed due to unspecified error */
#define UGUZZI_EXT_ALGS_INVALID_ARG (-2) /** At least one of the arguments was invalid. */
#define UGUZZI_EXT_ALGS_ALG_NOT_FOUND (-3) /** The algorithm was not found in the DTP database. */
#define UGUZZI_EXT_ALGS_DTP_QUERY_FAILURE (-4) /** The DTP database failed to process the query. */

/**
 * Maximum number of MMS external algorithms supported.
 * Keep in sync with uguzzi_dtpdb_ext_algs_id_t
 */
#define UGUZZI_EXT_ALGS_MMS_MAX (1U)
/**
 * Maximum number of customer external algorithms supported.
 * Keep in sync with uguzzi_dtpdb_ext_algs_id_t
 */
#define UGUZZI_EXT_ALGS_CUSTOMER_MAX (16U)

/**
 * @brief Represents the possible algorithm IDs.
 *
 * The first set of IDs is for MMS external algorithms.
 * The rest of the IDs are for customer specific algorithms.
 */
typedef enum {
    /**
     * Camera info algorithm ID.
     * Calling uguzzi_ext_algs_dtp_query with this ID will provide
     * general info about the camera configuration of the system.
     * See uguzzi_cam_info_dtp.h for details.
     */
    UGUZZI_DTP_ID_EXT_ALGS_CAMERA_INFO,

    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_0 = 16,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_1,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_2,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_3,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_4,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_5,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_6,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_7,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_8,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_9,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_10,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_11,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_12,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_13,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_14,
    UGUZZI_DTP_ID_EXT_ALGS_CUSTOMER_ALG_15
} uguzzi_dtpdb_ext_algs_id_t;

/**
 * @brief Represents the possible output data reference types.
 */
typedef enum {
    /**
     * The output data refers to only one set of algorithm specific data.
     */
    UGUZZI_DTP_DATA_TYPE_FIXED = 1 << 16,
    /**
     * The output data refers to two sets of algorithm specific output data.
     * Application must use 1D interpolation to produce the final output data.
     */
    UGUZZI_DTP_DATA_TYPE_1D,
    /**
     * The output data refers to four sets of algorithm specific output data.
     * Application must use 2D interpolation to produce the final output data.
     */
    UGUZZI_DTP_DATA_TYPE_2D,
    /**
     * Represents unknown data type.
     */
    UGUZZI_DTP_DATA_TYPE_INVALID
} uguzzi_dtp_data_type_t;

/**
 * @brief Structure to represent the extracted/output data (algorithm specific) of
 * DTP database.
 *
 * The data in this structure should be interpreted depending on the "type"
 * field as follows:
 *
 * UGUZZI_DTP_DATA_TYPE_FIXED:
 * All k_* coefficient will be equal to 0 and should not be used. All pointers
 * (x?y?) will point to the same data set. Use x1y1 to access the algorithm
 * specific data.
 *
 * UGUZZI_DTP_DATA_TYPE_1D:
 * In this case k_x1y1 == k_x1y2 and k_x2y1 == k_x2y2. Also x1y1 == x1y2 and
 * x2y1 == x2y2. Client application should use k_x1y1 and k_x2y1 coefficients
 * to interpolate (1D) between x1y1 and x2y1 data sets.
 *
 * UGUZZI_DTP_DATA_TYPE_2D:
 * Use k_x1y1, k_x2y1, k_x1y2 and k_x2y2 coefficients to interpolate (2D)
 * between data referenced by x1y1, x2y1, x1y2 and x2y2 pointers.
 *
 * All k_* coefficients are in UQ0.16 fixed point format.
 */
typedef struct {
    /** Data type referenced by this structure (see uguzzi_dtp_data_type_t). */
    int32_t type;

    uint32_t k_x1y1; /** Interpolation coefficient corresponding to x1y1 */
    uint32_t k_x2y1; /** Interpolation coefficient corresponding to x2y1 */
    uint32_t k_x1y2; /** Interpolation coefficient corresponding to x1y2 */
    uint32_t k_x2y2; /** Interpolation coefficient corresponding to x2y2 */

    uint32_t pad_1; /** Explicit padding for 64bit architectures. */

    const void *x1y1; /** Pointer to the actual algorithm specific data. */
    const void *x2y1; /** Pointer to the actual algorithm specific data. */
    const void *x1y2; /** Pointer to the actual algorithm specific data. */
    const void *x2y2; /** Pointer to the actual algorithm specific data. */
} uguzzi_dtp_output_t;

/**
 * Searches DTP database for algorithm with specified IDs. If found, extracts
 * the algorithm's tuning parameters according to the specified
 * camera's current static and dynamic parameters.
 *
 * @param camera [in] The camera whose static and dynamic parameters
 *     will be used to find the algorithm's tuning parameters.
 * @param alg_id [in] Algorithm ID. One of uguzzi_dtpdb_ext_algs_id_t IDs.
 * @param sub_type [in] The version/ID of algorithm structure.
 * @param output [out] Pointer to output structure to store the extracted
 *     tuning parameters.
 *
 * Conditions:
 *     uguzzi_init() must be called successfully before calling this function.
 *
 * @return Zero on success and negative error code on failure
 */
int uguzzi_ext_algs_dtp_query(
    uint16_t camera,
    uint32_t alg_id,
    uint32_t sub_type,
    uguzzi_dtp_output_t *output
);

#endif /* UGUZZI_EXT_ALGS_H */
