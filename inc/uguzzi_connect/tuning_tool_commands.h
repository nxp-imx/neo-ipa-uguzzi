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
* \defgroup uguzzi_tt_commands Live Tuning Tool commands
* \ingroup gr_uguzzi_connect
*  @{
*
* @file tuning_tool_commands.h
*
* @brief Tuning Tool Commands and APIs to serialize/deserialize external command
*/

#ifndef TUNING_TOOL_COMMANDS_H__
#define TUNING_TOOL_COMMANDS_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Command IDs send from Tuning Tool.
 */
typedef enum
{
    APP_LT_CMD_BASE      = 35,

    /** Establishes connection with the device. Its payload data \n
     *  will contain the string “HANDSHAKE-TOOL”. Its size is the \n
     *  length of the string in bytes. */
    APP_LT_CMD_HANDSHAKE,      /**< val = 36 */

    /** used to verify that DTP binary flashed in the device is the same as  \n
     *  the DTP binary used by live tuning tool. Its payload will contain \n
     *  the checksum of the DTP binary loaded in the tool. The size of the \n
     *  payload is 80 bytes
     */
    APP_LT_CMD_UNLOCK,              /**< (val = 37) */


/* ---------------------  DTP database  ---------------------------- */
    /** used to change a parameter in the DTP binary flashed to the device.\n
     * Its payload will contain offset value which is 4 bytes  and a data \n
     * which will be variable size. It can be calculated by subtracting \n
     * the offset filed size form the payload size.  Data will be saved \n
     * starting from the given offset. The payload can be represented with \n
     * the following drawing */
    APP_LT_CMD_APPLY,               /**< (val = 38) */

    /**
     * used to get tuning data from device. Its payload will contain starting \n
     * offset and size of the required data.  The size of the offset field \n
     * is 4 bytes. Size filed will be represented in 4 bytes too. */
    APP_LT_CMD_GET,                 /**< 39 (Optional)  */

    /**  get maximal possible size of tranmit and receive buffer from device. \n
     * The size of the fields “TX_SIZE” and “RX_SIZE” is 4 bytes. */
    APP_LT_CMD_GET_SIZE,                       /**< (val = 40) */

/* -------------------  ISP Registers  -------------------------- */
    /** store data to ISP registers */
    APP_LT_CMD_WRITE_ISP,                      /**< (val = 41) */

    /** read data from ISP registers. */
    APP_LT_CMD_READ_ISP,                       /**< (val = 42) */

/* -------------------  Sensor Registers  -------------------------- */
    /** store data to sensor registers. */
    APP_LT_CMD_WRITE_SENSOR,                   /**< (val = 43) */

    /** read data from sensor registers */
    APP_LT_CMD_READ_SENSOR,                    /**< (val = 44) */

/* -----------------  uGuzzi output Sensor   ------------------------ */
    /** store data to uGuzzi output sensor settings */
    APP_LT_CMD_WRITE_GUZZI_SEN,         /**< (val = 45) */

    /** read data from uGuzzi output sensor settings */
    APP_LT_CMD_READ_GUZZI_SEN,          /**< (val = 46) */

    /** store data to uGuzzi output sensor control */
    APP_LT_CMD_WRITE_GUZZI_SEN_CTRL,    /**< (val = 47) */

    /** read data from uGuzzi output sensor control */
    APP_LT_CMD_READ_GUZZI_SEN_CTRL,     /**< (val = 48) */

/* -----------------  uGuzzi output ISP   ------------------------ */
    /** store data to uGuzzi output ISP settings */
    APP_LT_CMD_WRITE_GUZZI_ISP,         /**< (val = 49) */

    /** read data from uGuzzi output  ISP settings */
    APP_LT_CMD_READ_GUZZI_ISP,          /**< (val = 50) */

    /** store data to uGuzzi output ISP control */
    APP_LT_CMD_WRITE_GUZZI_ISP_CTRL,    /**< (val = 51) */

    /** read data from uGuzzi output ISP control */
    APP_LT_CMD_READ_GUZZI_ISP_CTRL,     /**< (val = 52) */

    /** read metadata information */
    APP_LT_CMD_READ_METADATA,           /**< (val = 53) */

    /** store data to uGuzzi metadata control */
    APP_LT_CMD_WRITE_METADATA_CTRL,           /**< (val = 54) */

    /** read data from uGuzzi metadata control */
    APP_LT_CMD_READ_METADATA_CTRL,           /**< (val = 55) */

/* ----------------- DB Version Command ------------------------ */

    /** read DTP DB version structure */
    APP_LT_CMD_GET_DTP_DB_VER,               /**< (val = 56) */


/* ----------------- History Commands ------------------------ */

    /** read history buffer */
    APP_LT_CMD_READ_HISTORY_DATA,            /**< (val = 57) */
    /** read current history status */
    APP_LT_CMD_READ_HISTORY_STATUS,          /**< (val = 58) */
    /** write current history control */
    APP_LT_CMD_WRITE_HISTORY_CTRL,           /**< (val = 59) */
    /** read current history control */
    APP_LT_CMD_READ_HISTORY_CTRL,            /**< (val = 60) */
    /** write history packet */
    APP_LT_CMD_WRITE_HISTORY_PACKET,        /**< (val = 61) */
    /** read history packet */
    APP_LT_CMD_READ_HISTORY_PACKET,         /**< (val = 62) */

/* ----------------- RPC Commands ------------------------ */
    /** write RPC packet */
    APP_LT_CMD_WRITE_RPC_PACKET,            /**< (val = 63) */
    /** read RPC packet */
    APP_LT_CMD_READ_RPC_PACKET,             /**< (val = 64) */
    /** write RPC control */
    APP_LT_CMD_WRITE_RPC_CONTROL,            /**< (val = 65) */
    /** read RPC control */
    APP_LT_CMD_READ_RPC_CONTROL,             /**< (val = 66) */
    /** read RPC control */
    APP_LT_CMD_READ_RPC_STATUS,              /**< (val = 67) */

/* --------------- Custom Region Commands ------------------- */
    /** read CR1 */
    APP_LT_CMD_READ_CR1,             /**< (val = 68) */
    /** write CR1 */
    APP_LT_CMD_WRITE_CR1,            /**< (val = 69) */
    /** write CR1 control */
    APP_LT_CMD_WRITE_CR1_CONTROL,    /**< (val = 70) */
    /** read CR1 control*/
    APP_LT_CMD_READ_CR1_CONTROL,     /**< (val = 71) */
    /** read CR2 */
    APP_LT_CMD_READ_CR2,             /**< (val = 72) */
    /** write CR2 */
    APP_LT_CMD_WRITE_CR2,            /**< (val = 73) */
    /** write CR2 control */
    APP_LT_CMD_WRITE_CR2_CONTROL,    /**< (val = 74) */
    /** read CR2 control*/
    APP_LT_CMD_READ_CR2_CONTROL,     /**< (val = 75) */
    /** read CR3 */
    APP_LT_CMD_READ_CR3,             /**< (val = 76) */
    /** write CR3 */
    APP_LT_CMD_WRITE_CR3,            /**< (val = 77) */
    /** write CR3 control */
    APP_LT_CMD_WRITE_CR3_CONTROL,    /**< (val = 78) */
    /** read CR3 control*/
    APP_LT_CMD_READ_CR3_CONTROL,     /**< (val = 79) */

/* -------------------------------------------------------- */

    /** PLEASE DO NOT WRITE AFTER THAT ENTRY */
    LAST_APP_LT_CMD,           /**< (val = 80) */

} tuning_tool_command_id_t;

/**
 * Responses id returned for Tuning Tool command.
 *
 * Device will send response to every command it receives. The response have
 * the same structure as the commands but the payload will be different. Device
 * will repeat the command ID as response to it. Following enum represent
 * possible responses.
 *
 * \sa tuning_tool_command_id_t
 */
typedef enum
{
    /** APP_GUZZI_RESPONSE_NOP */
    APP_GUZZI_RESPONSE_NOP,

    /** command ID is is successfully executed */
    APP_GUZZI_RESPONSE_SUCCESS,

    /** command ID is not supported */
    APP_GUZZI_RESPONSE_WRONG_COMMAND,

    /** CRC verification fails */
    APP_GUZZI_RESPONSE_CRC_CHECK_FAIL,

    /** DTP checksum sent by the tuning tool differs \n
     *  from the checksum read from the DTP flashed on \n
     *  the device */
    APP_GUZZI_RESPONSE_WRONG_DTP_CHECKSUM,

    /** values of offset and size sent in command exceeds\n
     * the size of target region */
    APP_GUZZI_RESPONSE_WRONG_SIZE,

    /** APP_GUZZI_RESPONSE_MAX */
    APP_GUZZI_RESPONSE_MAX,

    /** Force to 32 bit */
    APP_GUZZI_FORCE_32BIT = INT32_MAX

} tuning_tool_response_id_t;

/**
 * Contains index to refer specific camera/sensor.
 */
typedef enum
{
    CAMERA_IDX_0 = 0,   /**< index for first camera/sensor  */
    CAMERA_IDX_1,       /**< index for second camera/sensor */
    CAMERA_IDX_2,       /**< index for third camera/sensor  */
    CAMERA_IDX_3,       /**< index for fourth camera/sensor */
    CAMERA_IDX_4,       /**< index for fifth camera/sensor */
    CAMERA_IDX_5,       /**< index for sixth camera/sensor */
    CAMERA_IDX_6,       /**< index for seventh camera/sensor */
    CAMERA_IDX_7,       /**< index for eighth camera/sensor */
    MAX_CAMERA_IDX,     /**< Maximum supported camera index */

    CAMERA_FORCE_32BIT = INT32_MAX

} tuning_tool_camera_index_t;


#if ( 8 /*MAX_CAMERA_IDX*/ < (UGUZZI_CAMERA_CNT) )
#error Please Fix Me MAX_CAMERA_IDX is less than UGUZZI_CAMERA_CNT
#endif

/**
 * Specifies access type.
 *
 * Used to specify required access type when reading/writing from/to
 * memory/register region. There could restrictions for access type -
 * usually this applies for registers.
 */
typedef enum
{
    ACCESS_INVALID = 0,
    ACCESS_BYTE  = 1, /**< Byte  access */
    ACCESS_WORD  = 2, /**< Word  access */
    ACCESS_DWORD = 4, /**< Dword access */
    ACCESS_QWORD = 8, /**< Qword access */
    ACCESS_MAX,       /**< used for boundary checks */

    ACCESS_FORCE_32BIT = INT32_MAX

} tuning_tool_access_type_t;


/** type to hold number of elements that will be exchange (read or write) */
typedef uint16_t tuning_tool_usize_t;
#define TUNING_TOOL_USIZE_MAX           UINT16_MAX

/** type to hold offset in region for read or write */
typedef uint32_t tuning_tool_uoffset_t;
#define TUNING_TOOL_UOFFSET_MAX         UINT32_MAX


/**
 * Specifies data transfer direction
 *
 * Used to specify data transfer direction. TT_CMD_WRITE means that
 * Host writes data to uGuzzi, so data will come with command and
 * the only meaningful field in the command response will be status
 * field - \see tuning_tool_response_id_t.
 * TT_CMD_READ means Host wants to read data, so the command doesn't
 * contain data, but the response should. The data will be valid only
 * if the status field is APP_GUZZI_RESPONSE_SUCCESS.
 */
typedef enum
{
    TT_CMD_WRITE  = 1,       /**< Host to uGuzzi */
    TT_CMD_READ   = 2,       /**< uGuzzi to Host */
    TT_CMD_FORCE_32BIT = INT32_MAX
} tuning_tool_direction_t;

/** Internal representation of Tuning Tool command
 *
 *  \sa uguzzi_command_packet_header_t
 */
typedef struct
{
    tuning_tool_command_id_t    cmd_id;       /**< Command id. */
    tuning_tool_direction_t     cmd_dir;      /**< Helper field - indicates data transfer direction. */
    tuning_tool_response_id_t   status;       /**< Status - valid in response. Indicates command result. */
    tuning_tool_camera_index_t  camera;       /**< camera index */
    tuning_tool_access_type_t   access;       /**< Access type - indicates element width in bytes. */
    tuning_tool_usize_t         num_elements; /**< Number of elements that will be transfered. */
    tuning_tool_uoffset_t       offset;       /**< Region offset in bytes. */
} ttc_command_t;

/** @brief Internal representation of command data payload and eventually response
 *
 *  On command is receive the \b p_cmd_data contains pointer to command payload. The
 *  field \c cmd_size_bytes shows how many bytes from payload are valid.
 *  After process of the command module prepare a response and populates
 *  \b p_cmd_resp with data based on command. The \b resp_size_bytes field
 *  gives size of the response.
 *
 *  \sa uguzzi_command_payload_t
 */
typedef struct
{
    uint32_t                  cmd_size_bytes;   /**< number of valid command data payload - in bytes */
    uint32_t                  resp_size_bytes;  /**< number of valid response payload - in bytes */
    uguzzi_command_payload_t *p_cmd_data;       /**< pointer to command data payload */
    uguzzi_command_payload_t *p_cmd_resp;       /**< pointer to command response payload */
} ttc_cmd_payload_t;


/**
 * Numeric value to Camera ID field
 *
 * @param camera [out] Internal representation
 * @param in_val [in]  External command byte
 * @return Zero on success
 */
con_status_t ttc_get_camera(tuning_tool_camera_index_t *camera, uint8_t in_val);


/**
 * Process external command
 *
 * @param hdr  [in/out] external command header
 * @param data [in] command payload
 * @param data_size [in] data payload size
 * @param p_response [out] pointer to response data. Null if there is no data.
 * @param resp_size [out] response data size. Zero if there is no data.
 * @return Zero on success
 */
con_status_t ttc_cmd_process(uguzzi_command_packet_header_t* hdr,
                             uint8_t   data[],
                             uint32_t  data_size,
                             uint8_t  *p_response[],
                             uint32_t *resp_size );


#ifdef __cplusplus
}
#endif

#endif /*TUNING_TOOL_COMMANDS_H__ */

/** @} */
