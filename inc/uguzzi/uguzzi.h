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
* @file uguzzi.h
*
* @brief Main API header for uGuzzi
*/

#ifndef UGUZZI_H
#define UGUZZI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <uguzzi/uguzzi_isp_out.h>

/** uGUZZI list of Configurations. Adding configs in sync with DTP params */
#include <uguzzi/uguzzi_config_cmd.h>
#include <uguzzi/uguzzi_dtp_db_version.h>

#ifndef UGUZZI_CAMERA_CNT
#define UGUZZI_CAMERA_CNT 4 /**< max number of cameras supported */
#endif

#define UGUZZI_PAR_STAT_COMMON_NUMBER 32
#define UGUZZI_PAR_DYN_COMMON_NUMBER 32

/* Max sizes of MMS proprietary engine simulated stats (optional) */
#define UGUZZI_AEWB_STATS_SUBBLK_MAX_X (60)
#define UGUZZI_AEWB_STATS_SUBBLK_MAX_Y (76)

#define UGUZZI_MAX_ALGOS (16)
/**
 * enumerate of WDR capture frames id's
 * embedded data and sensor settings are bound to WDR subframes
 */
typedef enum {
    UGUZZI_WDR3_ENTRY_LONG,
    UGUZZI_WDR3_ENTRY_SHORT,
    UGUZZI_WDR3_ENTRY_VERY_SHORT,
    UGUZZI_WDR3_ENTRY_MAX
} uguzzi_wdr_entry_type_t;

/**
 * Sensor exposure settings
 * type used for both settings and embedded data
 */
typedef struct {
    uint32_t exposure;  /**< Exposure time in us U32 */
    uint32_t dgain;     /**< Digital gain UQ16.16 */
    uint32_t again;     /**< Analog gain UQ16.16 */
} uguzzi_exposure_t;

/** Sensor settings White balance color gains */
typedef struct
{
    uint16_t red;   /**< UQ8.8 white balance red gain */
    uint16_t green; /**< UQ8.8 white balance green gain */
    uint16_t blue;  /**< UQ8.8 white balance blue gain */
} uguzzi_awb_gains_t;

/** Sensor White balance color offsets*/
typedef struct
{
    int16_t red;    /**< Q8.8 white balance red offset */
    int16_t green;  /**< Q8.8 white balance green offset */
    int16_t blue;   /**< Q8.8 white balance blue offset */
} uguzzi_awb_gains_offset_t;

/**
 * Sensor settings for WB,
 * gains, offsets, color temperature(for LSC personalization)
 */
typedef struct {
    uguzzi_awb_gains_t        wb_gains; /**< colors gains for the current frame*/
    uguzzi_awb_gains_offset_t  offsets; /**< colors offsets for the current frame*/
    uint16_t               colour_temp; /**< estimated color temp. in Kelvin's*/
} uguzzi_whitebalance_t;

/*
 * uGUZZI sensor settings from Embedded data for a single camera/channel
 * - exp_gain settings for 3 WDR subframes
 * - (optional) ratios between subframes
 * - WB coefficients applied (1.0 when WB not applied in the sensor)
 * - sensor temperature (for correction of sensor limits)
 * - validity flag - only valid channels in the multicamera package structure have this flag set
 */
typedef struct {
    uguzzi_exposure_t exp_gain[UGUZZI_WDR3_ENTRY_MAX]; /**< exposure, again,
                                    dgain for 3(or more) virtual subframes. */
    uint32_t l2s_ratio;             /**< ratio of dynamic UQ24.8 */
    uint32_t l2vs_ratio;            /**< ratio of dynamic UQ24.8 */
    uguzzi_whitebalance_t wb;       /**< WB coefficients applied
                                      (1.0 when WB not applied in the sensor)*/
    uint32_t sensor_temperature_c;  /**< sensor temperature UQ24.8
                                      (for correction of sensor limits)*/
    uint32_t valid;                 /**< set for valid channels in the
                                      multicamera package structure */
} uguzzi_sensor_data_t;

/**
 * uGUZZI input sensor settings from Embedded data as package for all channels
 * Application collects, sync and assigns sequential number and timestamp to
 * valid sets of frames
 */
typedef struct {
    uint64_t frame_num;     /**< considered as one for all - sync mandatory */
    uint64_t timestamp_ns;  /**< timestamp */
    uguzzi_sensor_data_t channel[UGUZZI_CAMERA_CNT];
} uguzzi_sensor_data_pkg_t;

typedef enum 
{
    HW_STATS,
    SW_STATS,
    HW_STATS_SEN_HIST,
    SW_STATS_SEN_HIST,
    HW_STATS_SEN_HIST_AR,
    MAX_STATS_TYPE
} uguzzi_stats_t;

typedef struct
{
    uint32_t sum_red;
    uint32_t sum_green;
    uint32_t sum_blue;
} wb_rgb_data_t;

typedef struct
{
    wb_rgb_data_t*  wb_statistics;
    uint16_t wb_statistics_grid_size_h;
    uint16_t wb_statistics_grid_size_v;
} wb_statistics_data_t;

/**
 * uGUZZI input statistics and histograms for a channel
 * including AE, AWB statistics and AE histograms
 */
typedef struct {
    ae_statistics_data_hw_t     *p_ae_stats;    /**< pointer to ISP generated AE
                                                  statistics */
    ae_histogram_data_hw_t      *p_ae_hist;     /**< pointer to HxV generated
                                                  weighted  histograms UQ4.12 */
    awb_statistics_data_hw_t    *p_awb_stats;   /**< pointer to ISP generated
                                                  AWB statistics */
    wb_statistics_data_t        *p_awb_sw_stats;/**< alternative stats */
    uguzzi_stats_t              stats_type;
    uint16_t                    valid;  /**< validity of channel stats input */
} uguzzi_stats_data_t;

/**
 * uGUZZI input statistics and histograms data as package for all channels
 * Application collects and packs stats for all valid channels as uGUZZI input
 */
typedef struct {
    uint64_t            timestamp_ns;   /**< timestamp */
    uguzzi_stats_data_t channel[UGUZZI_CAMERA_CNT];
} uguzzi_stats_data_pkg_t;

typedef struct {
    char        algo_name[16];
    uint32_t    algo_version[2];
    int32_t     algo_status;
}uguzzi_algo_report_t;

typedef struct {
    uguzzi_algo_report_t algos[UGUZZI_MAX_ALGOS];
}uguzzi_algo_version_t;

/**
 * Sensor frame settings for single exposure
 * Application needs to distribute it in case of WDR sensor. Conversion library
 * is needed to match sensor driver API
 */
typedef struct {
    uguzzi_exposure_t   exp;  /**< Sensor exposure settings */
    uint32_t ae_ratio_min;    /**< HDR exposure ratio control UQ22.10 */
    uint32_t ae_ratio_max;    /**< HDR exposure ratio control UQ22.10 */
} uguzzi_frame_settings_normal_t;

/**
 * uGUZZI output - Sensor frame settings for one channel
 * Exposure is for single exposure frame but WB settings are common for all exposure frames
 */
typedef struct {
    uguzzi_frame_settings_normal_t exp_n;   /**< normal settings */
    uguzzi_whitebalance_t          wb;      /**< WB coefficients */
} uguzzi_sensor_settings_t;

/** uGUZZI output - Sensor frame settings as package for all channels */
typedef struct {
    uguzzi_sensor_settings_t *channel[UGUZZI_CAMERA_CNT];
} uguzzi_sensor_settings_pkg_t;

typedef struct {
    uint32_t s_params[UGUZZI_PAR_STAT_COMMON_NUMBER];
    uint32_t d_params[UGUZZI_PAR_DYN_COMMON_NUMBER];
    uint32_t aec_info[32];
    uint32_t aglbce_info[32];
} uguzzi_metadata_t;

/**
 * uGUZZI output - ISP settings for all channels. Application's Provided buffers
 * are filled using ISP native format. Not NULL slot updated.
 * Application function have to write data to ISP. It have to decide to reuse
 * one for all if needed.
 */
typedef struct {
    vpipe_settings_hw_t    *isp_config[UGUZZI_CAMERA_CNT];
    uguzzi_metadata_t      uguzzi_metadata[UGUZZI_CAMERA_CNT];
} uguzzi_isp_settings_pkg_t;

/* Single channel initial configuration */
typedef struct {
    uint32_t        sensor_id;      /**< sensor ID for DTP identification */
    uint32_t        sensor_mode;    /**< optional mode - 0 is AUTO */
    uint32_t        image_W;        /**< image width */
    uint32_t        image_H;        /**< image height */
    uguzzi_bayer_pattern_t  pattern;/**< Bayer pattern */
    uint32_t        isp_active;     /**< run or not ISP algo for that channel */
} uguzzi_init_channel_configuration_t;

/** uGUZZI init input - initialization settings of uGUZZI */
typedef struct {
    uint16_t        camera_cnt;     /**< number of valid channels/cameras */
    void            *dtp_database;  /**< optional pointer to DTP database buffer
                        provided by Application,(NULL if external not available
                        and internal will be used) */
    uint32_t        dtp_database_size; /**< size of external DTP buffer */
    uguzzi_init_channel_configuration_t channel_config[UGUZZI_CAMERA_CNT];
                            /**< separate configurations for defined cameras */
} uguzzi_init_configuration_t;

/**
 * uGUZZI configuration data
 * used to extend initial configuration or to update parameters runtime
 */
typedef struct {
    uint16_t            channel_id;     /**< camera/channel id */
    uguzzi_cfg_cmd_t    config_id;      /**< config item id , table used to
                                    convert id's to commands for FD/SD update */
    int32_t             config_val;     /**< value to set in corresponding FD/SD
                                        element (flag, setting) */
} uguzzi_configuration_t;

/** LUT for generation of histograms for L/V/S frame using compressed data */
typedef struct {
    unsigned char l;
    unsigned char s;
    unsigned char v;
} uguzzi_hist_lut_lsv_t;

/**
 * ROI for histograms calculation
 * (offsets and size as part of original frame (float 0.0:1.0)
 */
typedef struct {
    float stats_x_offset;
    float stats_y_offset;
    float stats_width;
    float stats_height;
} uguzzi_hist_roi_cfg_t;

/**
 * @brief Getting decomp LUT for generation of histograms for L/V/S frames
 *
 * @param channel_idx   [in]        - channel selection
 *                                      (usually definition are common)
 * @param hist_lut      [out]       - pointer to a buffer for 4096 elements LUT
 *              (for processing of 12bit data stream - (2^12) -> 4096 elements)
 * @return                          - 0 for success, non-zero error IDs
 */
int uguzzi_get_hist_lut(uint32_t channel_idx, uguzzi_hist_lut_lsv_t *hist_lut);

/**
 * @brief Getting ROI configuration for generation of histograms
 *
 * @param channel_idx   [in]        - channel selection
 * @param hist_roi_cfg  [out]       - pointer to a buffer for offset/size
 *                                      configuration of rectangle ROI
 * @return                          - 0 for success, non-zero error IDs
 */
int uguzzi_get_hist_roi_cfg(uint32_t channel_idx, uguzzi_hist_roi_cfg_t *hist_roi_cfg);

/**
 * @brief uGUZZI init function
 *
 * Instantiates uGUZZI and defines start-up configuration for all channels
 *
 * @param uguzzi_cfg    [in]        - pointer to a structure ontaining start-up
 *                                    configuration
 * @return                          - 0 for success, non-zero error id's
 */
int uguzzi_init(uguzzi_init_configuration_t *uguzzi_cfg);

/**
 * @brief Export uGuzzi initial configuration to other modules
 *
 * @return              - pointer to uGuzzi initial configuration (read only)
 */
const uguzzi_init_configuration_t * uguzzi_get_init_configuration (void);


/**
 * @brief Export DTP db version structure
 *
 * @return              - pointer to DTP db version structure (read only)
 */
const uguzzi_dtp_db_version_t *  uguzzi_get_dtp_db_version (void);

/**
 * @brief Export Algos version structure
 *
 * @return              - pointer to Algos version structure (read only)
 */
const uguzzi_algo_version_t *  uguzzi_get_algos_version (void);

/**
 * @brief uGUZZI config function
 *
 * Passes to uGUZZI a single item config update.
 * Using after uguzzi_init() for setting additional configuration params
 * Using between uguzzi_process() for setting run-time parameters or "live" data
 *
 * @param uguzzi_cfg      [in]      - pointer to a structure containing single
 *                                    configuration data
 * @return                          - 0 for success, non-zero error id's
 */
int uguzzi_config(uguzzi_configuration_t *uguzzi_cfg);

/**
 * @brief uGUZZI process function
 *
 * Passes to uGUZZI pointers to buffers containing actual sensor settings
 * (from embedded data) and ISP statistics and histograms. Expects results
 * in provided by pointers buffers for calculated sensor and ISP settings.
 * Using when package with input data for all channels collected.
 * All input & output buffers allocated by Application, FD updated with pointers
 * to them
 *
 * @param sensor_data     [in]      - pointer to a structure containing Sensor
 *                                    frame settings as package for all channels
 * @param stats_data      [in]      - pointer to a structure containing ISP
 *                             statistics/histograms as package for all channels
 * @param sensor_settings [out]     - a pointer to a structure where new sensor
 *                                      setting will be set for all channels
 * @param isp_settings    [out]     - pointer to a structure where new ISP
 *                             settings will be set as package for all channels
 * @return                          - 0 for success, non-zero is error id's
 */
int uguzzi_process(uguzzi_sensor_data_pkg_t *sensor_data_pkg,
                 uguzzi_stats_data_pkg_t *stats_data_pkg,
                 uguzzi_sensor_settings_pkg_t *sensor_settings_pkg,
                 uguzzi_isp_settings_pkg_t *isp_settings_pkg);

/**
 * @brief Helper callback invoked at the beginning of uguzzi process
 *
 * Helps external library to monitor input data and if required to modify them.
 * There are multiple usecases that it could used - for example to store input
 * parameters in local data for further processing such as: send them to Host,
 * Live Tuning Tool may inject input data etc.
 *
 * @param sensor_data_pkg  [in/out] a pointer to a variable application uguzzi_process()
 *                                  input uguzzi_sensor_data_pkg_t. A callback
 *                                  could modify it if required.
 * @param stats_data_pkg   [in/out] a pointer to a variable application uguzzi_process()
 *                                  input uguzzi_stats_data_pkg_t. A callback could modify it
 *                                  if required.
 * @return Zero on success
 */
typedef int (*cb_uguzzi_process_begin_t) (uguzzi_sensor_data_pkg_t *sensor_data_pkg,
                                          uguzzi_stats_data_pkg_t  *stats_data_pkg);

/**
 * @brief Helper callback invoked at the end of uguzzi process.
 *
 * Helps external library to monitor uguzzi output data and if required to modify
 * them. There are multiple usecases that could be used - for example to send them
 * to Host, Live tuning may overwrite part or entire output etc.
 *
 * @param sensor_settings_pkg [in] a pointer to calculated sensor settings.
 * @param isp_settings_pkg    [in] a pointer to calculated isp settings.
 *
 * @return Zero on success
 *
 */
typedef int (*cb_uguzzi_process_end_t)  (uguzzi_sensor_settings_pkg_t *sensor_settings_pkg,
                                         uguzzi_isp_settings_pkg_t *isp_settings_pkg);


/**
 * @breif Set a uGuzzi process begin callback;
 *
 * New callback (if not NULL) will be set and will be executed at the beginning of
 * every uguzzi process. This API returns old callback value, allowing to chain
 * multiple callbacks (users).
 *
 * @param new_callback [in] pointer to new callback
 * @return old callback
 */
cb_uguzzi_process_begin_t uguzzi_set_process_begin_callback(cb_uguzzi_process_begin_t new_callback);

/**
 * @breif Set a uGuzzi process end callback;
 *
 * New callback (if not NULL) will be set and will be executed at the end of
 * every uguzzi process. This API returns old callback value, allowing to chain
 * multiple user callbacks (users).
 *
 * @param new_callback
 * @return
 */
cb_uguzzi_process_end_t   uguzzi_set_process_end_callback(cb_uguzzi_process_end_t new_callback);

/**
 * @breif uGUZZI delete function
 *
 * De-init uGUZZI in order resources to new Init to be ready
 * Using when usecase have to close
 *
 * @return          - 0 for success, non-zero error id's
 */
int uguzzi_delete(void);

#ifdef __cplusplus
}
#endif

#endif /* UGUZZI_H */
