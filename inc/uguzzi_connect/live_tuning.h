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
* \defgroup live_tuning Live Tuning handlers
* \ingroup uguzzi_tt_commands
*  @{
*
* \brief
* Implements live tuning commands.
*
* This module provides functions for implementing live tuning commands
* They are mainly used within uGuzzi Connect, but can also be used
* from external programs, by using supported callback hooks.
*
* @file live_tuning.h
*
* @brief Handles all live tuning tool commands.
*/

#ifndef LIVE_TUNING_H__
#define LIVE_TUNING_H__

#ifdef __cplusplus
extern "C" {
#endif

extern struct mmsdbg_dynamic_variable_register mmsdbg_reg_vdl_live_tuning;

struct module_update_context;

/**
 * Live Tuning Region ID
 */
typedef enum
{
    LT_REGION_UGUZZI_MIN              = 63, /**< used for boundary checks */
    LT_REGION_DTP_DB                  = 64, /**< specify DTP database region */
    LT_REGION_ISP_REGS                = 65, /**< specify ISP registers region */
    LT_REGION_SENSOR_REGS             = 66, /**< specify Sensor registers region */
    LT_REGION_UGUZZI_SENSOR           = 67, /**< specify uGuzzi output sensor settings region */
    LT_REGION_UGUZZI_SENSOR_CONTROL   = 68, /**< specify uGuzzi output sensor control region */
    LT_REGION_UGUZZI_ISP              = 69, /**< specify uGuzzi output ISP settings region */
    LT_REGION_UGUZZI_ISP_CONTROL      = 70, /**< specify uGuzzi output ISP control region */
    LT_REGION_UGUZZI_METADATA         = 71, /**< specify uGuzzi output metadata region */
    LT_REGION_UGUZZI_METADATA_CONTROL = 72, /**< specify uGuzzi output metadata control region */
    LT_REGION_DTP_DB_VERSION          = 73, /**< specify DTP DB_VERSION */

    LT_REGION_UGUZZI_HISTORY_DATA     = 74, /**< specify History buffer */
    LT_REGION_UGUZZI_HISTORY_STATUS   = 75, /**< specify History status structure  */
    LT_REGION_UGUZZI_HISTORY_CONTROL  = 76, /**< specify History control structure */
    LT_REGION_UGUZZI_HISTORY_PACKET   = 77, /**< specify History packet structure */

    LT_REGION_RPC_STATUS              = 78, /**< specify RPC status structure  */
    LT_REGION_RPC_CONTROL             = 79, /**< specify RPC control structure */
    LT_REGION_RPC_PACKET              = 80, /**< specify RPC packet structure */

    LT_CUSTOM_REGION_1                 = 81, /**< specify Custom Region 1 */
    LT_CUSTOM_REGION_1_CONTROL         = 82, /**< specify Custom Region 1 Control*/
    LT_CUSTOM_REGION_2                 = 83, /**< specify Custom Region 2 */
    LT_CUSTOM_REGION_2_CONTROL         = 84, /**< specify Custom Region 2 Control*/
    LT_CUSTOM_REGION_3                 = 85, /**< specify Custom Region 3 */
    LT_CUSTOM_REGION_3_CONTROL         = 86, /**< specify Custom Region 3 Control*/

    LT_REGION_UGUZZI_MAX                    /**< used for boundary checks */

}lt_region_types_t;

/**
 * Specify required operation - used for  \b exec_XXXX callbacks
 */
typedef enum
{
    LT_EXEC_READ = 1,   /**< specify READ operation */
    LT_EXEC_WRTE = 2,   /**< specify WRTE operation */
}lt_exec_oper_t;


/**
 * Definition for callback invoked at region data access from Host application.
 *
 * @param ctx pointer to client context provided with \b lt_set_region_callbacks().
 * @return Zero on success otherwise read/write process is aborted and error is
 *         returned to Host application.
 */
typedef con_status_t (*lt_region_update_cb_t) (struct module_update_context* ctx);

/** callback invoked to access uint8_t value module data update */
typedef void (*lt_region_exec_uint8_cb_t)  (struct module_update_context* ctx, lt_exec_oper_t oper, uint32_t offset, uint8_t *value);

/** callback invoked to access uint16_t value module data update */
typedef void (*lt_region_exec_uint16_cb_t) (struct module_update_context* ctx, lt_exec_oper_t oper, uint32_t offset, uint16_t *value);

/** callback invoked to access uint32_t value module data update */
typedef void (*lt_region_exec_uint32_cb_t) (struct module_update_context* ctx, lt_exec_oper_t oper, uint32_t offset, uint32_t *value);

/** callback invoked to access uint64_t value module data update */
typedef void (*lt_region_exec_uint64_cb_t) (struct module_update_context* ctx, lt_exec_oper_t oper, uint32_t offset, uint64_t *value);


/**
 * @brief Structure with callbacks provided (implemented) by application
 *
 * The application may set all or some of the callbacks. They will be invoked
 * during region update. Every region contains set of such callbacks. If the
 * application don't want to receive any callback it may set it as NULL - in
 * this case the default implementation will be used.
 * The default implementation for \b exec_XXXX callbacks is to make a memory
 * read/write based on region description. If the application has some specific
 * requirements (for example access thru kernel driver), it may populate required
 * callback. Typically Application needs only one callback, based on region access
 * type.
 */
typedef struct
{
        /** Pointer to opaque Application specific context.
         * It will be first parameter in callback invocation.
         */
        struct module_update_context   *p_context;

        /**
         * Definition for callback invoked at the begging of region data
         * update - Host application writes in that region (region is still
         * not updated).  If it returns non zero value, write process is
         * aborted and error is returned to Host application.
         *
         * \sa lt_region_read_end_cb_t
         */
        lt_region_update_cb_t           wr_begin;

        /**
         * callback invoked at the begging of region data read - Host
         * application reads from that region. If it returns non zero value,
         * read process is aborted and error is returned to Host application.
         *
         * \sa lt_region_read_end_cb_t
         */
        lt_region_update_cb_t           rd_begin;

        /** callback invoked to access uint8_t value */
        lt_region_exec_uint8_cb_t       exec_uint8;

        /** callback invoked to access uint16_t value */
        lt_region_exec_uint16_cb_t      exec_uint16;

        /** callback invoked to access uint32_t value */
        lt_region_exec_uint32_cb_t      exec_uint32;

        /** callback invoked to access uint64_t value */
        lt_region_exec_uint64_cb_t      exec_uint64;

        /**
         * callback invoked module data write finish - Host application writes
         * in that region (region region data are updated). If it returns non
         * zero error is returned to Host application.
         *
         * \sa lt_region_read_end_cb_t
         */
        lt_region_update_cb_t           wr_end;

        /**
         * callback invoked at when region data was read - Host application reads from
         * that region. If it returns non zero error is returned to Host application.
         *
         * \sa lt_region_read_end_cb_t
         */
        lt_region_update_cb_t           rd_end;

} lt_region_cb_t;


/**
 * Generic region description used for live tuning.
 */
typedef struct
{
    lt_region_cb_t                cb;           /**< a set of callbacks invoked during region update */
    void                         *base_address; /**< region base address */
    tuning_tool_access_type_t     access_type;  /**< access type */
    uint32_t                      region_size;  /**< total size of region */
} lt_region_t;

/**
 * Process Live Tuning commands
 *
 * @param ttc  [in] command coming from tuning tool
 * @param pld  [in/out] command/response payload - response fields
 *                      will be populated accordingly
 * @return Zero on success
 */
con_status_t lt_process_command( ttc_command_t *ttc,
                                 ttc_cmd_payload_t *pld );


/**
 * @brief Set callbacks for live tuning region
 *
 * With this API Application specifies region callbacks. If the default
 * read/write access is not sufficient, the Application may install own
 * callbacks that will be used instead of default implementation that will
 * assume the region is mapped into system memory. If the region represents
 * registers (for example Sensor registers) that are not memory-mapped, the
 * Application should provide APIs for proper read/write.
 * There are two callbacks that will be invoked before/after region update.
 * This may be useful to lock/unlock region access by other threads or some
 * Application specific handling.
 * Each of these callbacks has as a first parameter Application provided
 * context. The context is a pointer to opaque structure (for the live tuning
 * library) defined by Application and its content is Application specific.
 * \note None of the callbacks is mandatory. The unused callback pointers should
 * be leave as a NULL (default).
 *
 * @param camera [in]   [in] camera index
 * @param region_type   [in] region type - DTP, Sensor registers etc.
 * @param p_region_cb   [in] a set with callbacks
 * @return              zero on success
 */
con_status_t lt_set_region_callbacks ( tuning_tool_camera_index_t  camera,
                                       lt_region_types_t           region_type,
                                       lt_region_cb_t             *p_region_cb );

/**
 * @brief Set live tuning region dimensions and access type.
 *
 * With this API Application specifies region characteristics - such as
 * dimensions and access type. Each of supported regions (\see lt_region_types_t)
 * should be properly initialized by Application on order to make live tuning for
 * that region. If the default read/write access is not sufficient, the Application
 * may install own callbacks - \see lt_set_region_callbacks.
 *
 * @param camera        [in] camera index
 * @param region_type   [in] region type - DTP, Sensor registers etc.
 * @param addr          [in] region base address
 * @param region_size   [in] region size
 * @param access        [in] region access type
 * @return              zero on success
 */
con_status_t lt_set_region_params ( tuning_tool_camera_index_t camera,
                                    lt_region_types_t          region_type,
                                    void                      *addr,
                                    uint32_t                   region_size,
                                    tuning_tool_access_type_t  access );


/**
 * Helper function to clear lt_region_cb_t structure.
 *
 * @param p_cb [in] pointer to lt_region_cb_t
 * @return          zero on success
 */
static inline con_status_t lt_region_cb_clear(lt_region_cb_t *p_cb)
{
    con_status_t rc;

    if (NULL != p_cb)
    {
        memset(p_cb, 0, sizeof (lt_region_cb_t));
        rc = CONNECT_SUCCESS;
    } else {
        rc = CONNECT_E_PARAM;
    }

    return rc;
}

/**
 * Configures Live Tuning module with the number of active cameras.
 *
 * @param num_cameras number of active cameras - comes from uGuzzi configuration
 */
void lt_set_active_cameras(uint8_t num_cameras);

/**
 * Gets current Live Tuning configuration for active cameras.
 * @return number of active cameras
 */
uint8_t lt_get_active_cameras(void);


#ifdef __cplusplus
}
#endif

#endif /*LIVE_TUNING_H__ */

/** @} */
