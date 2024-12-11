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

#include "sc_cmds.h"
#include "sc_dispatch.h"
#include "sc_atsrq.h"
#include "sc_rtsrq.h"
#include "sc_state.h"
#include "sc_events.h"
#include "sc_msgids.h"
#include "sc_test_utils.h"
#include "sc_utils.h"
#include "sc_version.h"
#include "cfe_tbl_msg.h"
#include <time.h>

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

void UT_SC_Dispatch_MsgSizeHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    CFE_MSG_Size_t *Size            = UT_Hook_GetArgValueByName(Context, "Size", CFE_MSG_Size_t *);
    CFE_MSG_Size_t *TestCaseMsgSize = UserObj;

    *Size = *TestCaseMsgSize;
}

void UT_SC_Dispatch_MsgIdHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    CFE_SB_MsgId_t *MsgId         = UT_Hook_GetArgValueByName(Context, "MsgId", CFE_SB_MsgId_t *);
    CFE_SB_MsgId_t *TestCaseMsgId = UserObj;

    *MsgId = *TestCaseMsgId;
}

void UT_SC_Dispatch_FcnCodeHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    CFE_MSG_FcnCode_t *FcnCode         = UT_Hook_GetArgValueByName(Context, "FcnCode", CFE_MSG_FcnCode_t *);
    CFE_MSG_FcnCode_t *TestCaseFcnCode = UserObj;

    *FcnCode = *TestCaseFcnCode;
}

void UT_SC_Dispatch_SetMsgSize(CFE_MSG_Size_t MsgSize)
{
    static CFE_MSG_Size_t TestCaseMsgSize;

    TestCaseMsgSize = MsgSize;

    UT_SetHandlerFunction(UT_KEY(CFE_MSG_GetSize), UT_SC_Dispatch_MsgSizeHandler, &TestCaseMsgSize);
}

void UT_SC_Dispatch_SetMsgId(CFE_SB_MsgId_t MsgId)
{
    static CFE_SB_MsgId_t TestCaseMsgId;

    TestCaseMsgId = MsgId;

    UT_SetHandlerFunction(UT_KEY(CFE_MSG_GetMsgId), UT_SC_Dispatch_MsgIdHandler, &TestCaseMsgId);
}

void UT_SC_Dispatch_SetFcnCode(CFE_MSG_FcnCode_t FcnCode)
{
    static CFE_MSG_FcnCode_t TestCaseFcnCode;

    TestCaseFcnCode = FcnCode;

    UT_SetHandlerFunction(UT_KEY(CFE_MSG_GetFcnCode), UT_SC_Dispatch_FcnCodeHandler, &TestCaseFcnCode);
}

void SC_VerifyCmdLength_Test_Nominal(void)
{
    SC_NoopCmd_t      CmdPacket;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_NOOP_CC;
    size_t            MsgSize   = sizeof(CmdPacket);

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(MsgSize);

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(SC_VerifyCmdLength(CFE_MSG_PTR(CmdPacket.CommandHeader), sizeof(CmdPacket)));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_VerifyCmdLength_Test_LenError(void)
{
    SC_NoopCmd_t      CmdPacket;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_NOOP_CC;
    size_t            MsgSize   = sizeof(CmdPacket);

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(MsgSize);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_VerifyCmdLength(CFE_MSG_PTR(CmdPacket.CommandHeader), 999));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_VerifyCmdLength_Test_LenErrorNotMID(void)
{
    SC_NoopCmd_t      CmdPacket;
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_SEND_HK_MID);
    CFE_MSG_FcnCode_t FcnCode   = 0;
    size_t            MsgSize   = sizeof(CmdPacket);

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(MsgSize);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_VerifyCmdLength(CFE_MSG_PTR(CmdPacket.CommandHeader), 999));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessRequest_Test_CmdNominal(void)
{
    /**
     **  Test case: SC_CMD_MID
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_NOOP_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_NoopCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(SC_NoopCmd, 1);
}

void SC_ProcessRequest_Test_SendHkNominal(void)
{
    /**
     **  Test case: SC_SEND_HK_MID
     **/

    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_SEND_HK_MID);

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(0);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_SendHkCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(SC_SendHkCmd, 1);
}

void SC_ProcessRequest_Test_SendHkCmdInvalidLength(void)
{
    /**
     **  Test case: SC_SEND_HK_MID
     **/

    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_SEND_HK_MID);

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(0);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
    UtAssert_STUB_COUNT(SC_SendHkCmd, 0);
}

void SC_ProcessRequest_Test_WakeupNominal(void)
{
    /**
     **  Test case: SC_WAKEUP_MID
     **/

    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_WAKEUP_MID);

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(0);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_WakeupCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(SC_WakeupCmd, 1);
}

void SC_ProcessRequest_Test_WakeupCmdInvalidLength(void)
{
    /**
     **  Test case: SC_WAKEUP_MID
     **/

    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_WAKEUP_MID);

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(0);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
    UtAssert_STUB_COUNT(SC_WakeupCmd, 0);
}

void SC_ProcessRequest_Test_MIDError(void)
{
    /**
     **  Test case: SC_MID_ERR_EID
     **/

    CFE_SB_MsgId_t TestMsgId = SC_UT_MID_1;

    UT_SC_Dispatch_SetMsgId(TestMsgId);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_MID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_NoopCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_NOOP_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
    UtAssert_STUB_COUNT(SC_NoopCmd, 0);
}

void SC_ProcessCommand_Test_ResetCounterCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_RESET_COUNTERS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
    UtAssert_STUB_COUNT(SC_ResetCountersCmd, 0);
}

void SC_ProcessCommand_Test_StartAtsCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_StopAtsCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_STOP_ATS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_StartRtsCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_RTS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_StopRtsCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_STOP_RTS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_DisableRtsCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_DISABLE_RTS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_EnableRtsCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_ENABLE_RTS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_SwitchAtsCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_SWITCH_ATS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_JumpAtsCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_JUMP_ATS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_ContinueAtsOnFailureCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_CONTINUE_ATS_ON_FAILURE_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_AppendAtsCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_APPEND_ATS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_TableManageCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_StartRtsGrpCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_RTS_GRP_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_StopRtsGrpCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_STOP_RTS_GRP_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_DisableRtsGrpCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_DISABLE_RTS_GRP_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_EnableRtsGrpCmdInvalidLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_ENABLE_RTS_GRP_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(CFE_MSG_Message_t) - 1);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CMD_LEN_ERR_EID);
}

void SC_ProcessCommand_Test_NoopCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_NOOP_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_NoopCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(SC_NoopCmd, 1);
}

void SC_ProcessCommand_Test_ResetCounterCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_RESET_COUNTERS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_ResetCountersCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(SC_ResetCountersCmd, 1);
}

void SC_ProcessCommand_Test_StartAtsCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_StartAtsCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_StartAtsCmd, 1);
}

void SC_ProcessCommand_Test_StopAtsCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_STOP_ATS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_StopAtsCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_StopAtsCmd, 1);
}

void SC_ProcessCommand_Test_StartRtsCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_RTS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_StartRtsCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_StartRtsCmd, 1);
}

void SC_ProcessCommand_Test_StopRtsCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_STOP_RTS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_StopRtsCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_StopRtsCmd, 1);
}

void SC_ProcessCommand_Test_DisableRtsCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_DISABLE_RTS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_DisableRtsCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_DisableRtsCmd, 1);
}

void SC_ProcessCommand_Test_EnableRtsCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_ENABLE_RTS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_EnableRtsCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_EnableRtsCmd, 1);
}

void SC_ProcessCommand_Test_SwitchAtsCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_SWITCH_ATS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_SwitchAtsCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_SwitchAtsCmd, 1);
}

void SC_ProcessCommand_Test_JumpAtsCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_JUMP_ATS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_JumpAtsCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_JumpAtsCmd, 1);
}

void SC_ProcessCommand_Test_ContinueAtsOnFailureCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_CONTINUE_ATS_ON_FAILURE_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_ContinueAtsOnFailureCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_ContinueAtsOnFailureCmd, 1);
}

void SC_ProcessCommand_Test_AppendAtsCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_APPEND_ATS_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_AppendAtsCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_AppendAtsCmd, 1);
}

void SC_ProcessCommand_Test_TableManageCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_ManageTableCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_ManageTableCmd, 1);
}

void SC_ProcessCommand_Test_StartRtsGrpCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_RTS_GRP_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_StartRtsGrpCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_StartRtsGrpCmd, 1);
}

void SC_ProcessCommand_Test_StopRtsGrpCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_STOP_RTS_GRP_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_StopRtsGrpCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_StopRtsGrpCmd, 1);
}

void SC_ProcessCommand_Test_DisableRtsGrpCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_DISABLE_RTS_GRP_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_DisableRtsGrpCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_DisableRtsGrpCmd, 1);
}

void SC_ProcessCommand_Test_EnableRtsGrpCmdNominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_ENABLE_RTS_GRP_CC;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);
    UT_SC_Dispatch_SetMsgSize(sizeof(SC_EnableRtsGrpCmd_t));

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(SC_EnableRtsGrpCmd, 1);
}

void SC_ProcessCommand_Test_InvalidCmdError(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = 99;

    UT_SC_Dispatch_SetMsgId(TestMsgId);
    UT_SC_Dispatch_SetFcnCode(FcnCode);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdCtr, 0);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.CmdErrCtr, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CC_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

/* Unreachable branches in sc_cmds.c SC_ProcessAtpCmd:236, 274, 310.
   There are only 2 ATS IDs defined, invalid IDs are already handled. */

void UtTest_Setup(void)
{
    UtTest_Add(SC_VerifyCmdLength_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_VerifyCmdLength_Test_Nominal");
    UtTest_Add(SC_VerifyCmdLength_Test_LenError, SC_Test_Setup, SC_Test_TearDown, "SC_VerifyCmdLength_Test_LenError");
    UtTest_Add(SC_VerifyCmdLength_Test_LenErrorNotMID, SC_Test_Setup, SC_Test_TearDown,
               "SC_VerifyCmdLength_Test_LenErrorNotMID");

    UtTest_Add(SC_ProcessRequest_Test_CmdNominal, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessRequest_Test_CmdNominal");
    UtTest_Add(SC_ProcessRequest_Test_SendHkNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_SendHkNominal");
    UtTest_Add(SC_ProcessRequest_Test_SendHkCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_SendHkCmdInvalidLength");
    UtTest_Add(SC_ProcessRequest_Test_WakeupCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupCmdInvalidLength");
    UtTest_Add(SC_ProcessRequest_Test_WakeupNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupNominal");
    UtTest_Add(SC_ProcessRequest_Test_MIDError, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessRequest_Test_MIDError");
#ifdef jphfix
    UtTest_Add(SC_ProcessRequest_Test_HkMIDAutoStartRts, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_HkMIDAutoStartRts");
    UtTest_Add(SC_ProcessRequest_Test_HkMIDAutoStartRtsLoaded, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_HkMIDAutoStartRtsLoaded");
    UtTest_Add(SC_ProcessRequest_Test_WakeupNONE, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupNONE");
    UtTest_Add(SC_ProcessRequest_Test_WakeupNoSwitchPending, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupNoSwitchPending");
    UtTest_Add(SC_ProcessRequest_Test_WakeupAtpNotExecutionTime, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupAtpNotExecutionTime");
    UtTest_Add(SC_ProcessRequest_Test_WakeupRtpExecutionTime, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupRtpExecutionTime");
    UtTest_Add(SC_ProcessRequest_Test_WakeupRtpExecutionTimeTooManyCmds, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupRtpExecutionTimeTooManyCmds");
#endif

    UtTest_Add(SC_ProcessCommand_Test_NoopCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_NoopCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_ResetCounterCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_ResetCounterCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_StartAtsCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StartAtsCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_StopAtsCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StopAtsCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_StartRtsCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StartRtsCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_StopRtsCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StopRtsCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_DisableRtsCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_DisableRtsCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_EnableRtsCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_EnableRtsCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_SwitchAtsCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_SwitchAtsCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_JumpAtsCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_JumpAtsCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_ContinueAtsOnFailureCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_ContinueAtsOnFailureCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_AppendAtsCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_AppendAtsCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_TableManageCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_StartRtsGrpCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StartRtsGrpCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_StopRtsGrpCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StopRtsGrpCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_DisableRtsGrpCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_DisableRtsGrpCmdNominal");
    UtTest_Add(SC_ProcessCommand_Test_EnableRtsGrpCmdNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_EnableRtsGrpCmdNominal");

    UtTest_Add(SC_ProcessCommand_Test_NoopCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_NoopCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_ResetCounterCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_ResetCounterCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_StartAtsCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StartAtsCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_StopAtsCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StopAtsCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_StartRtsCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StartRtsCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_StopRtsCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StopRtsCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_DisableRtsCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_DisableRtsCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_EnableRtsCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_EnableRtsCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_SwitchAtsCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_SwitchAtsCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_JumpAtsCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_JumpAtsCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_ContinueAtsOnFailureCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_ContinueAtsOnFailureCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_AppendAtsCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_AppendAtsCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_TableManageCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_StartRtsGrpCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StartRtsGrpCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_StopRtsGrpCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StopRtsGrpCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_DisableRtsGrpCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_DisableRtsGrpCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_EnableRtsGrpCmdInvalidLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_EnableRtsGrpCmdInvalidLength");
    UtTest_Add(SC_ProcessCommand_Test_InvalidCmdError, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_InvalidCmdError");
}
