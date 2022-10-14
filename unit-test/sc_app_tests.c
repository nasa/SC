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

/*
 * Includes
 */

#include "sc_rts.h"
#include "sc_app.h"
#include "sc_cmds.h"
#include "sc_state.h"
#include "sc_atsrq.h"
#include "sc_rtsrq.h"
#include "sc_utils.h"
#include "sc_loads.h"
#include "sc_msgids.h"
#include "sc_events.h"
#include "sc_test_utils.h"
#include <unistd.h>
#include <stdlib.h>

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

/* sc_app_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

uint16 SC_APP_TEST_CFE_TBL_RegisterHookCount;

uint16 SC_APP_TEST_CFE_TBL_GetAddressHookCount;

uint32 SC_APP_TEST_DummyTableBuffer[10][SC_NUMBER_OF_RTS];

SC_AtpControlBlock_t SC_APP_TEST_GlobalAtsCtrlBlck;

uint32 SC_APP_TEST_GlobalAtsCmdStatusTbl[2048];

SC_RtpControlBlock_t SC_APP_TEST_GlobalRtsCtrlBlck;

SC_RtsInfoEntry_t SC_APP_TEST_GlobalRtsInfoTbl;

/*
 * Function Definitions
 */

int32 CFE_TBL_RegisterHook1(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    CFE_TBL_Handle_t *TblHandle = (CFE_TBL_Handle_t *)Context->ArgPtr[0];

    *TblHandle = (CFE_TBL_Handle_t)SC_APP_TEST_CFE_TBL_RegisterHookCount++;

    return CFE_SUCCESS;
}

int32 CFE_TBL_GetAddressHookNominal(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    void **TblPtr = (void **)Context->ArgPtr[0];

    *TblPtr = &SC_APP_TEST_DummyTableBuffer[SC_APP_TEST_CFE_TBL_GetAddressHookCount];

    SC_OperData.AtsCtrlBlckAddr        = &SC_APP_TEST_GlobalAtsCtrlBlck;
    SC_OperData.AtsCmdStatusTblAddr[0] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsCmdStatusTblAddr[1] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[1024];
    SC_OperData.RtsCtrlBlckAddr        = &SC_APP_TEST_GlobalRtsCtrlBlck;
    SC_OperData.RtsInfoTblAddr         = &SC_APP_TEST_GlobalRtsInfoTbl;

    if (++SC_APP_TEST_CFE_TBL_GetAddressHookCount > 6)
        return CFE_TBL_INFO_UPDATED;
    else
        return CFE_SUCCESS;
}

int32 CFE_TBL_GetAddressHookNominal2(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                     const UT_StubContext_t *Context)
{
    return CFE_TBL_ERR_NEVER_LOADED;
}

int32 CFE_TBL_GetAddressHookError1(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    void **TblPtr = (void **)Context->ArgPtr[0];

    *TblPtr = &SC_APP_TEST_DummyTableBuffer[SC_APP_TEST_CFE_TBL_GetAddressHookCount];

    SC_OperData.AtsCtrlBlckAddr        = &SC_APP_TEST_GlobalAtsCtrlBlck;
    SC_OperData.AtsCmdStatusTblAddr[0] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsCmdStatusTblAddr[1] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[1024];
    SC_OperData.RtsCtrlBlckAddr        = &SC_APP_TEST_GlobalRtsCtrlBlck;
    SC_OperData.RtsInfoTblAddr         = &SC_APP_TEST_GlobalRtsInfoTbl;

    if (++SC_APP_TEST_CFE_TBL_GetAddressHookCount > 6)
        return -1;
    else
        return CFE_SUCCESS;
}

void SC_AppMain_Test_Nominal(void)
{
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    size_t         MsgSize   = sizeof(SC_NoArgsCmd_t);

    /* Called in a subfunction.  Set here to prevent segmentation fault. */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Load return buffer to make loop execute twice */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Return timeout first time through, will default to success on second */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SB_TIME_OUT);

    /* Prevents error messages in call to SC_GetLoadTablePointers */
    SC_APP_TEST_CFE_TBL_GetAddressHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_GetAddressHookNominal, NULL);

    /* Execute the function being tested */
    SC_AppMain();

    /* Verify results */
    /* Generates 2 event messages we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void SC_AppMain_Test_AppInitError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    char  ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "App terminating, Result = 0x%%08X");
    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "SC App terminating, Result = 0x%%08X\n");

    /* Prevents error messages in call to SC_GetLoadTablePointers */
    SC_APP_TEST_CFE_TBL_GetAddressHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_GetAddressHookNominal, NULL);

    /* Set CFE_EVS_Register to return -1 in order to make SC_AppInit return -1, in order to reach "RunStatus =
     * CFE_ES_APP_ERROR" */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    /* Execute the function being tested */
    SC_AppMain();

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_APP_EXIT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);
}

void SC_AppMain_Test_RcvMsgError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    char  ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "App terminating, Result = 0x%%08X");

    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "SC App terminating, Result = 0x%%08X\n");

    /* Prevents error messages in call to SC_GetLoadTablePointers */
    SC_APP_TEST_CFE_TBL_GetAddressHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_GetAddressHookNominal, NULL);

    /* Set to make loop execute exactly once */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set CFE_SB_RcvMsg to return -1 in order to reach "RunStatus = CFE_ES_APP_ERROR" */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, -1);

    /* Execute the function being tested */
    SC_AppMain();

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, SC_APP_EXIT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[2].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[2].Spec);

    /* Generates 3 event messages we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 3, "CFE_EVS_SendEvent was called %u time(s), expected 3",
                  call_count_CFE_EVS_SendEvent);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);
}

void SC_AppInit_Test_NominalPowerOnReset(void)
{
    uint32         ReturnValue;
    SC_OperData_t  Expected_SC_OperData;
    SC_AppData_t   Expected_SC_AppData;
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_HK_TLM_MID);
    size_t         MsgSize   = sizeof(SC_HkTlm_t);
    int32          strCmpResult;
    char           ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "SC Initialized. Version %%d.%%d.%%d.%%d");

    /* Set CFE_ES_GetResetType to return CFE_ES_POWERON_RESET in order to reach "SC_AppData.AutoStartRTS =
     * RTS_ID_AUTO_POWER_ON" */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetResetType), 1, CFE_PSP_RST_TYPE_POWERON);

    /* Prevents error messages in call to SC_GetLoadTablePointers */
    SC_APP_TEST_CFE_TBL_GetAddressHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_GetAddressHookNominal, NULL);

    /* Sets table handles */
    SC_APP_TEST_CFE_TBL_RegisterHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_Register), CFE_TBL_RegisterHook1, NULL);

    /* Set global data structures to non-zero values, to verify that SC_AppInit sets values to 0 (with some exceptions)
     */
    memset(&SC_OperData, 1, sizeof(SC_OperData));
    memset(&SC_AppData, 1, sizeof(SC_AppData));

    memset(&Expected_SC_OperData, 0, sizeof(Expected_SC_OperData));
    memset(&Expected_SC_AppData, 0, sizeof(Expected_SC_AppData));

    Expected_SC_AppData.NextProcNumber      = SC_NONE;
    Expected_SC_AppData.NextCmdTime[SC_ATP] = SC_MAX_TIME;
    Expected_SC_AppData.NextCmdTime[SC_RTP] = SC_MAX_TIME;
    Expected_SC_AppData.AutoStartRTS        = RTS_ID_AUTO_POWER_ON;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    ReturnValue = SC_AppInit();

    /* Verify results */
    UtAssert_True(ReturnValue == CFE_SUCCESS, "ReturnValue == CFE_SUCCESS");

    Expected_SC_OperData.AtsInfoHandle         = 0;
    Expected_SC_OperData.RtsCtrlBlckHandle     = 0;
    Expected_SC_OperData.AtsCtrlBlckHandle     = 0;
    Expected_SC_OperData.AtsCmdStatusHandle[0] = 0;
    Expected_SC_OperData.AtsCmdStatusHandle[1] = 0;

    Expected_SC_OperData.HkPacket.ContinueAtsOnFailureFlag = 1;

    UtAssert_MemCmp(&SC_OperData.CmdPipe, &Expected_SC_OperData.CmdPipe, sizeof(Expected_SC_OperData.CmdPipe), "2");
    UtAssert_MemCmp(&SC_OperData.AtsInfoHandle, &Expected_SC_OperData.AtsInfoHandle,
                    sizeof(Expected_SC_OperData.AtsInfoHandle), "AtsInfoHandle");
    UtAssert_MemCmp(&SC_OperData.RtsInfoHandle, &Expected_SC_OperData.RtsInfoHandle,
                    sizeof(Expected_SC_OperData.RtsInfoHandle), "13");
    UtAssert_MemCmp(&SC_OperData.RtsCtrlBlckHandle, &Expected_SC_OperData.RtsCtrlBlckHandle,
                    sizeof(Expected_SC_OperData.RtsCtrlBlckHandle), "RtsCtrlBlckHandle");
    UtAssert_MemCmp(&SC_OperData.AtsCtrlBlckHandle, &Expected_SC_OperData.AtsCtrlBlckHandle,
                    sizeof(Expected_SC_OperData.AtsCtrlBlckHandle), "AtsCtrlBlckHandle");
    UtAssert_MemCmp(&SC_OperData.AtsCmdStatusHandle, &Expected_SC_OperData.AtsCmdStatusHandle,
                    sizeof(Expected_SC_OperData.AtsCmdStatusHandle), "AtsCmdStatusHandle");
    UtAssert_MemCmp(&SC_OperData.AtsDupTestArray, &Expected_SC_OperData.AtsDupTestArray,
                    sizeof(Expected_SC_OperData.AtsDupTestArray), "21");
    UtAssert_MemCmp(&SC_OperData.NumCmdsSec, &Expected_SC_OperData.NumCmdsSec, sizeof(Expected_SC_OperData.NumCmdsSec),
                    "22");
    UtAssert_MemCmp(&SC_OperData.HkPacket, &Expected_SC_OperData.HkPacket, sizeof(Expected_SC_OperData.HkPacket), "23");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_INIT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    /* Generates 1 event messages we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void SC_AppInit_Test_Nominal(void)
{
    uint32         ReturnValue;
    SC_OperData_t  Expected_SC_OperData;
    SC_AppData_t   Expected_SC_AppData;
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_HK_TLM_MID);
    size_t         MsgSize   = sizeof(SC_HkTlm_t);
    int32          strCmpResult;
    char           ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "SC Initialized. Version %%d.%%d.%%d.%%d");

    /* Set CFE_ES_GetResetType to return something other than CFE_ES_POWERON_RESET in order to reach
     * "SC_AppData.AutoStartRTS = RTS_ID_AUTO_PROCESSOR" */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetResetType), 1, -1);

    /* Prevents error messages in call to SC_GetLoadTablePointers */
    SC_APP_TEST_CFE_TBL_GetAddressHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_GetAddressHookNominal, NULL);

    /* Sets table handles */
    SC_APP_TEST_CFE_TBL_RegisterHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_Register), CFE_TBL_RegisterHook1, NULL);

    /* Set global data structures to non-zero values, to verify that SC_AppInit sets values to 0 (with some exceptions)
     */
    memset(&SC_OperData, 1, sizeof(SC_OperData));
    memset(&SC_AppData, 1, sizeof(SC_AppData));

    memset(&Expected_SC_OperData, 0, sizeof(Expected_SC_OperData));
    memset(&Expected_SC_AppData, 0, sizeof(Expected_SC_AppData));

    Expected_SC_AppData.NextProcNumber      = SC_NONE;
    Expected_SC_AppData.NextCmdTime[SC_ATP] = SC_MAX_TIME;
    Expected_SC_AppData.NextCmdTime[SC_RTP] = SC_MAX_TIME;
    Expected_SC_AppData.AutoStartRTS        = RTS_ID_AUTO_PROCESSOR;

    Expected_SC_OperData.HkPacket.ContinueAtsOnFailureFlag = 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    ReturnValue = SC_AppInit();

    /* Verify results */
    UtAssert_True(ReturnValue == CFE_SUCCESS, "ReturnValue == CFE_SUCCESS");

    Expected_SC_OperData.AtsInfoHandle         = 0;
    Expected_SC_OperData.RtsCtrlBlckHandle     = 0;
    Expected_SC_OperData.AtsCtrlBlckHandle     = 0;
    Expected_SC_OperData.AtsCmdStatusHandle[0] = 0;
    Expected_SC_OperData.AtsCmdStatusHandle[1] = 0;

    UtAssert_MemCmp(&SC_OperData.CmdPipe, &Expected_SC_OperData.CmdPipe, sizeof(Expected_SC_OperData.CmdPipe), "2");
    UtAssert_MemCmp(&SC_OperData.AtsInfoHandle, &Expected_SC_OperData.AtsInfoHandle,
                    sizeof(Expected_SC_OperData.AtsInfoHandle), "AtsInfoHandle");
    UtAssert_MemCmp(&SC_OperData.RtsInfoHandle, &Expected_SC_OperData.RtsInfoHandle,
                    sizeof(Expected_SC_OperData.RtsInfoHandle), "13");
    UtAssert_MemCmp(&SC_OperData.RtsCtrlBlckHandle, &Expected_SC_OperData.RtsCtrlBlckHandle,
                    sizeof(Expected_SC_OperData.RtsCtrlBlckHandle), "RtsCtrlBlckHandle");
    UtAssert_MemCmp(&SC_OperData.AtsCtrlBlckHandle, &Expected_SC_OperData.AtsCtrlBlckHandle,
                    sizeof(Expected_SC_OperData.AtsCtrlBlckHandle), "AtsCtrlBlckHandle");
    UtAssert_MemCmp(&SC_OperData.AtsCmdStatusHandle, &Expected_SC_OperData.AtsCmdStatusHandle,
                    sizeof(Expected_SC_OperData.AtsCmdStatusHandle), "AtsCmdStatusHandle");
    UtAssert_MemCmp(&SC_OperData.AtsDupTestArray, &Expected_SC_OperData.AtsDupTestArray,
                    sizeof(Expected_SC_OperData.AtsDupTestArray), "21");
    UtAssert_MemCmp(&SC_OperData.NumCmdsSec, &Expected_SC_OperData.NumCmdsSec, sizeof(Expected_SC_OperData.NumCmdsSec),
                    "22");
    UtAssert_MemCmp(&SC_OperData.HkPacket, &Expected_SC_OperData.HkPacket, sizeof(Expected_SC_OperData.HkPacket), "23");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_INIT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    /* Generates 1 event messages we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void SC_AppInit_Test_EVSRegisterError(void)
{
    uint32 ReturnValue;
    int32  strCmpResult;
    char   ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Event Services Register returned: 0x%%08X\n");

    /* Set CFE_EVS_Register to return -1 in order to reach "CFE_ES_WriteToSysLog("Event Services Register returned:
     * 0x%08X\n", Result)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    /* Execute the function being tested */
    ReturnValue = SC_AppInit();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);
}

void SC_AppInit_Test_SBCreatePipeError(void)
{
    uint32 ReturnValue;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Software Bus Create Pipe returned: 0x%%08X");

    /* Set CFE_SB_CreatePipe to return -1 in order to generate error message SC_INIT_SB_CREATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 1, -1);

    /* Execute the function being tested */
    ReturnValue = SC_AppInit();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_INIT_SB_CREATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_AppInit_Test_SBSubscribeHKError(void)
{
    uint32 ReturnValue;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Software Bus subscribe to housekeeping returned: 0x%%08X");

    /* Set CFE_SB_Subscribe to return -1 on the first call in order to generate error message
     * SC_INIT_SB_SUBSCRIBE_HK_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 1, -1);

    /* Execute the function being tested */
    ReturnValue = SC_AppInit();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_INIT_SB_SUBSCRIBE_HK_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_AppInit_Test_SubscribeTo1HzError(void)
{
    uint32 ReturnValue;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Software Bus subscribe to 1 Hz cycle returned: 0x%%08X");

    /* Set CFE_SB_Subscribe to return -1 on the 2nd call in order to generate error message
     * SC_INIT_SB_SUBSCRIBE_1HZ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 2, -1);

    /* Execute the function being tested */
    ReturnValue = SC_AppInit();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_INIT_SB_SUBSCRIBE_1HZ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_AppInit_Test_SBSubscribeToCmdError(void)
{
    uint32 ReturnValue;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Software Bus subscribe to command returned: 0x%%08X");

    /* Set CFE_SB_Subscribe to return -1 on the 3rd call in order to generate error message
     * SC_INIT_SB_SUBSCRIBE_CMD_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 3, -1);

    /* Execute the function being tested */
    ReturnValue = SC_AppInit();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_INIT_SB_SUBSCRIBE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_AppInit_Test_InitTablesError(void)
{
    uint32 ReturnValue;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s table register failed, returned: 0x%%08X");

    /* Set CFE_TBL_Register to return -1 in order to reach return statement immediately after
     * comment "Must be able to create and initialize tables" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 1, -1);

    /* Execute the function being tested */
    ReturnValue = SC_AppInit();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_RTS_INFO_TABLE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_InitTables_Test_Nominal(void)
{
    /* Prevents error messages in call to SC_GetLoadTablePointers */
    SC_APP_TEST_CFE_TBL_GetAddressHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_GetAddressHookNominal, NULL);

    /* Sets table handles */
    SC_APP_TEST_CFE_TBL_RegisterHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_Register), CFE_TBL_RegisterHook1, NULL);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_AppInit(), CFE_SUCCESS);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LOAD_FAIL_COUNT_INFO_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_INIT_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_InitTables_Test_ErrorRegisterAllTables(void)
{
    uint32 ReturnValue;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s table register failed, returned: 0x%%08X");

    /* Set CFE_TBL_Register to return -1 in order to reach return statement immediately after
     * comment "Must be able to create and initialize tables" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 1, -1);

    /* Execute the function being tested */
    ReturnValue = SC_InitTables();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_RTS_INFO_TABLE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_InitTables_Test_ErrorGetDumpTablePointers(void)
{
    uint32 ReturnValue;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Table failed Getting Address, returned: 0x%%08X");

    /* Set CFE_TBL_GetAddress to return -1 in order to cause SC_GetDumpTablePointers to return -1, in order to reach
     * "return(Result)" after SC_GetDumpTablePointers */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    ReturnValue = SC_InitTables();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_RTS_INFO_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_InitTables_Test_ErrorGetLoadTablePointers(void)
{
    uint32 ReturnValue;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ATS table %%d failed Getting Address, returned: 0x%%08X");

    /* Prevents error messages in call to SC_GetLoadTablePointers */
    SC_APP_TEST_CFE_TBL_GetAddressHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_GetAddressHookError1, NULL);

    /* Sets table handles */
    SC_APP_TEST_CFE_TBL_RegisterHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_Register), CFE_TBL_RegisterHook1, NULL);

    /* Causes SC_GetLoadTablePointers to return -1 in order to cause SC_GetLoadTablePointers to return -1, in order to
     * reach "Return(Result)" after SC_GetLoadTablePointers */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 7, -1);

    /* Execute the function being tested */
    ReturnValue = SC_InitTables();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_GET_ADDRESS_ATS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    /* Generates 1 event message we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void SC_RegisterAllTables_Test_Nominal(void)
{
    int32 ReturnValue;

    /* Execute the function being tested */
    ReturnValue = SC_RegisterAllTables();

    /* Verify results */
    UtAssert_True(ReturnValue == CFE_SUCCESS, "ReturnValue == CFE_SUCCESS");
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_RegisterAllTables_Test_ErrorRegisterRTSInformation(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s table register failed, returned: 0x%%08X");

    /* Set CFE_TBL_Register to return -1 on the 1st call in order to generate error message
     * SC_REGISTER_RTS_INFO_TABLE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 1, -1);

    /* Execute the function being tested */
    ReturnValue = SC_RegisterAllTables();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_RTS_INFO_TABLE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_RegisterAllTables_Test_ErrorRegisterRTPControl(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s table register failed, returned: 0x%%08X");

    /* Set CFE_TBL_Register to return -1 on the 2nd call in order to generate error message
     * SC_REGISTER_RTS_CTRL_BLK_TABLE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 2, -1);

    /* Execute the function being tested */
    ReturnValue = SC_RegisterAllTables();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_RTS_CTRL_BLK_TABLE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_RegisterAllTables_Test_ErrorRegisterATSInformation(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s table register failed, returned: 0x%%08X");

    /* Set CFE_TBL_Register to return -1 on the 3rd call in order to generate error message
     * SC_REGISTER_ATS_INFO_TABLE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 3, -1);

    /* Execute the function being tested */
    ReturnValue = SC_RegisterAllTables();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_ATS_INFO_TABLE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_RegisterAllTables_Test_ErrorRegisterATPControl(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s table register failed, returned: 0x%%08X");

    /* Set CFE_TBL_Register to return -1 on the 4th call in order to generate error message
     * SC_REGISTER_ATS_CTRL_BLK_TABLE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 4, -1);

    /* Execute the function being tested */
    ReturnValue = SC_RegisterAllTables();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_ATS_CTRL_BLK_TABLE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_RegisterAllTables_Test_ErrorRegisterATSCommandStatus(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ATS command status table register failed for ATS %%d, returned: 0x%%08X");

    /* Set CFE_TBL_Register to return -1 on the 5th call in order to generate error message
     * SC_REGISTER_ATS_CMD_STATUS_TABLE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 5, -1);

    /* Execute the function being tested */
    ReturnValue = SC_RegisterAllTables();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_ATS_CMD_STATUS_TABLE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_RegisterAllTables_Test_ErrorRegisterLoadableRTS(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Table Registration Failed for %%s %%d, returned: 0x%%08X");

    /* Set CFE_TBL_Register to return -1 on the last call when registering loadable RTS tables in order to generate
     * error message SC_REGISTER_RTS_TBL_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 5 + SC_NUMBER_OF_ATS, -1);

    /* Execute the function being tested */
    ReturnValue = SC_RegisterAllTables();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_RTS_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_RegisterAllTables_Test_ErrorRegisterLoadableATS(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Table Registration Failed for %%s %%d, returned: 0x%%08X");

    /* Set CFE_TBL_Register to return -1 on the last call when registering loadable ATS tables in order to generate
     * error message SC_REGISTER_ATS_TBL_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 6 + SC_NUMBER_OF_ATS + SC_NUMBER_OF_RTS, -1);

    /* Execute the function being tested */
    ReturnValue = SC_RegisterAllTables();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_ATS_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_RegisterAllTables_Test_ErrorRegisterLoadableAppendATS(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Append ATS Table Registration Failed, returned: 0x%%08X");

    /* Set CFE_TBL_Register to return -1 on the last call when registering loadable Append ATS tables in order to
     * generate error message SC_REGISTER_APPEND_TBL_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 5 + 2 * SC_NUMBER_OF_ATS + SC_NUMBER_OF_RTS, -1);

    /* Execute the function being tested */
    ReturnValue = SC_RegisterAllTables();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_APPEND_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_RegisterDumpOnlyTables_Test_Nominal(void)
{
    int32 ReturnValue;

    /* Execute the function being tested */
    ReturnValue = SC_RegisterDumpOnlyTables();

    /* Verify results */
    UtAssert_True(ReturnValue == CFE_SUCCESS, "ReturnValue == CFE_SUCCESS");
    UtAssert_STUB_COUNT(CFE_TBL_Register, 6);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_RegisterLoadableTables_Test_Nominal(void)
{
    int32 ReturnValue;

    /* Execute the function being tested */
    ReturnValue = SC_RegisterLoadableTables();

    /* Verify results */
    UtAssert_True(ReturnValue == CFE_SUCCESS, "ReturnValue == CFE_SUCCESS");
    UtAssert_STUB_COUNT(CFE_TBL_Register, 67);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetDumpTablePointers_Test_Nominal(void)
{
    int32 ReturnValue;

    /* Same return value as default, but bypasses default hook function to simplify test */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_SUCCESS);

    /* Execute the function being tested */
    ReturnValue = SC_GetDumpTablePointers();

    /* Verify results */
    UtAssert_True(ReturnValue == CFE_SUCCESS, "ReturnValue == CFE_SUCCESS");
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_GetDumpTablePointers_Test_ErrorGetAddressRTSInformation(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Table failed Getting Address, returned: 0x%%08X");

    /* Need to set CFE_TBL_GetAddress to return -1 on 1st call (to generate error message
     * SC_GET_ADDRESS_RTS_INFO_ERR_EID) and CFE_SUCCESS on all other calls.  This could have been done using just a hook
     * function and a global variable, but it was simpler to set a hook function that just returns CFE_SUCCESS and then
     * also set a return code for the particular call number.  Because of the order of handling of return values and
     * hook functions in the stub function, this results in the return code being used if it's the right call number,
     * while otherwise using the hook function. */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    ReturnValue = SC_GetDumpTablePointers();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_RTS_INFO_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_GetDumpTablePointers_Test_ErrorGetAddressRTPControl(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Table failed Getting Address, returned: 0x%%08X");

    /* Need to set CFE_TBL_GetAddress to return -1 on 2nd call (to generate error message
     * SC_GET_ADDRESS_RTS_CTRL_BLCK_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 2, -1);

    /* Execute the function being tested */
    ReturnValue = SC_GetDumpTablePointers();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_RTS_CTRL_BLCK_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_GetDumpTablePointers_Test_ErrorGetAddressATSInformation(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Table failed Getting Address, returned: 0x%%08X");

    /* Need to set CFE_TBL_GetAddress to return -1 on 3rd call (to generate error message
     * SC_GET_ADDRESS_ATS_INFO_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 3, -1);

    /* Execute the function being tested */
    ReturnValue = SC_GetDumpTablePointers();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_ATS_INFO_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_GetDumpTablePointers_Test_ErrorGetAddressATPControl(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Table failed Getting Address, returned: 0x%%08X");

    /* Need to set CFE_TBL_GetAddress to return -1 on 5th call (to generate error message
     * SC_GET_ADDRESS_ATS_CTRL_BLCK_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 4, -1);

    /* Execute the function being tested */
    ReturnValue = SC_GetDumpTablePointers();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_ATS_CTRL_BLCK_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_GetDumpTablePointers_Test_ErrorGetAddressATSCommandStatus(void)
{
    int32 ReturnValue;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ATS Cmd Status table for ATS %%d failed Getting Address, returned: 0x%%08X");

    /* Need to set CFE_TBL_GetAddress to return -1 on 6th call (to generate error message
     * SC_GET_ADDRESS_ATS_CMD_STAT_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 5, -1);

    /* Execute the function being tested */
    ReturnValue = SC_GetDumpTablePointers();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_ATS_CMD_STAT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_GetLoadTablePointers_Test_Nominal(void)
{
    int32 ReturnValue;

    /* Prevents all error messages */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_ERR_NEVER_LOADED);

    /* Execute the function being tested */
    ReturnValue = SC_GetLoadTablePointers();

    /* Verify results */
    UtAssert_True(ReturnValue == CFE_SUCCESS, "ReturnValue == CFE_SUCCESS");
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableATS(void)
{
    uint32 ReturnValue;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ATS table %%d failed Getting Address, returned: 0x%%08X");

    /* Need to set CFE_TBL_GetAddress to return -1 on 1st call (to generate error message SC_GET_ADDRESS_ATS_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    ReturnValue = SC_GetLoadTablePointers();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_ATS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableATSAppend(void)
{
    uint32 ReturnValue;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Append ATS table failed Getting Address, returned: 0x%%08X");

    /* Need to set CFE_TBL_GetAddress to return -1 on call (SC_NUMBER_OF_ATS + 1) (to generate error message
     * SC_GET_ADDRESS_APPEND_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    ReturnValue = SC_GetLoadTablePointers();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_APPEND_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableRTS(void)
{
    uint32 ReturnValue;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "RTS table %%d failed Getting Address, returned: 0x%%08X");

    /* Need to set CFE_TBL_GetAddress to return -1 on call (SC_NUMBER_OF_ATS + 2) (to generate error message
     * SC_GET_ADDRESS_RTS_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    ReturnValue = SC_GetLoadTablePointers();

    /* Verify results */
    UtAssert_True(ReturnValue == -1, "ReturnValue == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_RTS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_LoadDefaultTables_Test(void)
{
    /* Set OS_open to return 1, in order to enter if-block "if (FileDesc >= 0)" */
    UT_SetDeferredRetcode(UT_KEY(OS_OpenCreate), 1, OS_SUCCESS);

    /* Cover branch for - Only try to load table files that can be opened */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Load), 1, -1);

    /* Cover branch for - send an event for each failed open */
    UT_SetDeferredRetcode(UT_KEY(OS_OpenCreate), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_LoadDefaultTables());

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LOAD_FAIL_DBG_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_RTS_OPEN_FAIL_DBG_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, SC_RTS_LOAD_FAIL_COUNT_INFO_EID);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 3);
}

void UtTest_Setup(void)
{
    UtTest_Add(SC_AppMain_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_AppMain_Test_Nominal");
    UtTest_Add(SC_AppMain_Test_AppInitError, SC_Test_Setup, SC_Test_TearDown, "SC_AppMain_Test_AppInitError");
    UtTest_Add(SC_AppMain_Test_RcvMsgError, SC_Test_Setup, SC_Test_TearDown, "SC_AppMain_Test_RcvMsgError");
    UtTest_Add(SC_AppInit_Test_NominalPowerOnReset, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppInit_Test_NominalPowerOnReset");
    UtTest_Add(SC_AppInit_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_AppInit_Test_Nominal");
    UtTest_Add(SC_AppInit_Test_EVSRegisterError, SC_Test_Setup, SC_Test_TearDown, "SC_AppInit_Test_EVSRegisterError");
    UtTest_Add(SC_AppInit_Test_SBCreatePipeError, SC_Test_Setup, SC_Test_TearDown, "SC_AppInit_Test_SBCreatePipeError");
    UtTest_Add(SC_AppInit_Test_SBSubscribeHKError, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppInit_Test_SBSubscribeHKError");
    UtTest_Add(SC_AppInit_Test_SBSubscribeToCmdError, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppInit_Test_SBSubscribeToCmdError");
    UtTest_Add(SC_AppInit_Test_SubscribeTo1HzError, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppInit_Test_SubscribeTo1HzError");
    UtTest_Add(SC_AppInit_Test_InitTablesError, SC_Test_Setup, SC_Test_TearDown, "SC_AppInit_Test_InitTablesError");
    UtTest_Add(SC_InitTables_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_InitTables_Test_Nominal");
    UtTest_Add(SC_InitTables_Test_ErrorRegisterAllTables, SC_Test_Setup, SC_Test_TearDown,
               "SC_InitTables_Test_ErrorRegisterAllTables");
    UtTest_Add(SC_InitTables_Test_ErrorGetDumpTablePointers, SC_Test_Setup, SC_Test_TearDown,
               "SC_InitTables_Test_ErrorGetDumpTablePointers");
    UtTest_Add(SC_InitTables_Test_ErrorGetLoadTablePointers, SC_Test_Setup, SC_Test_TearDown,
               "SC_InitTables_Test_ErrorGetLoadTablePointers");
    UtTest_Add(SC_RegisterAllTables_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_RegisterAllTables_Test_Nominal");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterRTSInformation, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterRTSInformation");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterRTPControl, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterRTPControl");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterATSInformation, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterATSInformation");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterATPControl, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterATPControl");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterATSCommandStatus, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterATSCommandStatus");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterLoadableRTS, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterLoadableRTS");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterLoadableATS, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterLoadableATS");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterLoadableAppendATS, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterLoadableAppendATS");
    UtTest_Add(SC_RegisterDumpOnlyTables_Test_Nominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterDumpOnlyTables_Test_Nominal");
    UtTest_Add(SC_RegisterLoadableTables_Test_Nominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterLoadableTables_Test_Nominal");
    UtTest_Add(SC_GetDumpTablePointers_Test_Nominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetDumpTablePointers_Test_Nominal");
    UtTest_Add(SC_GetDumpTablePointers_Test_ErrorGetAddressRTSInformation, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetDumpTablePointers_Test_ErrorGetAddressRTSInformation");
    UtTest_Add(SC_GetDumpTablePointers_Test_ErrorGetAddressRTPControl, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetDumpTablePointers_Test_ErrorGetAddressRTPControl");
    UtTest_Add(SC_GetDumpTablePointers_Test_ErrorGetAddressATSInformation, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetDumpTablePointers_Test_ErrorGetAddressATSInformation");
    UtTest_Add(SC_GetDumpTablePointers_Test_ErrorGetAddressATPControl, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetDumpTablePointers_Test_ErrorGetAddressATPControl");
    UtTest_Add(SC_GetDumpTablePointers_Test_ErrorGetAddressATSCommandStatus, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetDumpTablePointers_Test_ErrorGetAddressATSCommandStatus");
    UtTest_Add(SC_GetLoadTablePointers_Test_Nominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetLoadTablePointers_Test_Nominal");
    UtTest_Add(SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableATS, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableATS");
    UtTest_Add(SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableATSAppend, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableATSAppend");
    UtTest_Add(SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableRTS, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableRTS");
    UtTest_Add(SC_LoadDefaultTables_Test, SC_Test_Setup, SC_Test_TearDown, "SC_LoadDefaultTables_Test");
}
