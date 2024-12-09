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
 *   message constant definitions.
 *
 *  For SC this is only the function/command code definitions
 */
#ifndef SC_MSGDEFS_H
#define SC_MSGDEFS_H

#include "common_types.h"
#include "sc_extern_typedefs.h"
#include "sc_fcncodes.h"

/************************************************************************
 * Macro Definitions
 ************************************************************************/

#define SC_NUMBER_OF_RTS_IN_UINT16 16 /**< \brief Number of RTS represented in a uint16 */

/**
 * ATS/RTS Cmd Status Enumeratoion
 */
enum SC_Status
{
    SC_Status_EMPTY,           /**< \brief the object is not loaded */
    SC_Status_LOADED,          /**< \brief the object is loaded */
    SC_Status_IDLE,            /**< \brief the object is not executing */
    SC_Status_EXECUTED,        /**< \brief the object has completed executing */
    SC_Status_SKIPPED,         /**< \brief the object (ats command) was skipped */
    SC_Status_EXECUTING,       /**< \brief the object is currently executing */
    SC_Status_FAILED_CHECKSUM, /**< \brief the object failed a checksum test */
    SC_Status_FAILED_DISTRIB,  /**< \brief the object could not be sent on the SWB */
    SC_Status_STARTING         /**< \brief used when an inline switch is executed */
};

typedef uint8 SC_Status_Enum_t;

#ifndef SC_OMIT_DEPRECATED
/**
 * \name Old-style ATS/RTS Cmd Status macros
 * \{
 */
#define SC_EMPTY           SC_Status_EMPTY
#define SC_LOADED          SC_Status_LOADED
#define SC_IDLE            SC_Status_IDLE
#define SC_EXECUTED        SC_Status_EXECUTED
#define SC_SKIPPED         SC_Status_SKIPPED
#define SC_EXECUTING       SC_Status_EXECUTING
#define SC_FAILED_CHECKSUM SC_Status_FAILED_CHECKSUM
#define SC_FAILED_DISTRIB  SC_Status_FAILED_DISTRIB
#define SC_STARTING        SC_Status_STARTING
/**\}*/
#endif

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/**
 * Enumeration for SC processes
 * This specifies which process runs next
 */
enum SC_ProcessNum
{
    SC_Process_ATP  = 0,   /**< \brief ATP process next */
    SC_Process_RTP  = 1,   /**< \brief RTP process next */
    SC_Process_NONE = 0xFF /**< \brief No pending process */
};

typedef uint8 SC_Process_Enum_t;

#ifndef SC_OMIT_DEPRECATED
/**
 * \name Old-style defines for which process runs next
 * \{
 */
#define SC_ATP  SC_Process_ATP
#define SC_RTP  SC_Process_RTP
#define SC_NONE SC_Process_NONE
/**\}*/
#endif

#define SC_MAX_TIME 0xFFFFFFFF /**< \brief Maximum time in SC */
#define SC_MAX_WAKEUP_CNT 0xFFFFFFFF /**< \brief Maximum wakeup count in SC */

/**
 * Enumeration for ATS identifiers
 *
 * ATS identifiers are alphabetic letters that correspond to ATS numbers
 */
enum SC_AtsId
{
    SC_AtsId_NO_ATS, /**<\ brief No ATS */
    SC_AtsId_ATSA,   /**< \brief ATS A */
    SC_AtsId_ATSB    /**< \brief ATS B */
};

typedef uint8 SC_AtsId_Enum_t;

#ifndef SC_OMIT_DEPRECATED
/**
 * \name Old-style defines for each ATS
 * \{
 */
#define SC_NO_ATS SC_AtsId_NO_ATS
#define SC_ATSA   SC_AtsId_ATSA
#define SC_ATSB   SC_AtsId_ATSB
/**\}*/
#endif

/**
 * Enumeration of config parameters for which time reference to use
 */
enum SC_TimeRef
{
    SC_TimeRef_USE_CFE_TIME, /**< \brief Use cFE configured time */
    SC_TimeRef_USE_TAI,      /**< \brief Use TAI Time */
    SC_TimeRef_USE_UTC,      /**< \brief USE UTC Time */
    SC_TimeRef_MAX
};

typedef uint8 SC_TimeRef_Enum_t;

#ifndef SC_OMIT_DEPRECATED
/**
 * \name Old-style constants for config parameters for which TIME to use
 * \{
 */
#define SC_USE_CFE_TIME SC_TimeRef_USE_CFE_TIME
#define SC_USE_TAI      SC_TimeRef_USE_TAI
#define SC_USE_UTC      SC_TimeRef_USE_UTC
/**\}*/
#endif

#define SC_INVALID_RTS_NUMBER 0 /**< \brief Invalid RTS number */

/**
 * SC Continue After Failure Enumeration
 */
enum SC_AtsCont
{
    SC_AtsCont_FALSE = false, /**< \brief Do not continue on failure */
    SC_AtsCont_TRUE  = true   /**< \brief Continue on failure */
};

typedef uint8 SC_AtsCont_Enum_t;

#ifndef SC_OMIT_DEPRECATED
/**
 * \name Old-style SC Continue Flags
 * \{
 */
#define SC_CONTINUE_TRUE  SC_AtsCont_TRUE
#define SC_CONTINUE_FALSE SC_AtsCont_FALSE
/**\}*/
#endif

/************************************************************************
 * Type Definitions
 ************************************************************************/

/**
 * \defgroup cfssctlmpayload CFS Stored Command Telemetry Payload
 * \{
 */

/**
 *  \brief Housekeeping Packet Payload Structure
 */
typedef struct
{
    SC_AtsId_Enum_t   CurrAtsId;                /**< \brief Current ATS number: 1 = ATS A, 2 = ATS B */
    SC_Status_Enum_t  AtpState;                 /**< \brief Current ATP state: 2 = IDLE, 5 = EXECUTING */
    SC_AtsCont_Enum_t ContinueAtsOnFailureFlag; /**< \brief Continue ATS execution on failure flag */

    uint8 CmdErrCtr; /**< \brief Counts Request Errors */
    uint8 CmdCtr;    /**< \brief Counts Ground Requests */
    uint8 Padding8;  /**< \brief Structure padding */

    uint16           SwitchPendFlag;  /**< \brief Switch pending flag: 0 = NO, 1 = YES */
    uint16           NumRtsActive;    /**< \brief Number of RTSs currently active */
    SC_RtsNum_t      RtsNum;          /**< \brief Next RTS number */
    uint16           RtsActiveCtr;    /**< \brief Increments when an RTS is started without error */
    uint16           RtsActiveErrCtr; /**< \brief Increments when an attempt to start an RTS fails */
    uint16           AtsCmdCtr;       /**< \brief Total ATS cmd cnter counts commands sent by the ATS */
    uint16           AtsCmdErrCtr;    /**< \brief Total ATS cmd Error ctr command errors in the ATS */
    uint16           RtsCmdCtr;       /**< \brief Counts TOTAL rts cmds that were sent out from ALL active RTSs */
    uint16           RtsCmdErrCtr;    /**< \brief Counts TOTAL number of errs from ALL RTSs that are active */
    SC_AtsNum_t      LastAtsErrSeq;   /**< \brief Last ATS Errant Sequence Num Values: 1 or 2 */
    SC_CommandNum_t  LastAtsErrCmd;   /**< \brief Last ATS Errant Command Num */
    SC_RtsNum_t      LastRtsErrSeq;   /**< \brief Last RTS Errant Sequence Num */
    SC_EntryOffset_t LastRtsErrCmd;   /**< \brief Offset in the RTS buffer for the last command error, in "words" */

    SC_AtsNum_t AppendCmdArg;     /**< \brief ATS selection argument from most recent Append ATS command */
    uint16      AppendEntryCount; /**< \brief Number of cmd entries in current Append ATS table */
    uint16      AppendByteCount;  /**< \brief Size of cmd entries in current Append ATS table */
    uint16      AppendLoadCount;  /**< \brief Total number of Append ATS table loads */
    uint32      AtpCmdNumber;     /**< \brief Current command number */
    uint32      AtpFreeBytes[SC_NUMBER_OF_ATS]; /**< \brief Free Bytes in each ATS  */
    uint32      NextRtsWakeupCnt;               /**< \brief Next RTS Command Absolute Wakeup Count */
    uint32      NextAtsTime;                    /**< \brief Next ATS Command Time (seconds) */

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
} SC_HkTlm_Payload_t;

/**\}*/

/**
 * \defgroup cfssccmdpayload CFS Stored Command Command Payload Structures
 * \{
 */

/**
 *  \brief ATS Id Command Payload
 */
typedef struct
{
    SC_AtsNum_t AtsNum;  /**< \brief The ID of the ATS to start, 1 = ATS_A, 2 = ATS_B */
    uint16      Padding; /**< \brief Structure padding */
} SC_StartAtsCmd_Payload_t;

/**
 *  \brief RTS Id Command Payload
 */
typedef struct
{
    SC_RtsNum_t RtsNum;  /**< \brief The ID of the RTS to start, 1 through #SC_NUMBER_OF_RTS */
    uint16      Padding; /**< \brief Structure padding */
} SC_RtsCmd_Payload_t;

/**
 *  \brief Jump running ATS to a new time Command Payload
 */
typedef struct
{
    uint32 NewTime; /**< \brief the time to 'jump' to */
} SC_JumpAtsCmd_Payload_t;

/**
 *  \brief Continue ATS on failure command Payload
 */
typedef struct
{
    SC_AtsCont_Enum_t ContinueState; /**< \brief true or false, to continue ATS after a failure  */
    uint16            Padding;       /**< \brief Structure Padding */
} SC_SetContinueAtsOnFailureCmd_Payload_t;

/**
 *  \brief Append to ATS Command Payload
 */
typedef struct
{
    SC_AtsNum_t AtsNum;  /**< \brief The ID of the ATS to append to, 1 = ATS_A, 2 = ATS_B */
    uint16      Padding; /**< \brief Structure Padding */
} SC_AppendAtsCmd_Payload_t;

/**
 *  \brief RTS Group Command Payload
 */
typedef struct
{
    SC_RtsNum_t FirstRtsNum; /**< \brief ID of the first RTS to act on, 1 through #SC_NUMBER_OF_RTS */
    SC_RtsNum_t LastRtsNum;  /**< \brief ID of the last RTS to act on, 1 through #SC_NUMBER_OF_RTS */
} SC_RtsGrpCmd_Payload_t;

/**\}*/

#endif
