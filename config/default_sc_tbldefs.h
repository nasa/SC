/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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
 *   Specification for the CFS Stored Command (SC) table related
 *   constant and structure definitions.
 *
 */
#ifndef DEFAULT_SC_TBLDEFS_H
#define DEFAULT_SC_TBLDEFS_H

#include "common_types.h"
#include "sc_extern_typedefs.h"

/************************************************************************
 * Type Definitions
 ************************************************************************/

/**
 *  \brief Absolute Value time tag for ATC's
 */
typedef uint32 SC_AbsTimeTag_t;

/**
 *  \brief Relative wakeup count
 */
typedef uint32 SC_RelWakeupCount_t;

/**
 * @brief A sequence number for commands.
 *
 * This value is intended to indicate the order with which commands
 * are executed within an ATS
 *
 * @note The command number in ATS entries is strictly for command
 * identification and referencing purposes.  Commands do not need to
 * appear in the table, nor be do they need to be numbered in order.
 *
 * For example, the command with SeqIndex == 3 will be the 4th command executed
 * within the ATS, after sorting by time.
 *
 * The valid range is [0..SC_MAX_ATS_CMDS-1]
 */
typedef uint16 SC_SeqIndex_t;

/**
 *  \brief ATS Table Entry Header Type
 */
typedef struct
{
    uint16 Pad; /**< \brief Structure padding */

    SC_CommandNum_t CmdNumber; /**< \brief command number, range = 1 to SC_MAX_ATS_CMDS */

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
    SC_AtsEntryHeader_t     Header; /**< \brief ATS header */
    CFE_MSG_CommandHeader_t Msg;    /**< \brief Command Message to be sent */
} SC_AtsEntry_t;

/**
 * \brief RTS Command Header Type
 */
typedef struct
{
    SC_RelWakeupCount_t WakeupCount; /**< \brief Relative wakeup count */

    /*
     * Note: the command packet data is variable length,
     *       the command packet header (not shown here),
     *       comes directly after WakeupCount.
     */
} SC_RtsEntryHeader_t;

/**
 * \brief RTS header and message header
 */
typedef struct
{
    SC_RtsEntryHeader_t     Header; /**< \brief RTS header */
    CFE_MSG_CommandHeader_t Msg;    /**< \brief Command Message to be sent */
} SC_RtsEntry_t;

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

#endif
