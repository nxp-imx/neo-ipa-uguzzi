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
/** \ingroup uguzzi_live_tuning
* @file uguzzi_live_tuning.h
*
* @brief Live tuning implementation for DTP database, uGuzzi output and History.
*
* Implements Live tuning for:
*  - DTP database - provides direct interface for read/write access.
*  - uGuzzi output
*      - sensor settings
*      - ISP settings
*      - metadata produced during this processing
*  - History - provides logging capabilities for uGuzzi
*
*  For DTP database it registers a region in uGuzzi Connect. Thus uGuzzi Connect
*  may read/write directly parts from/to DTP database while system runs.
*  The Tuning Tool may modify on-the-fly tuning data for specific algorithm.
*  The effect may be observed on next (frame) uGuzzi process.
*  The other option is Tuning Tool to read/write decisions taken form uGuzzi.
*  uGuzzi process generates settings for Sensor(s) and ISP settings. For each
*  settings a mirror variable is instantiated.
*/

#ifndef HDR_UGUZZI_LIVE_TUNING_H__
#define HDR_UGUZZI_LIVE_TUNING_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup uguzzi_live_tuning uGuzzi Live Tuning
 * @{
 *
 * uGuzzi is a framework (packet into library) that generates all settings required for creation of arbitrary
 * camera application. It supports multiple cameras running simultaneously. It's input/output is highly
 * customizable for target ISP and application specific.
 *
 * The main uGuzzi API is 'uguzzi_process()'. It expects all available statistics gathered by application as input
 * and produces output settings for the system - ISP, sensor, application etc.
 *
 *  \dot
 *  digraph ug_process_normal
 *  {
 *      node [shape=record, fontname=Helvetica, fontsize=10];
 *      label = "uGuzzi normal operation"
 *
 *      in_sen -> ug_process;
 *      in_hist -> ug_process;
 *      in_awb -> ug_process
 *      ug_process -> out_sen;
 *      ug_process -> out_isp;
 *      ug_process -> out_meta;
 *
 *      subgraph cluster_uguzzi_input {
 *          label = "uGuzzi Input";
 *          subgraph dot_uguzzi_input {
 *              {rank=same in_sen in_hist in_awb}
 *              in_sen  [ label = "Sensor\nsettings" ];
 *              in_hist [ label = "Histograms" ];
 *              in_awb  [ label = "AWB\nstatistics" ];
 *          }
 *      }
 *
 *      ug_process [shape=plaintext, label="uguzzi_process()", fontsize=16];
 *
 *      subgraph cluster_uguzzi_output
 *      {
 *         label = "uGuzzi Output";
 *         subgraph dot_uguzzi_output {
 *            {rank=same out_sen out_isp out_meta}
 *            out_sen  [ label = "Sensor\nsettings" ];
 *            out_isp [ label = "ISP settings" ];
 *            out_meta  [ label = "Metadata" ];
 *         }
 *      }
 *
 *   }
 *  \enddot
 *
 *  Usually application executes `uguzzi_process()` for every incoming frame from the sensor. uGuzzi processes
 *  input data and evaluates best settings to be applied on the next frame. In this process uGuzzi uses
 *  a Dynamic Tuning Parameters&copy; (DTP) database that contains tuning parameters for every algorithm inside
 *  uGuzzi. By using such database uGuzzi becomes very flexible and adjustable. The DTP is separate from
 *  uGuzzi, so it could be modified and updated without affecting uGuzzi executable.
 *
 * Because it is very important to be able to configure, control, adjust settings and even debug uGuzzi
 * running on real device and environment, a set of supplementary tools was developed. The main tool is a
 * PC application named Tuning Tool&copy; (TT). It generates DTP database, gives online access access to
 * uGuzzi key features. If available (application/OS dependent) access to ISP registers, Sensor registers.
 * Also it may be used in monitor mode where user may observe current uGuzzi parameters/settings and monitor
 * their change in time during normal operation.
 *
 * Live Tuning is a complementary module that sits between uGuzzi and TT. In order to be as much less
 * intrusive as possible, Live Tuning module is attached/detached by set of callbacks. uGuzzi process API
 * has such hooks that are executed at the beginning/end of `uguzzi_process()`.
 *
 *  \dot
 *  digraph ug_process_normal
 *  {
 *      node [shape=record, fontname=Helvetica, fontsize=10];
 *      label = "uGuzzi with Live Tuning attached";
 *      graph [nodesep=0.1, ranksep=0.25, pad="0.25"];

 *      in_sen -> ug_process_begin_cb;
 *      in_hist -> ug_process_begin_cb;
 *      in_awb -> ug_process_begin_cb;
 *      ug_process_begin_cb -> lt_process_begin [style="dotted", dir="both", constraint=false];
 *      ug_process_begin_cb -> ug_process -> ug_process_end_cb;
 *      ug_process_end_cb -> lt_process_end [style="dotted", dir="both", constraint=false];
 *      ug_process_end_cb -> out_sen;
 *      ug_process_end_cb -> out_isp;
 *      ug_process_end_cb -> out_meta;
 *
 *      lt_process_begin -> lt_process_end [arrowhead="open", arrowtail="open", style="invis"]
 *
 *
 *      subgraph cluster_uguzzi_input
 *      {
 *          label = "uGuzzi Input";
 *          subgraph dot_uguzzi_input {
 *              {rank=same in_sen in_hist in_awb}
 *              in_sen  [ label = "Sensor\nsettings" ];
 *              in_hist [ label = "Histograms" ];
 *              in_awb  [ label = "AWB\nstatistics" ];
 *          }
 *      }
 *
 *      newrank=true;
 *      { rank=same; ug_process_begin_cb; lt_process_begin; }
 *      { rank=same; ug_process_end_cb; lt_process_end; }
 *
 *     subgraph cluster_ug_with_CB
 *     {
 *         label="";
 *         ug_process_begin_cb [shape="plaintext", label="ug_process_begin_cb()", fontsize=14];
 *         ug_process [shape=plaintext, label="uguzzi process body", fontsize=14];
 *         ug_process_end_cb [shape="plaintext", label="ug_process_end_cb()", fontsize=14];
 *     }
 *
 *     subgraph cluster_LT
 *     {
 *         label="Live Tuning Library";
 *         style="rounded"
 *         lt_process_begin [shape="plaintext", label="lte_uguzzi_process_begin()", fontsize=14];
 *         lt_process_end   [shape="plaintext", label="lte_uguzzi_process_end()", fontsize=14];
 *     }
 *
 *     subgraph cluster_uguzzi_output
 *     {
 *        label = "uGuzzi Output";
 *        subgraph dot_uguzzi_output {
 *           {rank=same out_sen out_isp out_meta}
 *           out_sen  [ label = "Sensor\nsettings" ];
 *           out_isp [ label = "ISP settings" ];
 *           out_meta  [ label = "Metadata" ];
 *        }
 *     }
 *   }
 *  \enddot
 *
 *  In this way Live Tuning may observe and/or change all input/output `uguzzi_process()` data.
 *  - On begin callback Live Tuning may access input parameters.
 *  - On end callback it may access output parameters
 *
 *  Live tuning connects with Tuning Tool thru `uguzzi_connect` library that handles communication
 *  protocol - in that case commands related to live tuning.
 *
 *  \dot
 *  digraph LT_library
 *  {
 *      label="uGuzzi with Live Tuning library stack."
 *      node [shape=record, style="rounded", fontname=Helvetica, fontsize=11];
 *      graph [nodesep=0.1, ranksep=0.25, pad="0.25"];
 *
 *      ug      -> lt:lt                     [arrowhead="none", arrowtail="none", style="invisible", dir="both"]
 *      lt:plat -> ucmd -> app -> link -> TT [arrowhead="none", arrowtail="none", style="invisible", dir="both"]
 *
 *      subgraph cluster_device
 *      {
 *          label="Camera application"
 *          style=filled;
 *          fillcolor=gray;
 *
 *          subgraph cluster_lt_ug
 *          {
 *              label = "uGuzzi";
 *              style=filled;
 *              fillcolor="#F0B050";
 *
 *              ug [shape="plaintext", label="uguzzi_process()"]
 *          }
 *
 *          lt  [style=filled, fillcolor="#8FBDA8", label="{<lt>uguzzi_live_tuning|<plat>platform_live_tuning}"];
 *
 *          subgraph cluster_con_lib
 *          {
 *              label="  uguzzi_connect  "
 *              style=filled;
 *              fillcolor="#8FBDA8";
 *
 *              ucmd [label="Live Tuning\ncommands"];
 *          }
 *
 *          app  [label="application low\nlevel communication"];
 *      }
 *
 *      link [label="Physical Link\n(Serial, Ethernet ... )"];
 *
 *      subgraph cluster_con_lib
 *      {
 *        label="PC";
 *        style=filled;
 *        fillcolor="#8FBDA8";
 *
 *          TT   [label="Tuning Tool Application"];
 *      }
 *
 *  }
 *  \enddot
 *
 * uGuzzi Built-in support for Live Tuning
 * =======================================
 * uGuzzi connect provides support to populate predefined regions in Device (application) memory. Thus application
 * should provide real memory regions - start address and size are of each region is application specific. The content
 * and layout of each region is well known to application and Tuning Tool (TT). For uGuzzi connect library the region
 * is opaque structure. It just provides link between device application and Tuning Tool. For regions that are related
 * to uGuzzi we know everything, so we may create a built-in support for such regions. The `uguzzi_live_tuning` library
 * provides required data regions and implements supporting functions for:
 *  - DTP database
 *  - ISP output
 *  - Sensor output
 *  - Metadata
 *
 *  For each of above regions we provide memory regions which layout is predefined from uGuzzi interface and is well
 *  known by Tuning Tool. Even that layout is platform (ISP) specific we may handle data structures by inherit their
 *  definitions from uGuzz interface - for details see uGuzzi platform dependent interface file uguzzi_isp_out.h. Thus
 *  uGuzzi adjusts its output structures (layout) based on build target and we use uGuzzi interface include files, we
 *  may handle them without real dependency from target ISP. In a very less cases where we need some platform specific
 *  code, so such functions are implemented from external library named `platform_live_tunning` which is platform specific.
 *
 *  `uguzzi_live_tuning` library also contains submodule `uguzzi_history` that extends built-in support for Live tuning
 *  with capabilities for logging arbitrary information for long period of time.
 *
 *  For more details see: \ref uguzzi_history "uGuzzi history"
 *
 *  DTP database
 *  ------------
 *
 *  Live tuning DTP database requires only two regions:
 *  - `LT_REGION_DTP_DB` The region characteristics is taken from exported initial uGuzzi configuration structure. It
 *     is returned by `uguzzi_get_init_configuration()`. This structure has following fields:
 *      - 'dtp_database'      - start address of database in use. It may be external or built-in uGuzzi.
 *      - 'dtp_database_size' - region size.
 *
 *  \note TT has read/write access to that region.
 *
 *  - `LT_REGION_DTP_DB_VERSION` - region that contains loaded DTP version data. The region characteristics is taken from
 *     exported from uGuzzi by `uguzzi_get_dtp_db_version()`. The size is equal to `uguzzi_dtp_db_version_t`. The content
 *     is from type `uguzzi_dtp_db_version_t`. Purpose of that region is to provide SHAID of DTP in use. TT uses it to
 *     verify that DTP in TT is equal to the used by uGuzzi. Only if both DTPs are equal, the live tuning (modification)
 *     of DTP is allowed - TT may safely modify tuning data in DTP.
 *
 *  \note TT has read only access to that region.
 *
 *  uGuzzi output
 *  -------------
 *  On each `uguzzi_process()` a new set of ISP, Sensor settings and metadata is prepared for each active cameras. In order
 *  to provide dynamic live tuning for uGuzzi output we provide a mirror variable for each output type (ISP, sensor etc.).
 *  Additionally we split every type for active cameras. For example ISP output we have:
 *
 *
 * <center>
 *  CAMERA |          ISP output region      | Mirror Variable region | Mirror control region |
 *  :-----:|:-------------------------------:|:----------------------:|:---------------------:|
 *     0   |  isp_settings_pkg.isp_config[0] |  lte_isp_settings[0]   |    lte_isp_ctrl[0]    |
 *     1   |  isp_settings_pkg.isp_config[0] |  lte_isp_settings[1]   |    lte_isp_ctrl[1]    |
 *     2   |  isp_settings_pkg.isp_config[0] |  lte_isp_settings[2]   |    lte_isp_ctrl[2]    |
 *     3   |  isp_settings_pkg.isp_config[0] |  lte_isp_settings[3]   |    lte_isp_ctrl[3]    |
 * </center>
 *
 *
 * For every ISP output settings and for every camera, we have separate couple of variables (regions):
 * - mirror variable
 * - control variable
 *
 * In control variable (region) we have two flags that should be set by Tuning Tool (for example see \link lte_uguzzi_out_isp_control_t \endlink).
 * - update flag. if it is set is uGuzzi output (for corresponding camera) is copied to mirror variable. TT may read
 *                 mirror variable to obtain current uGuzzi setting. Based on update flag value there are two possible
 *                 cases:
 *        - update flag = 1 means that after `uguzzi_process()` prepares ISP output it will be copied (once) to mirror variable
 *                          and update flag will be reset to Zero. TT may wait (read) this flag to go to Zero in order to
 *                          be sure the mirror variable was updated and contains valid data.
 *        - update flag = 2 this is continuous update mode. Update flag will not be modified after mirror variable update, so
 *                          it will continue update mirror variable for every `uguzzi_process()`. TT may read periodically to
 *                          update UI.
 *
 *  \dot
 *  digraph LT_update
 *  {
 *    node [shape=record, fontname=Helvetica, fontsize=10];
 *    rankdir=LR;
 *    b [ label="uGuzzi\noutput" ];
 *    c [ label="mirror\nvariable" ];
 *
 *    b -> c [ arrowhead="open", style="dashed" label=" If update flag is set "];
 *  }
 *  \enddot
 *
 * - control flag. If it is set after each `uguzzi_process()` the uGuzzi output will be overwritten from mirror variable. In this
 *                 mode TT may modify content of mirror variable and that will affect directly uGuzzi output - in our example ISP
 *                 settings for corresponding camera.
 *
 * *  \dot
 *  digraph LT_control {
 *  node [shape=record, fontname=Helvetica, fontsize=10];
 *    rankdir=LR;
 *
 *  b [ label="uGuzzi\noutput" ];
 *  c [ label="mirror\nvariable" ];
 *  b -> c [ arrowtail="open", style="dashed", dir="back", label=" If control flag is set " ];
 *  }
 *  \enddot
 *
 * \note: NOTE: Setting both flag is not anticipated and if happen the system behavior is not defined and is implementation
 *              specific.
 *
 * With help of above two flags following major use cases are achieved:
 * - Manual Live Tuning.
 *      - User starts camera.
 *      - uGuzzi calculates ISP output settings. For a few frames uGuzzi calculates close to optimal settings for given light
 *        conditions (scene).
 *      - From TT user sets update flag to 1 (and clear control flag). On next uGuzzi process the output will be copied to
 *        mirror variable. This effectively "locks" output in mirror variable.
 *      - TT waits (automatically) update flag to go to zero. This is indication that in mirror variable we have current (valid)
 *        values. TT Reads mirror variable to update UI with current values.
 *      - User sets (by TT UI) control flag to 1. This will effectively overwrite every uGuzzi output from mirror variable. User
 *        may modify mirror variable with new settings. This will have effect on next `uguzzi_process()`.
 * This mode is very useful to evaluate better settings in fixed scene.
 *
 * - Monitoring mode.
 *      - User starts camera.
 *      - From TT user sets update flag to 2 (and clear control flag). On every uGuzzi process the output will be copied to
 *        mirror variable.
 *      - Periodically read (automatically or manually) mirror variable and update UI
 * This mode is very useful to observe uGuzzi decisions in dynamic scene.
 *
 *  ISP output
 *  ----------
 *  To provide Live Tuning functionality we need two regions per camera `uguzzi_live_tuning` library provides:
 *  - `lte_isp_settings[LTE_GUZZI_CAMERA_COUNT_MAX]` mirror variable for ISP settings
 *  - `lte_isp_ctrl[LTE_GUZZI_CAMERA_COUNT_MAX]` control variable for ISP settings
 *
 * `uguzzi_live_tuning` library also implements required logic for copying forth and back of ISP settings based on control flags.
 *
 *  Sensor output
 *  -------------
 *  To provide Live Tuning functionality we need two regions per camera `uguzzi_live_tuning` library provides:
 *  - `lte_sensor_channel_settings[LTE_GUZZI_CAMERA_COUNT_MAX]` mirror variable for Sensor settings
 *  - `lte_sen_ctrl[LTE_GUZZI_CAMERA_COUNT_MAX]` control variable for Sensor settings
 *
 * `uguzzi_live_tuning` library also implements required logic for copying forth and back of Sensor settings based on control flags.
 *
 *
 *  Metadata output
 *  -------------
 *  To provide Live Tuning functionality we need two regions per camera `uguzzi_live_tuning` library provides:
 *  - `lte_metadata_settings[LTE_GUZZI_CAMERA_COUNT_MAX]` mirror variable for Metadata
 *  - `lte_metadata_ctrl[LTE_GUZZI_CAMERA_COUNT_MAX]` control variable for Metadata
 *
 * `uguzzi_live_tuning` library also implements required logic for copying forth and back of Metadata based on control flags.
 *
 *
 *
 *
 */

/** \ingroup app_integration
 * \brief Initializes internal structures and hooks to uGuzzi.
 *
 * Initializes built-in support for uGuzzi DTP, ISP output, Sensor output
 * metadata and History
 *
 * @return Zero on success
 */
int uguzzi_live_tuning_init (void);

/** \ingroup app_integration
 * \brief Detaches from uGuzzi and free resources.
 *
 * Finalizes built-in support for uGuzzi DTP, ISP output, Sensor output,
 * History and metadata
 *
 * @return Zero on success
 */
int uguzzi_live_tuning_deinit (void);

#ifdef __cplusplus
}
#endif

#endif /* HDR_UGUZZI_LIVE_TUNING_H__ */

/** @} */
