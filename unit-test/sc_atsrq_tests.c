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

/*
 * Function Definitions
 */

CFE_TIME_Compare_t UT_SC_StartAtsRq_CompareHookAgreaterthanB(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                                             const UT_StubContext_t *Context)
{
    return CFE_TIME_A_GT_B;
}

uint8              UT_SC_StartAtsRq_CompareHookRunCount;
CFE_TIME_Compare_t UT_SC_StartAtsRq_CompareHook3(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                                 const UT_StubContext_t *Context)
{
    if (UT_SC_StartAtsRq_CompareHookRunCount == 0)
    {
        UT_SC_StartAtsRq_CompareHookRunCount++;
        return CFE_TIME_A_GT_B;
    }
    else
    {
        return CFE_TIME_A_LT_B;
    }
}

void SC_StartAtsCmd_Test_NominalA(void)
{
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.StartAtsCmd.AtsId                    = 1;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_IDLE;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING,
                  "SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_STARTATS_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_StartAtsCmd_Test_NominalB(void)
{
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.StartAtsCmd.AtsId                    = 2;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_IDLE;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING,
                  "SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_STARTATS_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_StartAtsCmd_Test_CouldNotStart(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    UT_CmdBuf.StartAtsCmd.AtsId                    = 1;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_IDLE;

    /* Set to cause SC_BeginAts to return false, in order to reach block starting with "could not start the ats" */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHookAgreaterthanB, NULL);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_SKP_ALL_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartAtsCmd_Test_NoCommandsA(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.StartAtsCmd.AtsId                    = 1;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 0;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_IDLE;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTATS_CMD_NOT_LDED_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartAtsCmd_Test_NoCommandsB(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.StartAtsCmd.AtsId                    = 2;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 0;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_IDLE;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTATS_CMD_NOT_LDED_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartAtsCmd_Test_InUse(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.StartAtsCmd.AtsId           = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTATS_CMD_NOT_IDLE_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartAtsCmd_Test_InvalidAtsId(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.StartAtsCmd.AtsId           = 99;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTATS_CMD_INVLD_ID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartAtsCmd_Test_InvalidAtsIdZero(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.StartAtsCmd.AtsId           = 0;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTATS_CMD_INVLD_ID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartAtsCmd_Test_InvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_StopAtsCmd_Test_NominalA(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSA;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPATS_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopAtsCmd_Test_NominalB(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSB;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPATS_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopAtsCmd_Test_NoRunningAts(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 99;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPATS_NO_ATS_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopAtsCmd_Test_InvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 99;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_BeginAts_Test_Nominal(void)
{
    uint16 AtsIndex   = 0;
    uint16 TimeOffset = 0;

    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 1;

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(SC_BeginAts(AtsIndex, TimeOffset));

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtsNumber == 1, "SC_OperData.AtsCtrlBlckAddr->AtsNumber == 1");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->CmdNumber == SC_AppData.AtsTimeIndexBuffer[AtsIndex][0],
                  "SC_OperData.AtsCtrlBlckAddr->CmdNumber == SC_AppData.AtsTimeIndexBuffer[AtsIndex][0]");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr == 0, "SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr == 0");
    UtAssert_True(SC_AppData.NextCmdTime[SC_ATP] == 0, "SC_AppData.NextCmdTime[SC_ATP] == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_ERR_SKP_DBG_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_BeginAts_Test_AllCommandsSkipped(void)
{
    uint16 AtsIndex   = 0;
    uint16 TimeOffset = 0;

    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    SC_AppData.AtsTimeIndexBuffer[0][0] = 1;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 1;

    /* Set to cause all commnds to be skipped, to generate error message SC_ATS_SKP_ALL_ERR_EID */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHookAgreaterthanB, NULL);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_BeginAts(AtsIndex, TimeOffset));

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_SKIPPED,
                  "SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_SKIPPED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_SKP_ALL_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_BeginAts_Test_InvalidAtsIndex(void)
{
    uint16 AtsIndex   = SC_NUMBER_OF_ATS;
    uint16 TimeOffset = 0;

    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    SC_AppData.AtsTimeIndexBuffer[0][0] = 1;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 1;

    /* Set to cause all commnds to be skipped, to generate error message SC_ATS_SKP_ALL_ERR_EID */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHookAgreaterthanB, NULL);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_BeginAts(AtsIndex, TimeOffset));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_BEGINATS_INVLD_INDEX_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_KillAts_Test(void)
{
    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = 99;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_KillAts());

    /* Verify results */
    UtAssert_True(SC_OperData.AtsInfoTblAddr[0].AtsUseCtr == 1, "SC_OperData.AtsInfoTblAddr[0].AtsUseCtr == 1");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_IDLE, "SC_OperData.AtsCtrlBlckAddr->AtpState == SC_IDLE");
    UtAssert_True(SC_AppData.NextCmdTime[SC_ATP] == SC_MAX_TIME, "SC_AppData.NextCmdTime[SC_ATP] == SC_MAX_TIME");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GroundSwitchCmd_Test_Nominal(void)
{
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_1HZ_WAKEUP_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_EXECUTING;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    UT_SetDeferredRetcode(UT_KEY(SC_ToggleAtsIndex), 1, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GroundSwitchCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == true,
                  "SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == true");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_SWITCH_ATS_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GroundSwitchCmd_Test_DestinationAtsNotLoaded(void)
{
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_1HZ_WAKEUP_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_EXECUTING;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GroundSwitchCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false,
                  "SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_SWITCH_ATS_CMD_NOT_LDED_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GroundSwitchCmd_Test_AtpIdle(void)
{
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_1HZ_WAKEUP_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = 99;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GroundSwitchCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false,
                  "SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_SWITCH_ATS_CMD_IDLE_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GroundSwitchCmd_Test_InvalidCmdLength(void)
{
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_1HZ_WAKEUP_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_GroundSwitchCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ServiceSwitchPend_Test_NominalA(void)
{
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    /* Set to satisfy first if-statement, while not affecting later calls to CFE_TIME_Compare */
    UT_SC_StartAtsRq_CompareHookRunCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHook3, NULL);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_EXECUTING;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 1;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    UT_SetDeferredRetcode(UT_KEY(SC_ToggleAtsIndex), 1, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ServiceSwitchPend());

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING,
                  "SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false,
                  "SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_SERVICE_SWTCH_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ServiceSwitchPend_Test_NominalB(void)
{
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    /* Set to satisfy first if-statement, while not affecting later calls to CFE_TIME_Compare */
    UT_SC_StartAtsRq_CompareHookRunCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHook3, NULL);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 2;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_EXECUTING;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 1;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    UT_SetDeferredRetcode(UT_KEY(SC_ToggleAtsIndex), 1, 0);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ServiceSwitchPend());

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING,
                  "SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false,
                  "SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_SERVICE_SWTCH_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ServiceSwitchPend_Test_AtsEmpty(void)
{
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    /* Set to satisfy first if-statement, while not affecting later calls to CFE_TIME_Compare */
    UT_SC_StartAtsRq_CompareHookRunCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHook3, NULL);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_EXECUTING;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 0;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ServiceSwitchPend());

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false,
                  "SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_SERVICE_SWITCH_ATS_CMD_LDED_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ServiceSwitchPend_Test_AtpIdle(void)
{
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    /* Set to satisfy first if-statement, while not affecting later calls to CFE_TIME_Compare */
    UT_SC_StartAtsRq_CompareHookRunCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHook3, NULL);

    SC_OperData.AtsCtrlBlckAddr->AtpState = 99;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ServiceSwitchPend());

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false,
                  "SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_SERVICE_SWITCH_IDLE_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ServiceSwitchPend_Test_NoSwitch(void)
{
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, false);

    /* Set to satisfy first if-statement, while not affecting later calls to CFE_TIME_Compare */
    UT_SC_StartAtsRq_CompareHookRunCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHook3, NULL);

    /* Execute the function being tested */
    SC_ServiceSwitchPend();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ServiceSwitchPend_Test_AtsNotStarted(void)
{
    /* Set to cause SC_BeginAts to return false, in order to reach block starting with "could not start the ats" */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHookAgreaterthanB, NULL);

    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    /* Set to satisfy first if-statement, while not affecting later calls to CFE_TIME_Compare */
    UT_SC_StartAtsRq_CompareHookRunCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHook3, NULL);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_EXECUTING;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 1;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ServiceSwitchPend());

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_IDLE, "SC_OperData.AtsCtrlBlckAddr->AtpState == SC_IDLE");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false,
                  "SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_SKP_ALL_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_InlineSwitch_Test_NominalA(void)
{
    /* Set to satisfy first if-statement, while not affecting later calls to CFE_TIME_Compare */
    UT_SC_StartAtsRq_CompareHookRunCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHook3, NULL);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 1;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 1;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    UT_SetDeferredRetcode(UT_KEY(SC_ToggleAtsIndex), 1, 1);

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(SC_InlineSwitch());

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_STARTING,
                  "SC_OperData.AtsCtrlBlckAddr->AtpState == SC_STARTING");

    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false,
                  "SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_INLINE_SWTCH_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_InlineSwitch_Test_NominalB(void)
{
    /* Set to satisfy first if-statement, while not affecting later calls to CFE_TIME_Compare */
    UT_SC_StartAtsRq_CompareHookRunCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHook3, NULL);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 2;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 1;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    UT_SetDeferredRetcode(UT_KEY(SC_ToggleAtsIndex), 1, 0);

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(SC_InlineSwitch());

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_STARTING,
                  "SC_OperData.AtsCtrlBlckAddr->AtpState == SC_STARTING");

    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false,
                  "SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_INLINE_SWTCH_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_InlineSwitch_Test_AllCommandsSkipped(void)
{
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    /* Set to cause all commnds to be skipped, to reach block starting with comment "all of the commands in the new ats
     * were skipped" */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHookAgreaterthanB, NULL);

    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 1;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 1;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_InlineSwitch());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false,
                  "SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_InlineSwitch_Test_DestinationAtsNotLoaded(void)
{
    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 1;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 0;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 0;

    UT_SetDeferredRetcode(UT_KEY(SC_ToggleAtsIndex), 1, 0);
    UT_SetDeferredRetcode(UT_KEY(SC_ToggleAtsIndex), 1, 1);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_InlineSwitch());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false,
                  "SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_INLINE_SWTCH_NOT_LDED_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_JumpAtsCmd_Test_SkipOneCmd(void)
{
    uint8             AtsIndex  = 0;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_JUMP_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_JumpAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    /* Set to satisfy first if-statement, while not affecting later calls to CFE_TIME_Compare */
    UT_SC_StartAtsRq_CompareHookRunCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHook3, NULL);

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0]   = SC_LOADED;
    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][1]   = SC_LOADED;
    SC_AppData.AtsTimeIndexBuffer[AtsIndex][0]     = 1;
    SC_AppData.AtsTimeIndexBuffer[AtsIndex][1]     = 2;
    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_EXECUTING;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber         = 2;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 2;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_JumpAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_SKIPPED,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_SKIPPED");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][1] == SC_LOADED,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][1] == SC_LOADED");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->CmdNumber == SC_AppData.AtsTimeIndexBuffer[AtsIndex][1],
                  "SC_OperData.AtsCtrlBlckAddr->CmdNumber == SC_AppData.AtsTimeIndexBuffer[AtsIndex][0]");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr == 1, "SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr == 1");
    UtAssert_True(SC_AppData.NextCmdTime[0] == 0, "SC_AppData.NextCmdTime[0] == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_JUMP_ATS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, SC_JUMP_ATS_SKIPPED_DBG_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 3);
}

void SC_JumpAtsCmd_Test_AllCommandsSkipped(void)
{
    uint8             AtsIndex  = 0;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_JUMP_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_JumpAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    /* Set to satisfy first if-statement, while not affecting later calls to CFE_TIME_Compare */
    UT_SC_StartAtsRq_CompareHookRunCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHook3, NULL);

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0]   = SC_LOADED;
    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_EXECUTING;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_JumpAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_JUMPATS_CMD_STOPPED_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_JumpAtsCmd_Test_NoRunningAts(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_JUMP_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_JumpAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_IDLE;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_JumpAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_JUMPATS_CMD_NOT_ACT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_JumpAtsCmd_Test_AtsNotLoaded(void)
{
    uint8             AtsIndex  = 0;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_JUMP_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_JumpAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    /* Set to satisfy first if-statement, while not affecting later calls to CFE_TIME_Compare */
    UT_SC_StartAtsRq_CompareHookRunCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), UT_SC_StartAtsRq_CompareHook3, NULL);

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0]   = SC_SKIPPED;
    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][1]   = SC_SKIPPED;
    SC_OperData.AtsCtrlBlckAddr->AtsNumber         = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState          = SC_EXECUTING;
    SC_OperData.AtsInfoTblAddr[0].NumberOfCommands = 2;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_JumpAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_SKIPPED,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_SKIPPED");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][1] == SC_SKIPPED,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][1] == SC_SKIPPED");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->CmdNumber == SC_AppData.AtsTimeIndexBuffer[AtsIndex][0],
                  "SC_OperData.AtsCtrlBlckAddr->CmdNumber == SC_AppData.AtsTimeIndexBuffer[AtsIndex][0]");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr == 1, "SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr == 1");
    UtAssert_True(SC_AppData.NextCmdTime[0] == 0, "SC_AppData.NextCmdTime[0] == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_JUMP_ATS_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_JumpAtsCmd_Test_InvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_JUMP_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_JumpAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_IDLE;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_JumpAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void ContinueAtsOnFailureCmd_Test_Nominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_CONTINUE_ATS_ON_FAILURE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_SetContinueAtsOnFailureCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.SetContinueAtsOnFailureCmd.ContinueState = true;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ContinueAtsOnFailureCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.ContinueAtsOnFailureFlag == true,
                  "SC_OperData.HkPacket.ContinueAtsOnFailureFlag == true");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CONT_CMD_DEB_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void ContinueAtsOnFailureCmd_Test_FalseState(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_CONTINUE_ATS_ON_FAILURE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_SetContinueAtsOnFailureCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.SetContinueAtsOnFailureCmd.ContinueState = false;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ContinueAtsOnFailureCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.ContinueAtsOnFailureFlag == false,
                  "SC_OperData.HkPacket.ContinueAtsOnFailureFlag == false");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CONT_CMD_DEB_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void ContinueAtsOnFailureCmd_Test_InvalidState(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_CONTINUE_ATS_ON_FAILURE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_SetContinueAtsOnFailureCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.SetContinueAtsOnFailureCmd.ContinueState = 99;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ContinueAtsOnFailureCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CONT_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void ContinueAtsOnFailureCmd_Test_InvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_CONTINUE_ATS_ON_FAILURE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_SetContinueAtsOnFailureCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ContinueAtsOnFailureCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_AppendAtsCmd_Test_Nominal(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_APPEND_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    Entry                            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber                 = 1;

    UT_CmdBuf.AppendAtsCmd.AtsId                          = 1;
    SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands = 1;
    SC_OperData.HkPacket.AppendEntryCount                 = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AppendAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AppendCmdArg == 1, "SC_OperData.HkPacket.AppendCmdArg == 1");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_APPEND_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AppendAtsCmd_Test_InvalidAtsId(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_APPEND_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.AppendAtsCmd.AtsId          = 99;
    SC_OperData.HkPacket.AppendEntryCount = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AppendAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_APPEND_CMD_ARG_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AppendAtsCmd_Test_InvalidAtsIdZero(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_APPEND_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.AppendAtsCmd.AtsId          = 0;
    SC_OperData.HkPacket.AppendEntryCount = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AppendAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_APPEND_CMD_ARG_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AppendAtsCmd_Test_AtsTableEmpty(void)
{
    uint8             AtsIndex  = 0;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_APPEND_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.AppendAtsCmd.AtsId                          = 1;
    SC_OperData.HkPacket.AppendEntryCount                 = 1;
    SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AppendAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_APPEND_CMD_TGT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AppendAtsCmd_Test_AppendTableEmpty(void)
{
    uint8             AtsIndex  = 0;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_APPEND_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.AppendAtsCmd.AtsId                          = 1;
    SC_OperData.HkPacket.AppendEntryCount                 = 0;
    SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AppendAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_APPEND_CMD_SRC_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AppendAtsCmd_Test_NoRoomForAppendInAts(void)
{
    uint8             AtsIndex  = 0;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_APPEND_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.AppendAtsCmd.AtsId                          = 1;
    SC_OperData.HkPacket.AppendEntryCount                 = 1;
    SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands = 1;
    SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize          = SC_ATS_BUFF_SIZE;
    SC_AppData.AppendWordCount                            = SC_ATS_BUFF_SIZE;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AppendAtsCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_APPEND_CMD_FIT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AppendAtsCmd_Test_InvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_APPEND_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_AppendAtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void UtTest_Setup(void)
{
    UtTest_Add(SC_StartAtsCmd_Test_NominalA, SC_Test_Setup, SC_Test_TearDown, "SC_StartAtsCmd_Test_NominalA");
    UtTest_Add(SC_StartAtsCmd_Test_NominalB, SC_Test_Setup, SC_Test_TearDown, "SC_StartAtsCmd_Test_NominalB");
    UtTest_Add(SC_StartAtsCmd_Test_CouldNotStart, SC_Test_Setup, SC_Test_TearDown, "SC_StartAtsCmd_Test_CouldNotStart");
    UtTest_Add(SC_StartAtsCmd_Test_NoCommandsA, SC_Test_Setup, SC_Test_TearDown, "SC_StartAtsCmd_Test_NoCommandsA");
    UtTest_Add(SC_StartAtsCmd_Test_NoCommandsB, SC_Test_Setup, SC_Test_TearDown, "SC_StartAtsCmd_Test_NoCommandsB");
    UtTest_Add(SC_StartAtsCmd_Test_InUse, SC_Test_Setup, SC_Test_TearDown, "SC_StartAtsCmd_Test_InUse");
    UtTest_Add(SC_StartAtsCmd_Test_InvalidAtsId, SC_Test_Setup, SC_Test_TearDown, "SC_StartAtsCmd_Test_InvalidAtsId");
    UtTest_Add(SC_StartAtsCmd_Test_InvalidAtsIdZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartAtsCmd_Test_InvalidAtsIdZero");
    UtTest_Add(SC_StartAtsCmd_Test_InvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartAtsCmd_Test_InvalidCmdLength");
    UtTest_Add(SC_StopAtsCmd_Test_NominalA, SC_Test_Setup, SC_Test_TearDown, "SC_StopAtsCmd_Test_NominalA");
    UtTest_Add(SC_StopAtsCmd_Test_NominalB, SC_Test_Setup, SC_Test_TearDown, "SC_StopAtsCmd_Test_NominalB");
    UtTest_Add(SC_StopAtsCmd_Test_NoRunningAts, SC_Test_Setup, SC_Test_TearDown, "SC_StopAtsCmd_Test_NoRunningAts");
    UtTest_Add(SC_StopAtsCmd_Test_InvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_StopAtsCmd_Test_InvalidCmdLength");
    UtTest_Add(SC_BeginAts_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_BeginAts_Test_Nominal");
    UtTest_Add(SC_BeginAts_Test_AllCommandsSkipped, SC_Test_Setup, SC_Test_TearDown,
               "SC_BeginAts_Test_AllCommandsSkipped");
    UtTest_Add(SC_BeginAts_Test_InvalidAtsIndex, SC_Test_Setup, SC_Test_TearDown, "SC_BeginAts_Test_InvalidAtsIndex");
    UtTest_Add(SC_KillAts_Test, SC_Test_Setup, SC_Test_TearDown, "SC_KillAts_Test");
    UtTest_Add(SC_GroundSwitchCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_GroundSwitchCmd_Test_Nominal");
    UtTest_Add(SC_GroundSwitchCmd_Test_DestinationAtsNotLoaded, SC_Test_Setup, SC_Test_TearDown,
               "SC_GroundSwitchCmd_Test_DestinationAtsNotLoaded");
    UtTest_Add(SC_GroundSwitchCmd_Test_AtpIdle, SC_Test_Setup, SC_Test_TearDown, "SC_GroundSwitchCmd_Test_AtpIdle");
    UtTest_Add(SC_GroundSwitchCmd_Test_InvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_GroundSwitchCmd_Test_InvalidCmdLength");
    UtTest_Add(SC_ServiceSwitchPend_Test_NominalA, SC_Test_Setup, SC_Test_TearDown,
               "SC_ServiceSwitchPend_Test_NominalA");
    UtTest_Add(SC_ServiceSwitchPend_Test_NominalB, SC_Test_Setup, SC_Test_TearDown,
               "SC_ServiceSwitchPend_Test_NominalB");
    UtTest_Add(SC_ServiceSwitchPend_Test_AtsEmpty, SC_Test_Setup, SC_Test_TearDown,
               "SC_ServiceSwitchPend_Test_AtsEmpty");
    UtTest_Add(SC_ServiceSwitchPend_Test_AtpIdle, SC_Test_Setup, SC_Test_TearDown, "SC_ServiceSwitchPend_Test_AtpIdle");
    UtTest_Add(SC_ServiceSwitchPend_Test_NoSwitch, SC_Test_Setup, SC_Test_TearDown,
               "SC_ServiceSwitchPend_Test_NoSwitch");
    UtTest_Add(SC_ServiceSwitchPend_Test_AtsNotStarted, SC_Test_Setup, SC_Test_TearDown,
               "SC_ServiceSwitchPend_Test_AtsNotStarted");
    UtTest_Add(SC_InlineSwitch_Test_NominalA, SC_Test_Setup, SC_Test_TearDown, "SC_InlineSwitch_Test_NominalA");
    UtTest_Add(SC_InlineSwitch_Test_NominalB, SC_Test_Setup, SC_Test_TearDown, "SC_InlineSwitch_Test_NominalB");
    UtTest_Add(SC_InlineSwitch_Test_AllCommandsSkipped, SC_Test_Setup, SC_Test_TearDown,
               "SC_InlineSwitch_Test_AllCommandsSkipped");
    UtTest_Add(SC_InlineSwitch_Test_DestinationAtsNotLoaded, SC_Test_Setup, SC_Test_TearDown,
               "SC_InlineSwitch_Test_DestinationAtsNotLoaded");
    UtTest_Add(SC_JumpAtsCmd_Test_SkipOneCmd, SC_Test_Setup, SC_Test_TearDown, "SC_JumpAtsCmd_Test_SkipOneCmd");
    UtTest_Add(SC_JumpAtsCmd_Test_AllCommandsSkipped, SC_Test_Setup, SC_Test_TearDown,
               "SC_JumpAtsCmd_Test_AllCommandsSkipped");
    UtTest_Add(SC_JumpAtsCmd_Test_NoRunningAts, SC_Test_Setup, SC_Test_TearDown, "SC_JumpAtsCmd_Test_NoRunningAts");
    UtTest_Add(SC_JumpAtsCmd_Test_AtsNotLoaded, SC_Test_Setup, SC_Test_TearDown, "SC_JumpAtsCmd_Test_AtsNotLoaded");
    UtTest_Add(SC_JumpAtsCmd_Test_InvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_JumpAtsCmd_Test_InvalidCmdLength");
    UtTest_Add(ContinueAtsOnFailureCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown,
               "ContinueAtsOnFailureCmd_Test_Nominal");
    UtTest_Add(ContinueAtsOnFailureCmd_Test_FalseState, SC_Test_Setup, SC_Test_TearDown,
               "ContinueAtsOnFailureCmd_Test_FalseState");
    UtTest_Add(ContinueAtsOnFailureCmd_Test_InvalidState, SC_Test_Setup, SC_Test_TearDown,
               "ContinueAtsOnFailureCmd_Test_InvalidState");
    UtTest_Add(ContinueAtsOnFailureCmd_Test_InvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "ContinueAtsOnFailureCmd_Test_InvalidCmdLength");
    UtTest_Add(SC_AppendAtsCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_AppendAtsCmd_Test_Nominal");
    UtTest_Add(SC_AppendAtsCmd_Test_InvalidAtsId, SC_Test_Setup, SC_Test_TearDown, "SC_AppendAtsCmd_Test_InvalidAtsId");
    UtTest_Add(SC_AppendAtsCmd_Test_InvalidAtsIdZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppendAtsCmd_Test_InvalidAtsIdZero");
    UtTest_Add(SC_AppendAtsCmd_Test_AtsTableEmpty, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppendAtsCmd_Test_AtsTableEmpty");
    UtTest_Add(SC_AppendAtsCmd_Test_AppendTableEmpty, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppendAtsCmd_Test_AppendTableEmpty");
    UtTest_Add(SC_AppendAtsCmd_Test_NoRoomForAppendInAts, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppendAtsCmd_Test_NoRoomForAppendInAts");
    UtTest_Add(SC_AppendAtsCmd_Test_InvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppendAtsCmd_Test_InvalidCmdLength");
}
