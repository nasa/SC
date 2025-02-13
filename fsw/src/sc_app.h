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
#include "sc_index_types.h"
#include "sc_platform_cfg.h"
#include "sc_tbldefs.h"
#include "sc_msgdefs.h"
#include "sc_msg.h"

/**
 * SC time accessor object
 *
 * Wrapper structure containing a method to get
 * the current reference time.  This reference is
 * used for evaluationg all time references in ATSs
 * and RTSs.
 */
typedef struct SC_TimeAccessor
{
    CFE_TIME_SysTime_t (*GetTime)(void);
} SC_TimeAccessor_t;

/**
 *  \brief ATP Control Block Type
 */
typedef struct
{
    SC_Status_Enum_t AtpState;       /**< \brief execution state of the ATP */
    SC_AtsNum_t      CurrAtsNum;     /**< \brief current ATS running if any */
    uint16           Padding;        /**< \brief Structure padding to align to 32-bit boundaries */
    SC_CommandNum_t  CmdNumber;      /**< \brief current cmd number to run if any */
    SC_SeqIndex_t    TimeIndexPtr;   /**< \brief time index pointer for current cmd */
    uint16           SwitchPendFlag; /**< \brief indicates that a buffer switch is waiting */
} SC_AtpControlBlock_t;

/**
 *  \brief ATS Info Table Type - One of these records are kept for each ATS
 */
typedef struct
{
    uint16 AtsUseCtr;        /**< \brief How many times it has been used */
    uint16 NumberOfCommands; /**< \brief number of commands in the ATS */
    uint32 AtsSize;          /**< \brief size of the ATS */
} SC_AtsInfoTable_t;

/**
 *  \brief RTP Control Block Type
 *
 *  \note Now there is only really one RTP
 *  This structure contains overall info for the next relative time
 *  processor.
 */
typedef struct
{
    uint16      NumRtsActive; /**< \brief number of RTSs currently active */
    SC_RtsNum_t CurrRtsNum;   /**< \brief next RTS number */
} SC_RtpControlBlock_t;

/**
 *  \brief RTS info table entry type -One of these records is kept for each RTS
 */
typedef struct
{
    SC_Status_Enum_t RtsStatus;            /**< \brief status of the RTS */
    bool             DisabledFlag;         /**< \brief disabled/enabled flag */
    uint8            CmdCtr;               /**< \brief Cmds executed in current rts */
    uint8            CmdErrCtr;            /**< \brief errs in current RTS */
    uint32           NextCommandTgtWakeup; /**< \brief target wakeup count for next RTS command */
    SC_EntryOffset_t NextCommandPtr;       /**< \brief where next rts cmd is */
    uint16           UseCtr;               /**< \brief how many times RTS is run */
} SC_RtsInfoEntry_t;

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
CFE_Status_t SC_AppInit(void);

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
CFE_Status_t SC_InitTables(void);

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
CFE_Status_t SC_RegisterAllTables(void);

CFE_Status_t SC_RegisterDumpOnlyTables(void);

CFE_Status_t SC_RegisterLoadableTables(void);

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
CFE_Status_t SC_GetDumpTablePointers(void);

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
CFE_Status_t SC_GetLoadTablePointers(void);

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
 *  \sa #SC_ManageTableCmd
 */
void SC_RegisterManageCmds(void);

/************************************************************************
 * Macro Definitions
 ************************************************************************/

#define SC_BYTES_IN_WORD 4 /**< \brief Number of bytes in "word" used to define table lengths */

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

#define SC_INVALID_CMD_NUMBER ((SC_CommandNum_t) {0}) /**< \brief Invalid command number */

#define SC_ROUND_UP_BYTES 3 /**< \brief Round up to word length (in bytes) */

#define SC_BYTES_IN_ATS_APPEND_ENTRY 2 /**< \brief Bytes in an ATS append table entry */

/**
 * @brief Entry definition for the command status table
 *
 * This table stores the status of commands executed in an ATS
 */
typedef struct SC_AtsCmdStatusEntry
{
    SC_Status_Enum_t Status;
} SC_AtsCmdStatusEntry_t;

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

    CFE_TBL_Handle_t        AtsCmdStatusHandle[SC_NUMBER_OF_ATS];  /**< \brief ATS Cmd Status table handle     */
    SC_AtsCmdStatusEntry_t *AtsCmdStatusTblAddr[SC_NUMBER_OF_ATS]; /**< \brief ATS Cmd Status table address    */

    int32 AtsDupTestArray[SC_MAX_ATS_CMDS]; /**< \brief ATS test for duplicate cmd numbers  */

    uint16 NumCmdsWakeup; /**< \brief the num of cmds that have gone out in this wakeup cycle */

    SC_HkTlm_t HkPacket; /**< \brief SC Housekeeping structure */
} SC_OperData_t;

/**
 * @brief Entry definition for the command entry offset table
 *
 * This table stores the offset of commands within in an ATS
 */
typedef struct SC_AtsCmdEntryOffsetRecord
{
    SC_EntryOffset_t Offset;
} SC_AtsCmdEntryOffsetRecord_t;

/**
 * @brief Entry definition for the command number table
 *
 * This table maps the sequence numbers to command numbers in an ATS
 */
typedef struct SC_AtsCmdNumRecord
{
    SC_CommandNum_t CmdNum;
} SC_AtsCmdNumRecord_t;

/**
 *  \brief SC Application Data Structure
 *  This structure is used by the application to process time ordered commands.
 */
typedef struct
{
    SC_AtsCmdNumRecord_t AtsTimeIndexBuffer[SC_NUMBER_OF_ATS][SC_MAX_ATS_CMDS];
    /**< \brief  This table is used to keep a time ordered listing
         of ATS command indexes (0 based). The first entry
         in this table holds the command index of the command that will execute
         first, the second entry has the index of the 2nd cmd, etc.. */

    SC_AtsCmdEntryOffsetRecord_t AtsCmdIndexBuffer[SC_NUMBER_OF_ATS][SC_MAX_ATS_CMDS];
    /**< \brief  This table is used to keep a list of ATS table command offsets.
         These offsets correspond to the addresses of ATS commands located in the ATS table.
         The index used is the ATS command index with values from 0 to SC_MAX_ATS_CMDS-1 */

    SC_TimeAccessor_t TimeRef; /**< \brief Configured time reference */

    bool EnableHeaderUpdate; /**< \brief whether to update headers in outgoing messages */

    uint32            NextCmdTime[2];     /**< \brief The overall next command time for ATP (0) and command wakeup count for RTP (1) */
    SC_AbsTimeTag_t   CurrentTime;        /**< \brief this is the current time for SC */
    uint32            CurrentWakeupCount; /**< \brief this is the current wakeup count for SC */
    SC_RtsNum_t       AutoStartRTS;       /**< \brief Start selected auto-exec RTS after init */
    uint16            AppendWordCount;    /**< \brief Size of cmd entries in current Append ATS table */
} SC_AppData_t;

/************************************************************************
 * Exported Data
 ************************************************************************/
extern SC_AppData_t  SC_AppData;
extern SC_OperData_t SC_OperData;

/**
 * @brief Locates the Info object associated with the given RTS index
 *
 * @param RtsIndex The RTS index
 * @returns The info object for the given RTS
 */
static inline SC_RtsInfoEntry_t *SC_GetRtsInfoObject(SC_RtsIndex_t RtsIndex)
{
    return &SC_OperData.RtsInfoTblAddr[SC_IDX_AS_UINT(RtsIndex)];
}

/**
 * @brief Locates the Info object associated with the given ATS index
 *
 * @param AtsIndex The ATS index
 * @returns The info object for the given ATS
 */
static inline SC_AtsInfoTable_t *SC_GetAtsInfoObject(SC_AtsIndex_t AtsIndex)
{
    return &SC_OperData.AtsInfoTblAddr[SC_IDX_AS_UINT(AtsIndex)];
}

/**
 * @brief Locates a specific entry within an RTS
 *
 * The entry is specified via its offset (in words) from the start of the RTS
 *
 * @param RtsIndex The RTS index
 * @param EntryOffset The offset from the start of the RTS, in words
 * @returns Pointer to the entry within the RTS
 */
static inline SC_RtsEntry_t *SC_GetRtsEntryAtOffset(SC_RtsIndex_t RtsIndex, SC_EntryOffset_t EntryOffset)
{
    return (SC_RtsEntry_t *)&SC_OperData.RtsTblAddr[SC_IDX_AS_UINT(RtsIndex)][SC_IDX_AS_UINT(EntryOffset)];
}

/**
 * @brief Locates a specific entry within an ATS
 *
 * The entry is specified via its offset (in words) from the start of the ATS
 *
 * @param AtsIndex The ATS index
 * @param EntryOffset The offset from the start of the ATS, in words
 * @returns Pointer to the entry within the ATS
 */
static inline SC_AtsEntry_t *SC_GetAtsEntryAtOffset(SC_AtsIndex_t AtsIndex, SC_EntryOffset_t EntryOffset)
{
    return (SC_AtsEntry_t *)&SC_OperData.AtsTblAddr[SC_IDX_AS_UINT(AtsIndex)][SC_IDX_AS_UINT(EntryOffset)];
}

/**
 * @brief Locates the record that maps the command index to an ATS entry offset
 *
 * @param AtsIndex The ATS index
 * @param CommandIndex The command index
 * @returns Pointer to the SC_AtsCmdEntryOffsetRecord_t object
 */
static inline SC_AtsCmdEntryOffsetRecord_t *SC_GetAtsEntryOffsetForCmd(SC_AtsIndex_t     AtsIndex,
                                                                       SC_CommandIndex_t CommandIndex)
{
    return &SC_AppData.AtsCmdIndexBuffer[SC_IDX_AS_UINT(AtsIndex)][SC_IDX_AS_UINT(CommandIndex)];
}

/**
 * @brief Locates the record that maps the sequence index to an ATS command number
 *
 * @param AtsIndex The ATS index
 * @param SeqIndex The sequence index
 * @returns Pointer to the SC_AtsCmdNumRecord_t object
 */
static inline SC_AtsCmdNumRecord_t *SC_GetAtsCommandNumAtSeq(SC_AtsIndex_t AtsIndex, SC_SeqIndex_t SeqIndex)
{
    return &SC_AppData.AtsTimeIndexBuffer[SC_IDX_AS_UINT(AtsIndex)][SC_IDX_AS_UINT(SeqIndex)];
}

/**
 * @brief Locates the record that holds the status of an ATS command
 *
 * @param AtsIndex The ATS index
 * @param CommandIndex The command index
 * @returns Pointer to the SC_AtsCmdStatusEntry_t object
 */
static inline SC_AtsCmdStatusEntry_t *SC_GetAtsStatusEntryForCommand(SC_AtsIndex_t     AtsIndex,
                                                                     SC_CommandIndex_t CommandIndex)
{
    return &SC_OperData.AtsCmdStatusTblAddr[SC_IDX_AS_UINT(AtsIndex)][SC_IDX_AS_UINT(CommandIndex)];
}

#endif
