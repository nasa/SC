/************************************************************************
 * NASA Docket No. GSC-18,924-1, and identified as “Core Flight
 * System (cFS) Stored Command Application version 3.1.1”
 *
 * Copyright (c) 2021 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   This file contains the Stored Command main event loop header
 */
#ifndef SC_APP_H
#define SC_APP_H

/*************************************************************************
 * Includes
 *************************************************************************/

#include "cfe.h"
#include "sc_platform_cfg.h"
#include "sc_tbldefs.h"
#include "sc_msgdefs.h"
#include "sc_msg.h"

/**
 * \brief Wakeup for SC
 *
 * \par Description
 *      Wakes up SC every 1 second for routine maintenance whether a
 *      message was received or not.
 */
#define SC_SB_TIMEOUT 1000

/**
 * \brief Main loop for SC
 *
 *  \par Description
 *       This function is the entry point and main loop for the Stored
 *       Commands (SC) application.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 */
void SC_AppMain(void);

/**
 * \brief Initialize application
 *
 *  \par Description
 *       This function initializes the SC application. The return value
 *       is either CFE_SUCCESS or the error code from the failed cFE
 *       function call. Note that all errors generate an identifying
 *       event message.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
int32 SC_AppInit(void);

/**
 * \brief Initialize application tables
 *
 *  \par Description
 *       This function initializes the SC application tables. The
 *       return value is either CFE_SUCCESS or the error code from the
 *       failed cFE function call. Note that all errors generate an
 *       identifying event message.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
int32 SC_InitTables(void);

/**
 * \brief Register tables with cFE Table Services
 *
 *  \par Description
 *       This function registers all SC tables with cFE Table Services.
 *       The return value is either CFE_SUCCESS or the error code from
 *       the failed cFE function call. Note that all errors generate an
 *       identifying event message.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
int32 SC_RegisterAllTables(void);

/**
 * \brief Get dump only table buffer pointers
 *
 *  \par Description
 *       This function acquires buffer pointers to the dump only tables.
 *       The return value is either CFE_SUCCESS or the error code from
 *       the failed cFE function call. Note that all errors generate an
 *       identifying event message.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
int32 SC_GetDumpTablePointers(void);

/**
 * \brief Get loadable table buffer pointers
 *
 *  \par Description
 *       This function acquires buffer pointers to the loadable tables.
 *       The return value is either CFE_SUCCESS or the error code from
 *       the failed cFE function call. Note that all errors generate an
 *       identifying event message.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
int32 SC_GetLoadTablePointers(void);

/**
 * \brief Load default RTS tables
 *
 *  \par Description
 *       This function loads the default RTS tables. The return value
 *       is either CFE_SUCCESS or the error code from the failed cFE
 *       function call. Note that all errors generate an identifying
 *       event message.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 */
void SC_LoadDefaultTables(void);

/**
 * \brief Register to receive cFE Table Services manage request commands
 *
 *  \par Description
 *       This function provides cFE Table Services with the information
 *       necessary to send a notification command when one of the SC dump
 *       only tables has a dump pending, or when one of the SC loadable
 *       tables has a load pending.  Upon receipt of the command, the
 *       command handler will call the cFE Table Services API function
 *       to manage the table.  This sequence of events ensures that dump
 *       tables are not being updated by SC at the same moment that the
 *       dump occurs, and likewise, that loadable tables are not being
 *       referenced by SC at the moment that the update occurs.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \sa #SC_TableManageCmd
 */
void SC_RegisterManageCmds(void);

/************************************************************************
 * Macro Definitions
 ************************************************************************/

#define SC_BYTES_IN_WORD 4 /**< \brief Number of bytes in "word" used to define table lengths */

#define SC_ATS_HEADER_SIZE (sizeof(SC_AtsEntryHeader_t)) /**< \brief ATS header size in bytes */
#define SC_RTS_HEADER_SIZE (sizeof(SC_RtsEntryHeader_t)) /**< \brief RTS header size in bytes */

#define SC_ATS_BUFF_SIZE32    (SC_ATS_BUFF_SIZE / 2)    /**< \brief ATS buffer number of 32-bit elements */
#define SC_RTS_BUFF_SIZE32    (SC_RTS_BUFF_SIZE / 2)    /**< \brief RTS buffer number of 32-bit elements */
#define SC_APPEND_BUFF_SIZE32 (SC_APPEND_BUFF_SIZE / 2) /**< \brief Append buffer number of 32-bit elements */

#define SC_ERROR -1 /**< \brief SC error return value */

#define SC_CMD_PIPE_NAME "SC_CMD_PIPE" /**< \brief Command pipe name */

/** \brief ATS header + minimum packet size in "words" */
#define SC_ATS_HDR_WORDS ((SC_ATS_HEADER_SIZE + SC_PACKET_MIN_SIZE) / SC_BYTES_IN_WORD)

/** \brief ATS header only size in "words" */
#define SC_ATS_HDR_NOPKT_WORDS (SC_ATS_HEADER_SIZE / SC_BYTES_IN_WORD)

/** \brief RTS header + minimum packet size in "words" */
#define SC_RTS_HDR_WORDS ((SC_RTS_HEADER_SIZE + SC_PACKET_MIN_SIZE) / SC_BYTES_IN_WORD)

/** \brief RTS header only size in "words" */
#define SC_RTS_HDR_NOPKT_WORDS (SC_RTS_HEADER_SIZE / SC_BYTES_IN_WORD)

#define SC_DUP_TEST_UNUSED -1 /**< \brief Unused marking for duplicate test */

#define SC_INVALID_CMD_NUMBER 0xFFFF /**< \brief Invalid command number */

#define SC_ROUND_UP_BYTES 3 /**< \brief Round up to word length (in bytes) */

#define SC_BYTES_IN_ATS_APPEND_ENTRY 2 /**< \brief Bytes in an ATS append table entry */

#define SC_ATS_CMD_NUM_TO_INDEX(num)   ((num)-1)     /**< \brief Convert ATS command number to index */
#define SC_ATS_CMD_INDEX_TO_NUM(index) ((index) + 1) /**< \brief Convert ATS command index to number */
#define SC_ATS_NUM_TO_INDEX(num)       ((num)-1)     /**< \brief Convert ATS table number to index */
#define SC_ATS_INDEX_TO_NUM(index)     ((index) + 1) /**< \brief Convert ATS table index to number */
#define SC_ATS_ID_TO_INDEX(id)         ((id)-1)      /**< \brief Convert ATS ID to index */
#define SC_RTS_NUM_TO_INDEX(num)       ((num)-1)     /**< \brief Convert RTS table number to index */
#define SC_RTS_INDEX_TO_NUM(index)     ((index) + 1) /**< \brief Convert RTS table index to number */
#define SC_RTS_ID_TO_INDEX(id)         ((id)-1)      /**< \brief Convert RTS ID to index */
#define SC_RTS_INDEX_TO_ID(index)      ((index) + 1) /**< \brief Convert RTS table index to ID */

/**
 *  \brief ATS Table Entry Header Type
 */
typedef struct
{
    uint16 Pad; /**< \brief Structure padding */

    uint16 CmdNumber; /**< \brief command number, range = 1 to SC_MAX_ATS_CMDS */

    uint16 TimeTag_MS; /**< \brief Time tag most significant 16 bits */
    uint16 TimeTag_LS; /**< \brief Time tag least significant 16 bits */

    /*
     * Note: the command packet data is variable length,
     *       the command packet header (not shown here),
     *       comes directly after the time tag
     */
} SC_AtsEntryHeader_t;

/**
 * \brief ATS header and message header
 */
typedef struct
{
    SC_AtsEntryHeader_t Header; /**< \brief ATS header */
    CFE_MSG_Message_t   Msg;    /**< \brief MSG header */
} SC_AtsEntry_t;

/**
 * \brief RTS Command Header Type
 */
typedef struct
{
    SC_RelTimeTag_t TimeTag; /**< \brief Relative time tag */

    /*
     * Note: the command packet data is variable length,
     *       the command packet header (not shown here),
     *       comes directly after Time tag.
     */
} SC_RtsEntryHeader_t;

/**
 * \brief RTS header and message header
 */
typedef struct
{
    SC_RtsEntryHeader_t Header; /**< \brief RTS header */
    CFE_MSG_Message_t   Msg;    /**< \brief MSG header */
} SC_RtsEntry_t;

/**
 *  \brief SC Operational Data Structure
 *
 *  This structure contains addresses and handles for loadable and dump-only tables
 *  along with storage for the housekeeping packet.
 */
typedef struct
{
    CFE_SB_PipeId_t CmdPipe; /**< \brief Command pipe ID */

    CFE_TBL_Handle_t AtsTblHandle[SC_NUMBER_OF_ATS]; /**< \brief Table handles for all ATS tables    */
    uint32 *         AtsTblAddr[SC_NUMBER_OF_ATS];   /**< \brief Table Addresses for all ATS tables  */

    CFE_TBL_Handle_t AppendTblHandle; /**< \brief Table handle for Append ATS table   */
    uint32 *         AppendTblAddr;   /**< \brief Table Address for Append ATS table  */

    CFE_TBL_Handle_t RtsTblHandle[SC_NUMBER_OF_RTS]; /**< \brief Table handles for all RTS tables    */
    uint32 *         RtsTblAddr[SC_NUMBER_OF_RTS];   /**< \brief Table addresses for all RTS tables  */

    CFE_TBL_Handle_t   AtsInfoHandle;  /**< \brief Table handle the for ATS Info Table */
    SC_AtsInfoTable_t *AtsInfoTblAddr; /**< \brief Table address for the ATS Info Table*/

    CFE_TBL_Handle_t   RtsInfoHandle;  /**< \brief Table handle for RTS Info Table     */
    SC_RtsInfoEntry_t *RtsInfoTblAddr; /**< \brief Table address for RTS INfo Table    */

    CFE_TBL_Handle_t      RtsCtrlBlckHandle; /**< \brief Table handle for the RTP ctrl block */
    SC_RtpControlBlock_t *RtsCtrlBlckAddr;   /**< \brief Table address for the RTP ctrl block*/

    CFE_TBL_Handle_t      AtsCtrlBlckHandle; /**< \brief Table handle for the ATP ctrl block */
    SC_AtpControlBlock_t *AtsCtrlBlckAddr;   /**< \brief Table address for the ATP ctrl block*/

    CFE_TBL_Handle_t AtsCmdStatusHandle[SC_NUMBER_OF_ATS];  /**< \brief ATS Cmd Status table handle     */
    uint32 *         AtsCmdStatusTblAddr[SC_NUMBER_OF_ATS]; /**< \brief ATS Cmd Status table address    */

    int32 AtsDupTestArray[SC_MAX_ATS_CMDS]; /**< \brief ATS test for duplicate cmd numbers  */

    uint16 NumCmdsSec; /**< \brief the num of cmds that have gone out in a one second period */

    SC_HkTlm_t HkPacket; /**< \brief SC Housekeeping structure */
} SC_OperData_t;

/**
 *  \brief SC Application Data Structure
 *  This structure is used by the application to process time ordered commands.
 */
typedef struct
{
    uint16 AtsTimeIndexBuffer[SC_NUMBER_OF_ATS][SC_MAX_ATS_CMDS];
    /**< \brief  This table is used to keep a time ordered listing
         of ATS command indexes (0 based). The first entry
         in this table holds the command index of the command that will execute
         first, the second entry has the index of the 2nd cmd, etc.. */

    int32 AtsCmdIndexBuffer[SC_NUMBER_OF_ATS][SC_MAX_ATS_CMDS];
    /**< \brief  This table is used to keep a list of ATS table command offsets.
         These offsets correspond to the addresses of ATS commands located in the ATS table.
         The index used is the ATS command index with values from 0 to SC_MAX_ATS_CMDS-1 */

    uint8           NextProcNumber;  /**< \brief the next command processor number */
    SC_AbsTimeTag_t NextCmdTime[2];  /**< \brief The overall next command time  0 - ATP, 1- RTP*/
    SC_AbsTimeTag_t CurrentTime;     /**< \brief this is the current time for SC */
    uint16          Unused;          /**< \brief Unused */
    uint16          AutoStartRTS;    /**< \brief Start selected auto-exec RTS after init */
    uint16          AppendWordCount; /**< \brief Size of cmd entries in current Append ATS table */
} SC_AppData_t;

/************************************************************************
 * Exported Data
 ************************************************************************/
extern SC_AppData_t  SC_AppData;
extern SC_OperData_t SC_OperData;

#endif
