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
* @file uguzzi_config_cmd.h
*
* @brief Various configuration commands for uGuzzi
*/

#ifndef UGUZZI_CONFIG_CMD_H
#define UGUZZI_CONFIG_CMD_H

/**
 * Non comprehensive list of config ID's used for reconfiguration of uGUZZI
 * List could be extended in sync with adding of parameters to DTP
 *
 * On configuration call, value corresponding to the config command ID is
 * distributed to corresponding variable in Config Descriptor(CD) or
 * Frame Descriptor(FD). Values will affect next processing calls via algorithm
 * reconfiguration or DTP parameters
 */

typedef enum {
/*------------------------------------------------------------ */
    /** Common static parameters ----------------------------- */
    /** CAMERA_ID  - 0 set only by INIT */
    CMD_CAMERA_MODE = 1,
    CMD_EFFECT,
    CMD_SCENE_MODE, /**< Scene , CD-> uint32_t scene_mode */
    CMD_CONTRAST,   /**< contrast, CD -> uint32_t contrast */
    CMD_SATURATION, /**< saturation, CD-> uint32_t saturation */
    CMD_SHARPNESS,  /**< sharpness, CD-> uint32_t sharpness */
    CMD_BRIGHTNESS, /**< brightness, CD-> uint32_t brightness */
    CMD_AEC_CONV_SPEED, /**< AEC conv speed, CD-> uint32_t aec_conv_speed */
    CMD_SEN_CONV_RATIO, /**< Sensor conversion ratio (gain or sensitivity) */
    CMD_SEN_TEMPERATURE_LEVEL, /**< Sensor temperature/dark current level */

    /** Algorithm configuration parameters ------------------- */
    CMD_AE_MODE = 64,    /**< AE mode - 0 AUTO      CD->ae_mode */
    CMD_AEC_PROCESSING_MODE, /**< 0 NORMAL / 1 LOCK / 2 DISABLE CD->aec_processing_mode */
    CMD_AWB_PROCESSING_MODE, /**< 0 NORMAL / 1 LOCK / 2 DISABLE CD->awb_processing_mode */
    CMD_AE_FLICKER_MODE,    /**< uguzzi_cfg_mains_flicker_t mode
                                - 0 OFF, 1 - 50Hz, 2 - 60Hz
                                   conv. CD->ae_flicker_mode */
    CMD_AE_LFM_MODE,        /**< uguzzi_cfg_lfm_mode_t mode
                                - 0 OFF, 1 - ON, 2 - ADAPTIVE */
    CMD_MANUAL_COLOR_TEMP,  /**< Manual color temperature
                                - 0 AUTO / value MANUAL in Kelvin's
                                   CD->manual_color_temp */
    CMD_MANUAL_Y_LEVEL,     /**< Manual AEC Y level
                                - 0 AUTO / value
                                   CD->manual_y_level */
    CMD_HISTOGRAM_WEIGHT,   /**< Histogram weight coefficient
                                - 0 DTP control, coefficient value overwriting DTP */
    CMD_AGLBCE_PROCESSING_MODE, /**< 0 NORMAL / 1 LOCK / 2 DISABLE CD->aglbce_processing_mode */
    CMD_MIN_FPS, /**< Minimum FPS - requires value > 0   CD->min_fps */
    CMD_ALGO_RECONFIG       = 399,
    /** enumerate of custom static CAR parameters supported via DTP */
    CMD_CUSTOM_STAT_PARAM_0 = 400,
    CMD_CUSTOM_STAT_PARAM_1,
    CMD_CUSTOM_STAT_PARAM_2,
    CMD_CUSTOM_STAT_PARAM_3,
    CMD_CUSTOM_STAT_PARAM_4,
    CMD_CUSTOM_STAT_PARAM_5,
    CMD_CUSTOM_STAT_PARAM_6,
    CMD_CUSTOM_STAT_PARAM_7,
    CMD_CUSTOM_STAT_PARAM_8,
    CMD_CUSTOM_STAT_PARAM_9,
    CMD_CUSTOM_STAT_PARAM_10,
    CMD_CUSTOM_STAT_PARAM_11,
    CMD_CUSTOM_STAT_PARAM_12,
    CMD_CUSTOM_STAT_PARAM_13,
    CMD_CUSTOM_STAT_PARAM_14,
    CMD_CUSTOM_STAT_PARAM_15,
    /** enumerate of custom dynamic CAR parameters supported via DTP */
    CMD_CUSTOM_DYN_PARAM_0 = 500,
    CMD_CUSTOM_DYN_PARAM_1,
    CMD_CUSTOM_DYN_PARAM_2,
    CMD_CUSTOM_DYN_PARAM_3,
    CMD_CUSTOM_DYN_PARAM_4,
    CMD_CUSTOM_DYN_PARAM_5,
    CMD_CUSTOM_DYN_PARAM_6,
    CMD_CUSTOM_DYN_PARAM_7,
    CMD_CUSTOM_DYN_PARAM_8,
    CMD_CUSTOM_DYN_PARAM_9,
    CMD_CUSTOM_DYN_PARAM_10,
    CMD_CUSTOM_DYN_PARAM_11,
    CMD_CUSTOM_DYN_PARAM_12,
    CMD_CUSTOM_DYN_PARAM_13,
    CMD_CUSTOM_DYN_PARAM_14,
    CMD_CUSTOM_DYN_PARAM_15,

    CMD__PARAM_INVALID
} uguzzi_cfg_cmd_t;

typedef enum {
    UGUZZI_MAINS_FREQ_UNKNOWN,
    UGUZZI_MAINS_FREQ_50HZ,
    UGUZZI_MAINS_FREQ_60HZ
} uguzzi_cfg_mains_flicker_t;

typedef enum {
    UGUZZI_LFM_MODE_OFF,
    UGUZZI_LFM_MODE_ON,
    UGUZZI_LFM_MODE_ADAPTIVE
} uguzzi_cfg_lfm_mode_t;

#endif /* UGUZZI_CONFIG_CMD_H */
