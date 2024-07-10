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
* \ingroup gr_uguzzi_connect
* @{
*
* @file uguzzi_commands.h
*
* @brief uGuzzi Command interface
*
* uGuzzi Connect provides a generic protocol for accessing uGuzzi resources.
* So far only the Live Tuning part is implemented.
*/

#ifndef UGUZZI_COMMANDS__H__
#define UGUZZI_COMMANDS__H__

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup gr_uguzzi_connect  uGuzzi Connect
 *
 * uGuzzi Connect provides a generic protocol for accessing uGuzzi's resources. So far only part of
 * Live Tuning is implemented. It is packed as a library named `uguzzi_connect` consisted of
 * multiple submodules:
 * - **uguzzi_commands** provides generic handling for all communication commands supported by
 *                       uGuzzi Connect.
 * - **tuning_tool_commands** handles commands coming from Tuning Tool
 *    - **live_tuning** Handles specific commands related to Live Tuning
 *    - **live_tuning_util** Supplementary library that provides common handling of objects used for
 *                           Live Tuning - such as regions.
 *    - **live_tuning_dtp** DTP specific handling
 *    - **live_tuning_uguzzi_output** Handling of uGuzzi output.
 *    - **live_tuning_isp_regs** Provides access to ISP registers
 *    - **live_tuning_sensor** Provides  access to the sensor's registers
 *    - **live_tuning_uguzzi_history** Provides access history log collected from uGuzzi.
 */


/** maximum payload size that uGuzzi may accept with one command */
#ifdef UGUZZI_CONNECT_COMMAND_MAX_PAYLOAD_SIZE
#define MAX_UGUZZI_COMMAND_PAYLOAD      (UGUZZI_CONNECT_COMMAND_MAX_PAYLOAD_SIZE)
#else
#define MAX_UGUZZI_COMMAND_PAYLOAD      1024u
#endif
/**
 * Common header for all commands to uGuzzi
 *
 */
typedef struct
{
    /** Command ID */
    uint8_t  command_id;

    /** command execution status - valid only in command responses.*/
    uint8_t  command_status;

    /** camera index that command should be applied. If the camera
     * index is not applicable for specific command ID, it should be set
     * to Zero*/
    uint8_t  camera_index;

    /** reserved for future extensions */
    uint8_t  reserved[6];

    /** Element size:
     *  - 1 means byte
     *  - 2 means word
     *  - 4 dword
     *  - 8 qword
     * All other values are forbidden
     */
    uint8_t  element_size;

    /** number of elements that should be exchanged with this command */
    uint16_t num_elements;

    /** offset in region that will be accessed - read/write. Region is
     * specified implicitly by command - ex. sensor, ISP registers etc.
     * Only device (uGuzzi) knows the region's base address */
    uint32_t offset;
} uguzzi_command_packet_header_t;

/**  Data exchange payload
 *
 * Data exchanged with one command. Size is determined by element_size
 * and num_elements in command header. The total \e valid \e data should be:
 *  \f$ payload bytes = num_elements * element_size \f$
 */
typedef struct
{
    /*lint --e{658}*/
    /** Data payload */
    union {
        uint8_t  cmd_payload_uint8 [MAX_UGUZZI_COMMAND_PAYLOAD];
        uint16_t cmd_payload_uint16[MAX_UGUZZI_COMMAND_PAYLOAD / sizeof(uint16_t)];
        uint32_t cmd_payload_uint32[MAX_UGUZZI_COMMAND_PAYLOAD / sizeof(uint32_t)];
        uint64_t cmd_payload_uint64[MAX_UGUZZI_COMMAND_PAYLOAD / sizeof(uint64_t)];
    };
} uguzzi_command_payload_t;

/** \addtogroup app_integration uGuzzi Connect module integration with Camera Application
 *
 *  @brief uGuzzi Connect module integration with Camera application.
 *
 * Because of the importance of being able to configure, control, adjust settings and even debug uGuzzi,
 * running on a real device and environment, a set of supplementary tools was developed. The main tool is a
 * PC application named Tuning Tool&copy; (TT). It generates a DTP database and gives online access to
 * uGuzzi's key features. Also, it may be used in monitor mode, where the user may observe the current uGuzzi
 * parameters/settings and monitor (in normal operation) their change in real time.
 *
 * uGuzzi Connect is a complementary module that sits between uGuzzi and TT. In order to be as less
 * intrusive as possible, Live Tuning module is attached/detached by a set of callbacks. uGuzzi process API
 * provides such hooks that are executed at the beginning/end of `uguzzi_process()`.
 *
 *  \dot
 *  digraph LT_library
 *  {
 *      label="uGuzzi with Live Tuning library stack.";
 *      node [shape=record, style="rounded", fontname=Helvetica, fontsize=11];
 *      graph [nodesep=0.1, ranksep=0.25, pad="0.25", compound=true];
 *
 *      ug  -> lt           [ltail=cluster_lt_ug, arrowhead="none", arrowtail="none", style="bold", color="red" dir="both",  label=" need integration\n(init functions)"]
 *      lt  -> app          [arrowhead="none", arrowtail="none", style="bold", color="red", dir="both", label=" need integration\n (callbacks)"]
 *      app -> link -> TT   [arrowhead="none", arrowtail="none", style="dotted", dir="both"]
 *
 *      subgraph cluster_device
 *      {
 *          label="Camera application"; style=filled; fillcolor=gray;
 *          subgraph cluster_lt_ug
 *          {
 *              label = "uGuzzi"; style=filled; fillcolor="#F0B050";
 *
 *              ug [shape="plaintext", label="   uguzzi_process()"];
 *          }
 *
 *          subgraph cluster_con_lib
 *          {
 *              label="uguzzi_connect"; style=filled; fillcolor="#8FBDA8";
 *
 *              lt  [style=filled, fillcolor="#8FBDA8", label="{<lt>uguzzi_live_tuning|<plat>platform_live_tuning}"];
 *          }
 *
 *          app  [label="application low level\n communication stack"];
 *
 *      }
 *
 *      link [label="Physical Link\n(Serial, Ethernet ... )"];
 *
 *      subgraph cluster_con_TT
 *      {
 *         label="PC"; style=filled; fillcolor="#8FBDA8";
 *
 *         TT   [label="Tuning Tool Application"];
 *      }
 *  }
 *  \enddot
 *
 *  The camera application is expected to ensure a successful connection between uGuzzi's main library and uGuzzi Connect.
 *  After uGuzzi is properly initialized, a call to `uguzzi_live_tuning_init()' is required.
 *  During this call uGuzzi Connect initializes its internal structures and hooks to uGuzzi's
 *  main library.
 *
 *  uGuzzi Connect requires external low level communication that implements physical
 *  communication such as: UART, Ethernet, USB, etc. It is expected that low level
 *  communication provides a reliable exchange of information (packets) between uGuzzi
 *  Connect (running on Device) and host application (Tuning Tool running on PC).
 *  Integration happens through two callback functions.
 *  One callback is for transferring data from host application to uGuzzi Connect and
 *  the other is the other way around - from uGuzzi Connect to host application.
 *  During initialization, the camera application provides a callback
 *  "app_callback". uGuzzi will use it to send data/responses. On
 *  return of initialization, uGuzzi Connect provides to the application an "uguzzi_callback",
 *  that it will use to send (push) data from TT to uGuzzi Connect.
 *
 * \msc
 *  hscale = "1", arcgradient="5";
 *
 *  Application, "uGuzzi Connect";
 *
 *  ||| ;
 *  Application => "uGuzzi Connect" [ label = "uguzzi_init_communication(app_callback)" ];
 *  Application << "uGuzzi Connect" [ label = "return uguzzi_callback" ];
 *  ||| ;
 * \endmsc
 *
 *  The parameters passed to the callbacks are of the same structure for both directions:
 *   - command header
 *   - data payload
 *   - payload size.
 *
 *  Data payload block could be of a significant size. It is expected that the
 *  application will transfer reliably an entire block (header and payload).
 *  If the low level communication protocol requires, it may split payload in
 *  communication dependent packets and add (if required) integrity
 *  verification data (e.g., CRC). The camera application should reassemble
 *  the communication dependent packets in a contiguous buffer before
 *  invoking an uguzzi Connect callback. It should be possible to transfer data
 *  of any size - e.g., images, statistics, meta data, etc. The content of
 *  the payload depends on the command ID located in the command header. The 
 *  command header is always of a fixed size and is expected ( but not mandatory )
 *  to travel first (before data payload). It is the low level communication protocol's responsibility to
 *  collect an entire packet before invoking a callback.
 *
 *  On every command, uGuzzi will send a corresponding response packet with a
 *  status of command execution, and optionally with a data block (payload).
 *
 * \msc
 *  hscale = "1";
 *
 *  "PC Application\nTuning Tool", "Application\nlow level\ncommunication", "uGuzzi\nConnect", "uGuzzi\nLive Tuning";
 *
 *  ||| ;
 *  "PC Application\nTuning Tool"              => "Application\nlow level\ncommunication" [ label = "command" ];
 *  "Application\nlow level\ncommunication"    => "uGuzzi\nConnect"                       [ label = "command" ];
 *  "uGuzzi\nConnect"                          => "uGuzzi\nLive Tuning"                   [ label = "command" ];
 *  "uGuzzi\nLive Tuning"                      => "uGuzzi\nLive Tuning"                   [ label = "process command"];
 *  "uGuzzi\nConnect"                          <= "uGuzzi\nLive Tuning"                   [ label = "response" ];
 *  "Application\nlow level\ncommunication"    <= "uGuzzi\nConnect"                       [ label = "response" ];
 *  "PC Application\nTuning Tool"              <= "Application\nlow level\ncommunication" [ label = "response" ];
 *  "Application\nlow level\ncommunication"    >> "uGuzzi\nConnect"                       [ label = "return from response" ];
 *  "uGuzzi\nConnect"                          >> "uGuzzi\nLive Tuning"                   [ label = "return from response" ];
 *  "uGuzzi\nConnect"                          << "uGuzzi\nLive Tuning"                   [ label = "return from command" ];
 *  "Application\nlow level\ncommunication"    << "uGuzzi\nConnect"                       [ label = "return from command" ];
 *  ||| ;
 * \endmsc
 *
 * Command/response packets have the following structure:
 *  - Packet Header (mandatory field with fixed size of 16 Bytes) Contains common information for all packets
 *  - Payload size  (mandatory field with fixed size of 4 Bytes) Indicates payload size. If this field is 0,
 *                   payload is skipped.
 *  - Packet payload (Optional Size is from 1 up to 1024 Bytes).
 *
 * \dot
 * digraph hist_model
 * {
 *    label="Packet structure.";
 *    newrank=true;
 *    node [fontname=Helvetica, fontsize=11];
 *    graph [rankdir="LR" nodesep=0.1, ranksep=0.25, pad="0.25"];
 *    "comm_packet" [ shape=plaintext label=<
 *                            <table BORDER="0" CELLBORDER="1" CELLSPACING="0">
 *                               <tr> <td COLSPAN="16" bgcolor="#F0B050">Header</td>
 *                                    <td COLSPAN="4"  bgcolor="#E0E0F0">Payload<BR /> Size</td>
 *                                    <td COLSPAN="20" bgcolor="#D0D0D0">                Optional packet payload (1-1024)                </td></tr>
 *                               <tr> <td> 0</td><td> 1</td><td> 2</td><td> 3</td><td> 4</td><td> 5</td><td> 6</td><td> 7</td>
 *                                    <td> 8</td><td> 9</td><td>10</td><td>11</td><td>12</td><td>13</td><td>14</td><td>15</td>
 *                                    <td> 0</td><td> 1</td><td> 2</td><td> 3</td>
 *                                    <td> 0</td><td> 1</td><td>...</td><td>...</td><td>...</td><td>...</td><td>...</td>
 *                                    <td>...</td><td>...</td><td>...</td><td>...</td>
 *                                    <td>...</td><td>...</td><td>...</td><td>...</td>
 *                                    <td>...</td><td>...</td><td>...</td><td>...</td>
 *                                    <td>1023</td></tr>
 *                            </table>
 *                           > ];
 * }
 *  \enddot
 *
 *
 * <center>
 *   **Packet Header fields.**
 *
 *     FIELD NAME  |  TYPE  | SIZE | Comment                                                         |
 *  :-------------:|:------:|:----:|:----------------------------------------------------------------|
 *      command_id |  uint8 |   1  | Command ID                                                      |
 *  command_status |  uint8 |   1  | command execution status ( valid only in\n command responses )  |
 *    camera_index |  uint8 |   1  | camera index that command should be applied.                    |
 *     reserved[0] |  uint8 |   1  | reserved for future extensions                                  |
 *     reserved[1] |  uint8 |   1  | reserved for future extensions                                  |
 *     reserved[2] |  uint8 |   1  | reserved for future extensions                                  |
 *     reserved[3] |  uint8 |   1  | reserved for future extensions                                  |
 *     reserved[4] |  uint8 |   1  | reserved for future extensions                                  |
 *     reserved[5] |  uint8 |   1  | reserved for future extensions                                  |
 *    element_size |  uint8 |   1  | Element size: 1 byte 2 word 4 dword 8 qword                     |
 *    num_elements | uint16 |   2  | number of elements that should be exchanged\n with this command |
 *          offset | uint32 |   4  | offset in region that will be accessed - read/write.            |
 * </center>
 *
 * \note All packet fields are stored in Little-Endian order (least significant byte
 *       first (lowest address)).
 *
 * @{ */

/** \brief Callback prototype for receiving packets from low level communication.
 *
 * With this callback function, the camera application provides new command/data
 * coming from the outside world (e.g., from Tuning Tool). The assumption
 * is that the communication layer is responsible to provide a reliable data exchange:
 *  - validate packet integrity (e.g., CRC check)
 *  - retry packets in case integrity inconstancy is detected
 *  - split data in smaller packets (if required) and reassemble on receiver side.
 *
 * \note This is a blocking function.
 * \sa uguzzi_init_communication \sa uguzzi_command_send_t
 */
typedef int (*uguzzi_command_receive_cb_t)(uguzzi_command_packet_header_t* p_hdr, uint8_t data[], uint32_t data_size);

/**
 * \brief Callback prototype for sending packets through low level communication.
 *
 * With this function uGuzzi provides a response (or new command/data) for transmission to the
 * outside world (e.g., to Tuning Tool). The assumption is that the communication
 * layer (usually implemented by application) is responsible to split data on smaller
 * packets (if require) and reassemble on receiver side.
 *
 * \note This is a blocking function.
 * \sa uguzzi_init_communication \sa uguzzi_command_receive_cb_t
 */
typedef int (*uguzzi_command_send_t)(uguzzi_command_packet_header_t* p_hdr, uint8_t data[], uint32_t data_size);

/**
 * \brief Links uGuzzi Connect with Application communication stack.
 *
 * Callbacks are exchanged between uGuzzi Connect and Low level communication stack.
 *
 * @param p_snd  [in]  Application provided callback pointer, that uGuzzi will
 *                     use to output data packets.
 * @param rcv_cb [out] uGuzzi provides a callback pointer, that communication
 *                     stack will call on receive of data packet.
 * @return zero on Success.
 * \sa uguzzi_command_send_t \sa uguzzi_command_receive_cb_t
 */
int uguzzi_init_communication( uguzzi_command_send_t         p_snd,
                               uguzzi_command_receive_cb_t * rcv_cb );

/** @} */

#ifdef __cplusplus
}
#endif

#endif /*UGUZZI_COMMANDS__H__ */

/** @} */
