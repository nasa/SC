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
 *   Unit test utility implementations
 */

/*
 * Includes
 */

#include "sc_app.h"
#include "sc_test_utils.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#define UT_MAX_SENDEVENT_DEPTH 4
CFE_EVS_SendEvent_context_t    context_CFE_EVS_SendEvent[UT_MAX_SENDEVENT_DEPTH];
CFE_ES_WriteToSysLog_context_t context_CFE_ES_WriteToSysLog;

UT_CmdBuf_t UT_CmdBuf;

/* Table buffers */
uint32                 AtsTblAddr[SC_NUMBER_OF_ATS][SC_ATS_BUFF_SIZE32];
uint32                 AppendTblAddr[SC_APPEND_BUFF_SIZE32];
uint32                 RtsTblAddr[SC_NUMBER_OF_RTS][SC_RTS_BUFF_SIZE32];
SC_AtsInfoTable_t      AtsInfoTblAddr[SC_NUMBER_OF_ATS];
SC_RtsInfoEntry_t      RtsInfoTblAddr[SC_NUMBER_OF_RTS];
SC_RtpControlBlock_t   RtsCtrlBlckAddr;
SC_AtpControlBlock_t   AtsCtrlBlckAddr;
SC_AtsCmdStatusEntry_t AtsCmdStatusTblAddr[SC_NUMBER_OF_ATS][SC_MAX_ATS_CMDS];

/*
 * Function Definitions
 */
void UT_Handler_CFE_EVS_SendEvent(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va)
{
    uint16 CallCount;
    uint16 idx;

    CallCount = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    if (CallCount > (sizeof(context_CFE_EVS_SendEvent) / sizeof(context_CFE_EVS_SendEvent[0])))
    {
        UtAssert_Failed("CFE_EVS_SendEvent UT depth %u exceeded: %u, increase UT_MAX_SENDEVENT_DEPTH",
                        UT_MAX_SENDEVENT_DEPTH, CallCount);
    }
    else
    {
        idx                                      = CallCount - 1;
        context_CFE_EVS_SendEvent[idx].EventID   = UT_Hook_GetArgValueByName(Context, "EventID", uint16);
        context_CFE_EVS_SendEvent[idx].EventType = UT_Hook_GetArgValueByName(Context, "EventType", uint16);

        strncpy(context_CFE_EVS_SendEvent[idx].Spec, UT_Hook_GetArgValueByName(Context, "Spec", const char *),
                CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);
        context_CFE_EVS_SendEvent[idx].Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - 1] = '\0';
    }
}

void UT_Handler_CFE_ES_WriteToSysLog(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va)
{
    strncpy(context_CFE_ES_WriteToSysLog.Spec, UT_Hook_GetArgValueByName(Context, "SpecStringPtr", const char *),
            CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - 1);
    context_CFE_ES_WriteToSysLog.Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - 1] = '\0';
}

void SC_Test_SetTableAddrs(void)
{
    uint8 i;

    /* Set table addresses */
    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        SC_OperData.AtsTblAddr[i]          = AtsTblAddr[i];
        SC_OperData.AtsCmdStatusTblAddr[i] = AtsCmdStatusTblAddr[i];
    }
    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        SC_OperData.RtsTblAddr[i] = RtsTblAddr[i];
    }
    SC_OperData.AppendTblAddr   = AppendTblAddr;
    SC_OperData.AtsInfoTblAddr  = AtsInfoTblAddr;
    SC_OperData.RtsInfoTblAddr  = RtsInfoTblAddr;
    SC_OperData.RtsCtrlBlckAddr = &RtsCtrlBlckAddr;
    SC_OperData.AtsCtrlBlckAddr = &AtsCtrlBlckAddr;
}

void SC_Test_Setup(void)
{

    /* initialize test environment to default state for every test */
    UT_ResetState(0);

    /* Clear app data */
    memset(&SC_OperData, 0, sizeof(SC_OperData));
    memset(&SC_AppData, 0, sizeof(SC_AppData));

    /* Clear table buffers */
    memset(&AtsTblAddr, 0, sizeof(AtsTblAddr));
    memset(&AppendTblAddr, 0, sizeof(AppendTblAddr));
    memset(&RtsTblAddr, 0, sizeof(RtsTblAddr));
    memset(&AtsInfoTblAddr, 0, sizeof(AtsInfoTblAddr));
    memset(&RtsInfoTblAddr, 0, sizeof(RtsInfoTblAddr));
    memset(&RtsCtrlBlckAddr, 0, sizeof(RtsCtrlBlckAddr));
    memset(&AtsCtrlBlckAddr, 0, sizeof(AtsCtrlBlckAddr));
    memset(&AtsCmdStatusTblAddr, 0, sizeof(AtsCmdStatusTblAddr));

    /* Clear unit test buffers */
    memset(context_CFE_EVS_SendEvent, 0, sizeof(context_CFE_EVS_SendEvent));
    memset(&context_CFE_ES_WriteToSysLog, 0, sizeof(context_CFE_ES_WriteToSysLog));
    memset(&UT_CmdBuf, 0, sizeof(UT_CmdBuf));

    /* Set table addresses */
    SC_Test_SetTableAddrs();

    /* Register custom handlers */
    UT_SetVaHandlerFunction(UT_KEY(CFE_EVS_SendEvent), UT_Handler_CFE_EVS_SendEvent, NULL);
    UT_SetVaHandlerFunction(UT_KEY(CFE_ES_WriteToSysLog), UT_Handler_CFE_ES_WriteToSysLog, NULL);
}

void SC_Test_TearDown(void)
{
    /* cleanup test environment */
}
