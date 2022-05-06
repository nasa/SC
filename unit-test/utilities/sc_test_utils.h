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
 *   Unit test utilities header
 */
#ifndef SC_TEST_UTILS_H
#define SC_TEST_UTILS_H

/*
 * Includes
 */

#include "sc_app.h"
#include "utstubs.h"
#include "cfe_msgids.h"
#include "cfe_tbl_msg.h"

extern SC_AppData_t      SC_AppData;
extern SC_OperData_t     SC_OperData;
extern SC_RtsInfoEntry_t RtsInfoTbl[SC_NUMBER_OF_RTS];

/*
 * Global context structures
 */
typedef struct
{
    uint16 EventID;
    uint16 EventType;
    char   Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
} CFE_EVS_SendEvent_context_t;

typedef struct
{
    char Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
} CFE_ES_WriteToSysLog_context_t;

extern CFE_EVS_SendEvent_context_t    context_CFE_EVS_SendEvent[];
extern CFE_ES_WriteToSysLog_context_t context_CFE_ES_WriteToSysLog;

/* Command buffer typedef for any handler */
typedef union
{
    CFE_SB_Buffer_t                 Buf;
    SC_NoArgsCmd_t                  NoArgsCmd;
    SC_StartAtsCmd_t                StartAtsCmd;
    SC_RtsCmd_t                     RtsCmd;
    SC_JumpAtsCmd_t                 JumpAtsCmd;
    SC_SetContinueAtsOnFailureCmd_t SetContinueAtsOnFailureCmd;
    SC_AppendAtsCmd_t               AppendAtsCmd;
#if (SC_ENABLE_GROUP_COMMANDS == true)
    SC_RtsGrpCmd_t RtsGrpCmd;
#endif
    CFE_TBL_NotifyCmd_t NotifyCmd; /* SC subscribes to the table notify command */
} UT_CmdBuf_t;

extern UT_CmdBuf_t UT_CmdBuf;

/* Unit test ids */
#define SC_UT_MID_1 CFE_SB_ValueToMsgId(CFE_PLATFORM_TLM_MID_BASE + 1)

/*
 * Function Definitions
 */

void SC_Test_Setup(void);
void SC_Test_TearDown(void);

#endif
