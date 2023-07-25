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

#include "cfe.h"
#include "sc_utils.h"
#include "sc_events.h"
#include "sc_msgids.h"
#include "sc_test_utils.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

/* sc_utils_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

void SC_GetCurrentTime_Test(void)
{
    SC_AppData.CurrentTime = 0;

    /* Execute the function being tested */
    SC_GetCurrentTime();

    /* Verify results */
    UtAssert_True(SC_AppData.CurrentTime != 0, "SC_AppData.CurrentTime != 0");
}

void SC_GetAtsEntryTime_Test(void)
{
    SC_AbsTimeTag_t AbsTimeTag;

    SC_AtsEntryHeader_t Entry;
    Entry.TimeTag_MS = 0;
    Entry.TimeTag_LS = 10;

    /* Execute the function being tested */
    AbsTimeTag = SC_GetAtsEntryTime(&Entry);

    /* Verify results */
    UtAssert_True(AbsTimeTag == 10, "AbsTimeTag == 10");
}

void SC_ComputeAbsTime_Test(void)
{
    SC_AbsTimeTag_t AbsTimeTag;

    SC_AppData.CurrentTime = 0;

    /* Execute the function being tested */
    AbsTimeTag = SC_ComputeAbsTime(0);

    /* Verify results */

    /* The CFE_TIME_Add stub increments when status >= 0 */
    UtAssert_True(AbsTimeTag == 1, "AbsTimeTag == 1");
}

void SC_CompareAbsTime_Test_True(void)
{
    bool Result;

    SC_AbsTimeTag_t AbsTimeTag1 = {0};
    SC_AbsTimeTag_t AbsTimeTag2 = {0};

    UT_SetDeferredRetcode(UT_KEY(CFE_TIME_Compare), 1, CFE_TIME_A_GT_B);

    /* Execute the function being tested */
    Result = SC_CompareAbsTime(AbsTimeTag1, AbsTimeTag2);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
}

void SC_CompareAbsTime_Test_False(void)
{
    bool Result;

    SC_AbsTimeTag_t AbsTimeTag1 = {0};
    SC_AbsTimeTag_t AbsTimeTag2 = {0};

    UT_SetDeferredRetcode(UT_KEY(CFE_TIME_Compare), 1, -1);

    /* Execute the function being tested */
    Result = SC_CompareAbsTime(AbsTimeTag1, AbsTimeTag2);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");
}

void SC_VerifyCmdLength_Test_Nominal(void)
{
    SC_NoArgsCmd_t    CmdPacket;
    bool              Result;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_NOOP_CC;
    size_t            MsgSize   = sizeof(CmdPacket);

    SC_InitTables();

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_VerifyCmdLength(&CmdPacket.CmdHeader.Msg, sizeof(CmdPacket));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
                  call_count_CFE_EVS_SendEvent);
}

void SC_VerifyCmdLength_Test_LenError(void)
{
    SC_NoArgsCmd_t    CmdPacket;
    bool              Result;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_NOOP_CC;
    size_t            MsgSize   = sizeof(CmdPacket);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid msg length: ID = 0x%%08lX, CC = %%d, Len = %%d, Expected = %%d");

    SC_InitTables();

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_VerifyCmdLength(&CmdPacket.CmdHeader.Msg, 999);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
                  call_count_CFE_EVS_SendEvent);
}

void SC_VerifyCmdLength_Test_LenErrorNotMID(void)
{
    SC_NoArgsCmd_t    CmdPacket;
    bool              Result;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_SEND_HK_MID);
    CFE_MSG_FcnCode_t FcnCode   = 0;
    size_t            MsgSize   = sizeof(CmdPacket);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid msg length: ID = 0x%%08lX, CC = %%d, Len = %%d, Expected = %%d");

    SC_InitTables();

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_VerifyCmdLength(&CmdPacket.CmdHeader.Msg, 999);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
                  call_count_CFE_EVS_SendEvent);
}

void SC_ToggleAtsIndex_Test(void)
{
    uint16 Result;

    SC_AtpControlBlock_t AtsCtrlBlck;

    SC_OperData.AtsCtrlBlckAddr = &AtsCtrlBlck;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 1;

    Result = SC_ToggleAtsIndex();

    UtAssert_True(Result == 1, "Result == 1");

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 2;

    Result = SC_ToggleAtsIndex();

    UtAssert_True(Result == 0, "Result == 0");
}

void UtTest_Setup(void)
{
    UtTest_Add(SC_GetCurrentTime_Test, SC_Test_Setup, SC_Test_TearDown, "SC_GetCurrentTime_Test");
    UtTest_Add(SC_GetAtsEntryTime_Test, SC_Test_Setup, SC_Test_TearDown, "SC_GetAtsEntryTime_Test");
    UtTest_Add(SC_ComputeAbsTime_Test, SC_Test_Setup, SC_Test_TearDown, "SC_ComputeAbsTime_Test");
    UtTest_Add(SC_CompareAbsTime_Test_True, SC_Test_Setup, SC_Test_TearDown, "SC_CompareAbsTime_Test_True");
    UtTest_Add(SC_CompareAbsTime_Test_False, SC_Test_Setup, SC_Test_TearDown, "SC_CompareAbsTime_Test_False");
    UtTest_Add(SC_VerifyCmdLength_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_VerifyCmdLength_Test_Nominal");
    UtTest_Add(SC_VerifyCmdLength_Test_LenError, SC_Test_Setup, SC_Test_TearDown, "SC_VerifyCmdLength_Test_LenError");
    UtTest_Add(SC_VerifyCmdLength_Test_LenErrorNotMID, SC_Test_Setup, SC_Test_TearDown,
               "SC_VerifyCmdLength_Test_LenErrorNotMID");
    UtTest_Add(SC_ToggleAtsIndex_Test, SC_Test_Setup, SC_Test_TearDown, "SC_ToggleAtsIndex_Test");
}
