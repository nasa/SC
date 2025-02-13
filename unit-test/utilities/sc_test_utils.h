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
#include "sc_msg.h"
#include "utstubs.h"
#include "cfe_msgids.h"
#include "cfe_tbl_msg.h"

extern SC_AppData_t  SC_AppData;
extern SC_OperData_t SC_OperData;

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
    CFE_SB_Buffer_t              Buf;
    SC_SendHkCmd_t               SendHkCmd;
    SC_WakeupCmd_t               WakeupCmd;
    SC_NoopCmd_t                 NoopCmd;
    SC_ResetCountersCmd_t        ResetCountersCmd;
    SC_StopAtsCmd_t              StopAtsCmd;
    SC_SwitchAtsCmd_t            SwitchAtsCmd;
    SC_StartAtsCmd_t             StartAtsCmd;
    SC_StartRtsCmd_t             StartRtsCmd;
    SC_StopRtsCmd_t              StopRtsCmd;
    SC_DisableRtsCmd_t           DisableRtsCmd;
    SC_EnableRtsCmd_t            EnableRtsCmd;
    SC_JumpAtsCmd_t              JumpAtsCmd;
    SC_ContinueAtsOnFailureCmd_t ContinueAtsOnFailureCmd;
    SC_AppendAtsCmd_t            AppendAtsCmd;
    SC_ManageTableCmd_t          ManageTableCmd;
    SC_StartRtsGrpCmd_t          StartRtsGrpCmd;
    SC_StopRtsGrpCmd_t           StopRtsGrpCmd;
    SC_DisableRtsGrpCmd_t        DisableRtsGrpCmd;
    SC_EnableRtsGrpCmd_t         EnableRtsGrpCmd;
} UT_CmdBuf_t;

extern UT_CmdBuf_t UT_CmdBuf;

/* Unit test ids */
#define SC_UT_MID_1 CFE_SB_ValueToMsgId(1)

/* SC-specific assert macros */
#define SC_Assert_ID_EQ(actual, ref)                                                                      \
    UtAssert_GenericUnsignedCompare(SC_IDNUM_AS_UINT(actual), UtAssert_Compare_EQ, SC_IDNUM_AS_UINT(ref), \
                                    UtAssert_Radix_DECIMAL, __FILE__, __LINE__, "ID", #actual, #ref)

#define SC_Assert_ID_VALUE(actual, ref)                                                                 \
    UtAssert_GenericUnsignedCompare(SC_IDNUM_AS_UINT(actual), UtAssert_Compare_EQ, (unsigned int)(ref), \
                                    UtAssert_Radix_DECIMAL, __FILE__, __LINE__, "IDV", #actual, #ref)

#define SC_Assert_IDX_EQ(actual, ref)                                                                 \
    UtAssert_GenericUnsignedCompare(SC_IDX_AS_UINT(actual), UtAssert_Compare_EQ, SC_IDX_AS_UINT(ref), \
                                    UtAssert_Radix_DECIMAL, __FILE__, __LINE__, "IDX", #actual, #ref)

#define SC_Assert_IDX_VALUE(actual, ref)                                                              \
    UtAssert_GenericUnsignedCompare(SC_IDX_AS_UINT(actual), UtAssert_Compare_EQ, (unsigned int)(ref), \
                                    UtAssert_Radix_DECIMAL, __FILE__, __LINE__, "IDXV", #actual, #ref)

#define SC_Assert_CmdStatus(actual, ref)                                                                \
    UtAssert_GenericUnsignedCompare((unsigned long)(actual), UtAssert_Compare_EQ, (unsigned long)(ref), \
                                    UtAssert_Radix_DECIMAL, __FILE__, __LINE__, "Status", #actual, #ref)

/*
 * Function Definitions
 */

void SC_Test_SetTableAddrs(void);
void SC_Test_Setup(void);
void SC_Test_TearDown(void);

#endif
