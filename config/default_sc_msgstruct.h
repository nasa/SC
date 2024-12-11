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
#ifndef SC_MSGSTRUCT_H
#define SC_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/
#include "sc_msgdefs.h"
#include "sc_mission_cfg.h"
#include "cfe_msg_hdr.h"

/* NOTE: The following should be just cfe_tbl_msgdefs.h,
 * but the CFE headers themselves are not fully correct */
#include "cfe_es_extern_typedefs.h"
#include "cfe_time_extern_typedefs.h"
#include "cfe_tbl_msg.h"

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
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SC_HkTlm_Payload_t        Payload;
} SC_HkTlm_t;

/**\}*/

/**
 * \defgroup cfssccmdstructs CFS Stored Command Command Structures
 * \{
 */

/**
 *  \brief ATS Id Command
 *
 *  For command details see #SC_START_ATS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t  CommandHeader; /**< \brief Command Header */
    SC_StartAtsCmd_Payload_t Payload;
} SC_StartAtsCmd_t;

/**
 *  \brief Jump running ATS to a new time Command
 *
 *  For command details see #SC_JUMP_ATS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
    SC_JumpAtsCmd_Payload_t Payload;
} SC_JumpAtsCmd_t;

/**
 *  \brief Continue ATS on failure command
 *
 *  For command details see #SC_CONTINUE_ATS_ON_FAILURE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t                 CommandHeader; /**< \brief Command Header */
    SC_SetContinueAtsOnFailureCmd_Payload_t Payload;
} SC_SetContinueAtsOnFailureCmd_t;

/**
 *  \brief Append to ATS Command
 *
 *  For command details see #SC_APPEND_ATS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t   CommandHeader; /**< \brief Command Header */
    SC_AppendAtsCmd_Payload_t Payload;
} SC_AppendAtsCmd_t;

/**
 *  \brief Send HK Command
 *
 *  For command details see #SC_SEND_HK_MID
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
} SC_SendHkCmd_t;

/**
 *  \brief Wakeup Command
 *
 *  For command details see #SC_WAKEUP_MID
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
} SC_WakeupCmd_t;

/**
 *  \brief No operation Command
 *
 *  For command details see #SC_NOOP_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
} SC_NoopCmd_t;

/**
 *  \brief Reset Counters Command
 *
 *  For command details see #SC_RESET_COUNTERS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
} SC_ResetCountersCmd_t;

/**
 *  \brief Stop ATS Command
 *
 *  For command details see #SC_STOP_ATS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
} SC_StopAtsCmd_t;

/**
 *  \brief Switch ATS Command
 *
 *  For command details see #SC_SWITCH_ATS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
} SC_SwitchAtsCmd_t;

/**
 *  \brief Start RTS Command
 *
 *  For command details see #SC_START_RTS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
    SC_RtsCmd_Payload_t     Payload;
} SC_StartRtsCmd_t;

/**
 *  \brief Stop RTS Command
 *
 *  For command details see #SC_STOP_RTS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
    SC_RtsCmd_Payload_t     Payload;
} SC_StopRtsCmd_t;

/**
 *  \brief Disable RTS Command
 *
 *  For command details see #SC_DISABLE_RTS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
    SC_RtsCmd_Payload_t     Payload;
} SC_DisableRtsCmd_t;

/**
 *  \brief Enable RTS Command
 *
 *  For command details see #SC_ENABLE_RTS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
    SC_RtsCmd_Payload_t     Payload;
} SC_EnableRtsCmd_t;

/**
 *  \brief Continue ATS on failure command
 *
 *  For command details see #SC_CONTINUE_ATS_ON_FAILURE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t                 CommandHeader; /**< \brief Command Header */
    SC_SetContinueAtsOnFailureCmd_Payload_t Payload;
} SC_ContinueAtsOnFailureCmd_t;

/**
 *  \brief Manage Table Command
 *
 *  For command details see #SC_MANAGE_TABLE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t     CommandHeader; /**< \brief Command Header */
    CFE_TBL_NotifyCmd_Payload_t Payload;
} SC_ManageTableCmd_t;

/**
 *  \brief RTS Group Command
 *
 *  For command details see #SC_START_RTS_GRP_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
    SC_RtsGrpCmd_Payload_t  Payload;
} SC_StartRtsGrpCmd_t;

/**
 *  \brief RTS Group Command
 *
 *  For command details see #SC_STOP_RTS_GRP_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
    SC_RtsGrpCmd_Payload_t  Payload;
} SC_StopRtsGrpCmd_t;

/**
 *  \brief RTS Group Command
 *
 *  For command details see #SC_DISABLE_RTS_GRP_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
    SC_RtsGrpCmd_Payload_t  Payload;
} SC_DisableRtsGrpCmd_t;

/**
 *  \brief RTS Group Command
 *
 *  For command details see #SC_ENABLE_RTS_GRP_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
    SC_RtsGrpCmd_Payload_t  Payload;
} SC_EnableRtsGrpCmd_t;

/**\}*/

#endif
