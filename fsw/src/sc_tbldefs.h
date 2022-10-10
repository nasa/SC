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
 *   Specification for the CFS Stored Command (SC) table structures
 */
#ifndef SC_TBLDEFS_H
#define SC_TBLDEFS_H

/*************************************************************************
 * Includes
 *************************************************************************/
#include "cfe.h"
#include "sc_platform_cfg.h"

/**
 * \defgroup cfscstblids ID definitions for cFE Table Services manage table request command
 * \{
 */
#define SC_TBL_ID_ATS_0     (1)                                  /**< \brief ATS 0 Table ID */
#define SC_TBL_ID_APPEND    (SC_TBL_ID_ATS_0 + SC_NUMBER_OF_ATS) /**< \brief Append Table ID */
#define SC_TBL_ID_RTS_0     (SC_TBL_ID_APPEND + 1)               /**< \brief RTS 0 Table ID */
#define SC_TBL_ID_RTS_INFO  (SC_TBL_ID_RTS_0 + SC_NUMBER_OF_RTS) /**< \brief RTS Info Table ID */
#define SC_TBL_ID_RTP_CTRL  (SC_TBL_ID_RTS_INFO + 1)             /**< \brief RTS Control Table ID */
#define SC_TBL_ID_ATS_INFO  (SC_TBL_ID_RTP_CTRL + 1)             /**< \brief ATS Info Table ID */
#define SC_TBL_ID_ATP_CTRL  (SC_TBL_ID_ATS_INFO + 1)             /**< \brief ATS Control Table ID */
#define SC_TBL_ID_ATS_CMD_0 (SC_TBL_ID_ATP_CTRL + 1)             /**< \brief ATS 0 Command Table ID */
/**\}*/

/************************************************************************
 * Type Definitions
 ************************************************************************/

/**
 *  \brief Absolute Value time tag for ATC's
 */
typedef uint32 SC_AbsTimeTag_t;

/**
 *  \brief Relative time tag for RTC's
 */
typedef uint32 SC_RelTimeTag_t;

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
 *  \brief ATP Control Block Type
 */
typedef struct
{
    uint8  AtpState;       /**< \brief execution state of the ATP */
    uint8  AtsNumber;      /**< \brief current ATS running if any */
    uint32 CmdNumber;      /**< \brief current cmd number to run if any */
    uint16 TimeIndexPtr;   /**< \brief time index pointer for current cmd */
    uint16 SwitchPendFlag; /**< \brief indicates that a buffer switch is waiting */
} SC_AtpControlBlock_t;

/**
 *  \brief RTP Control Block Type
 *
 *  \note Now there is only really one RTP
 *  This structure contains overall info for the next relative time
 *  processor.
 */
typedef struct
{
    uint16 NumRtsActive; /**< \brief number of RTSs currently active */
    uint16 RtsNumber;    /**< \brief next RTS number */
} SC_RtpControlBlock_t;

/**
 *  \brief RTS info table entry type -One of these records is kept for each RTS
 */
typedef struct
{
    uint8           RtsStatus;       /**< \brief status of the RTS */
    bool            DisabledFlag;    /**< \brief disabled/enabled flag */
    uint8           CmdCtr;          /**< \brief Cmds executed in current rts */
    uint8           CmdErrCtr;       /**< \brief errs in current RTS */
    SC_AbsTimeTag_t NextCommandTime; /**< \brief next command time for RTS */
    uint16          NextCommandPtr;  /**< \brief where next rts cmd is */
    uint16          UseCtr;          /**< \brief how many times RTS is run */
} SC_RtsInfoEntry_t;

#endif
