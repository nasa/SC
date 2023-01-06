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
 *   Specification for the CFS Stored Command (SC) command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in sc_msgdefs.h.
 */
#ifndef SC_MSG_H
#define SC_MSG_H

/************************************************************************
 * Includes
 ************************************************************************/
#include <cfe.h>
#include <sc_platform_cfg.h>
#include <sc_msgdefs.h>

/************************************************************************
 * Macro Definitions
 ************************************************************************/

#define SC_NUMBER_OF_RTS_IN_UINT16 16 /**< \brief Number of RTS represented in a uint16 */

/************************************************************************
 * Type Definitions
 ************************************************************************/

/**
 * \defgroup cfssctlm CFS Stored Command Telemetry
 * \{
 */

/**
 *  \brief Housekeeping Packet Structure
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;

    uint8 AtsNumber;                /**< \brief Current ATS number: 1 = ATS A, 2 = ATS B */
    uint8 AtpState;                 /**< \brief Current ATP state: 2 = IDLE, 5 = EXECUTING */
    uint8 ContinueAtsOnFailureFlag; /**< \brief Continue ATS execution on failure flag */

    uint8 CmdErrCtr; /**< \brief Counts Request Errors */
    uint8 CmdCtr;    /**< \brief Counts Ground Requests */
    uint8 Padding8;  /**< \brief Structure padding */

    uint16 SwitchPendFlag;  /**< \brief Switch pending flag: 0 = NO, 1 = YES */
    uint16 NumRtsActive;    /**< \brief Number of RTSs currently active */
    uint16 RtsNumber;       /**< \brief Next RTS number */
    uint16 RtsActiveCtr;    /**< \brief Increments when an RTS is started without error */
    uint16 RtsActiveErrCtr; /**< \brief Increments when an attempt to start an RTS fails */
    uint16 AtsCmdCtr;       /**< \brief Total ATS cmd cnter counts commands sent by the ATS */
    uint16 AtsCmdErrCtr;    /**< \brief Total ATS cmd Error ctr command errors in the ATS */
    uint16 RtsCmdCtr;       /**< \brief Counts TOTAL rts cmds that were sent out from ALL active RTSs */
    uint16 RtsCmdErrCtr;    /**< \brief Counts TOTAL number of errs from ALL RTSs that are active */
    uint16 LastAtsErrSeq;   /**< \brief Last ATS Errant Sequence Num Values: 1 or 2 */
    uint16 LastAtsErrCmd;   /**< \brief Last ATS Errant Command Num */
    uint16 LastRtsErrSeq;   /**< \brief Last RTS Errant Sequence Num */
    uint16 LastRtsErrCmd;   /**< \brief Offset in the RTS buffer for the last command error, in "words" */

    uint16 AppendCmdArg;                   /**< \brief ATS selection argument from most recent Append ATS command */
    uint16 AppendEntryCount;               /**< \brief Number of cmd entries in current Append ATS table */
    uint16 AppendByteCount;                /**< \brief Size of cmd entries in current Append ATS table */
    uint16 AppendLoadCount;                /**< \brief Total number of Append ATS table loads */
    uint32 AtpCmdNumber;                   /**< \brief Current command number */
    uint32 AtpFreeBytes[SC_NUMBER_OF_ATS]; /**< \brief Free Bytes in each ATS  */
    uint32 NextRtsTime;                    /**< \brief Next RTS cmd Absolute Time */
    uint32 NextAtsTime;                    /**< \brief Next ATS Command Time (seconds) */

    uint16 RtsExecutingStatus[(SC_NUMBER_OF_RTS + (SC_NUMBER_OF_RTS_IN_UINT16 - 1)) / SC_NUMBER_OF_RTS_IN_UINT16];
    /**< \brief RTS executing status bit map where each uint16 represents 16 RTS numbers.  Note: array
     index numbers and bit numbers use base zero indexing, but RTS numbers use base one indexing.  Thus,
     the LSB (bit zero) of uint16 array index zero represents RTS number 1, and bit one of uint16 array
     index zero represents RTS number 2, etc.  If an RTS is IDLE, then the corresponding bit is zero.
     If an RTS is EXECUTING, then the corresponding bit is one. */

    uint16 RtsDisabledStatus[(SC_NUMBER_OF_RTS + (SC_NUMBER_OF_RTS_IN_UINT16 - 1)) / SC_NUMBER_OF_RTS_IN_UINT16];
    /**< \brief RTS disabled status bit map where each uint16 represents 16 RTS numbers.  Note: array
     index numbers and bit numbers use base zero indexing, but RTS numbers use base one indexing.  Thus,
     the LSB (bit zero) of uint16 array index zero represents RTS number 1, and bit one of uint16 array
     index zero represents RTS number 2, etc.  If an RTS is ENABLED, then the corresponding bit is zero.
     If an RTS is DISABLED, then the corresponding bit is one. */
} SC_HkTlm_t;

/**\}*/

/**
 * \defgroup cfssccmdstructs CFS Stored Command Command Structures
 * \{
 */

/**
 *  \brief No Arguments Command
 *
 *  For command details see #SC_NOOP_CC, #SC_RESET_COUNTERS_CC, #SC_STOP_ATS_CC, #SC_SWITCH_ATS_CC
 *  Also see #SC_SEND_HK_MID
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command Header */
} SC_NoArgsCmd_t;

/**
 *  \brief ATS Id Command
 *
 *  For command details see #SC_START_ATS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command Header */

    uint16 AtsId;   /**< \brief The ID of the ATS to start, 1 = ATS_A, 2 = ATS_B */
    uint16 Padding; /**< \brief Structure padding */
} SC_StartAtsCmd_t;

/**
 *  \brief RTS Id Command
 *
 *  For command details see #SC_START_RTS_CC, #SC_STOP_RTS_CC, #SC_DISABLE_RTS_CC, #SC_ENABLE_RTS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command Header */

    uint16 RtsId;   /**< \brief The ID of the RTS to start, 1 through #SC_NUMBER_OF_RTS */
    uint16 Padding; /**< \brief Structure padding */
} SC_RtsCmd_t;

/**
 *  \brief Jump running ATS to a new time Command
 *
 *  For command details see #SC_JUMP_ATS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command Header */

    uint32 NewTime; /**< \brief the time to 'jump' to */
} SC_JumpAtsCmd_t;

/**
 *  \brief Continue ATS on failure command
 *
 *  For command details see #SC_CONTINUE_ATS_ON_FAILURE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command Header */

    uint16 ContinueState; /**< \brief true or false, to continue ATS after a failure  */
    uint16 Padding;       /**< \brief Structure Padding */
} SC_SetContinueAtsOnFailureCmd_t;

/**
 *  \brief Append to ATS Command
 *
 *  For command details see #SC_APPEND_ATS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command Header */

    uint16 AtsId;   /**< \brief The ID of the ATS to append to, 1 = ATS_A, 2 = ATS_B */
    uint16 Padding; /**< \brief Structure Padding */
} SC_AppendAtsCmd_t;

#if (SC_ENABLE_GROUP_COMMANDS == true)
/**
 *  \brief RTS Group Command
 *
 *  For command details see #SC_START_RTSGRP_CC, #SC_STOP_RTSGRP_CC, #SC_DISABLE_RTSGRP_CC, #SC_ENABLE_RTSGRP_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command Header */

    uint16 FirstRtsId; /**< \brief ID of the first RTS to act on, 1 through #SC_NUMBER_OF_RTS */
    uint16 LastRtsId;  /**< \brief ID of the last RTS to act on, 1 through #SC_NUMBER_OF_RTS */
} SC_RtsGrpCmd_t;
#endif

/**\}*/

#endif
