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

/* sc_cmds_tests globals */

/*
 * Function Definitions
 */

int32 Ut_CFE_TIME_CompareHookAlessthanB(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                        const UT_StubContext_t *Context)
{
    return CFE_TIME_A_LT_B;
}

void SC_ProcessAtpCmd_Test_SwitchCmd(void)
{
    SC_AtsEntryHeader_t *         Entry;
    CFE_SB_MsgId_t                TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t             FcnCode   = SC_SWITCH_ATS_CC;
    SC_AtsIndex_t                 AtsIndex  = SC_ATS_IDX_C(0);
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));
    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_AtsIndexToNum(AtsIndex);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    SC_AppData.EnableHeaderUpdate = true;

    /* Set return value for CFE_TIME_Compare to make SC_CompareAbsTime return false, to satisfy first if-statement of
     * SC_ProcessAtpCmd, and for all other calls to CFE_TIME_Compare called from subfunctions reached by this test */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), Ut_CFE_TIME_CompareHookAlessthanB, NULL);

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_InlineSwitch), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 1, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 0");
    UtAssert_True(SC_OperData.NumCmdsWakeup == 1, "SC_OperData.NumCmdsWakeup == 1");
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_EXECUTED);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAtpCmd_Test_NonSwitchCmd(void)
{
    SC_AtsEntryHeader_t *         Entry;
    CFE_SB_MsgId_t                TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t             FcnCode   = SC_NOOP_CC;
    SC_AtsIndex_t                 AtsIndex  = SC_ATS_IDX_C(0);
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_AtsIndexToNum(AtsIndex);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    SC_AppData.EnableHeaderUpdate = true;

    /* Set return value for CFE_TIME_Compare to make SC_CompareAbsTime return false, to satisfy first if-statement of
     * SC_ProcessAtpCmd, and for all other calls to CFE_TIME_Compare called from subfunctions reached by this test */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), Ut_CFE_TIME_CompareHookAlessthanB, NULL);

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_InlineSwitch), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 1, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 0");
    UtAssert_True(SC_OperData.NumCmdsWakeup == 1, "SC_OperData.NumCmdsWakeup == 1");
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_EXECUTED);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAtpCmd_Test_InlineSwitchError(void)
{
    SC_AtsEntryHeader_t *         Entry;
    CFE_SB_MsgId_t                TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t             FcnCode   = SC_SWITCH_ATS_CC;
    SC_AtsIndex_t                 AtsIndex  = SC_ATS_IDX_C(0);
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_AtsIndexToNum(AtsIndex);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    /* Set return value for CFE_TIME_Compare to make SC_CompareAbsTime return false, to satisfy first if-statement of
     * SC_ProcessAtpCmd, and for all other calls to CFE_TIME_Compare called from subfunctions reached by this test */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), Ut_CFE_TIME_CompareHookAlessthanB, NULL);

    /* Set to return true in order to satisfy the if-statement from which the function is called */
    SC_AppData.EnableHeaderUpdate = true;

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_InlineSwitch), 1, false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.NumCmdsWakeup == 1, "SC_OperData.NumCmdsWakeup == 1");
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_FAILED_DISTRIB);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrSeq, 1);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrCmd, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAtpCmd_Test_SBErrorAtsA(void)
{
    SC_AtsEntryHeader_t *         Entry;
    CFE_SB_MsgId_t                TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t             FcnCode   = SC_NOOP_CC;
    SC_AtsIndex_t                 AtsIndex  = SC_ATS_IDX_C(0);
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_AppData.NextCmdTime[SC_Process_ATP] = 0;
    SC_AppData.CurrentTime                 = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(SC_AtsId_ATSA);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    SC_AppData.EnableHeaderUpdate = true;

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* Set to return -1 in order to generate error message SC_ATS_DIST_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_TransmitMsg), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.NumCmdsWakeup == 1, "SC_OperData.NumCmdsWakeup == 1");
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_FAILED_DISTRIB);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrSeq, SC_AtsId_ATSA);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrCmd, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_DIST_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_ABT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ProcessAtpCmd_Test_SBErrorAtsB(void)
{
    SC_AtsEntryHeader_t *         Entry;
    CFE_SB_MsgId_t                TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t             FcnCode   = SC_NOOP_CC;
    SC_AtsIndex_t                 AtsIndex  = SC_ATS_IDX_C(1);
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_AppData.NextCmdTime[SC_Process_ATP] = 0;
    SC_AppData.CurrentTime                 = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(SC_AtsId_ATSB);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    SC_AppData.EnableHeaderUpdate = true;

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* Set to return -1 in order to generate error message SC_ATS_DIST_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_TransmitMsg), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.NumCmdsWakeup == 1, "SC_OperData.NumCmdsWakeup == 1");
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_FAILED_DISTRIB);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrSeq, SC_AtsId_ATSB);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrCmd, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_DIST_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_ABT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ProcessAtpCmd_Test_ChecksumFailedAtsA(void)
{
    SC_AtsEntryHeader_t *         Entry;
    CFE_SB_MsgId_t                TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t             FcnCode   = SC_SWITCH_ATS_CC;
    bool                          ChecksumValid;
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_AppData.NextCmdTime[SC_Process_ATP] = 0;
    SC_AppData.CurrentTime                 = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(SC_AtsId_ATSA);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    SC_OperData.HkPacket.Payload.ContinueAtsOnFailureFlag = false;

    SC_AppData.EnableHeaderUpdate = false;

    /* Set to return false in order to generate error message SC_ATS_CHKSUM_ERR_EID */
    ChecksumValid = false;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_InlineSwitch), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1");
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrSeq, SC_AtsId_ATSA);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrCmd, 1);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_FAILED_CHECKSUM);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_CHKSUM_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_ABT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ProcessAtpCmd_Test_ChecksumFailedAtsB(void)
{
    SC_AtsEntryHeader_t *         Entry;
    CFE_SB_MsgId_t                TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t             FcnCode   = SC_SWITCH_ATS_CC;
    bool                          ChecksumValid;
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(1);
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_AppData.NextCmdTime[SC_Process_ATP] = 0;
    SC_AppData.CurrentTime                 = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(SC_AtsId_ATSB);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    SC_OperData.HkPacket.Payload.ContinueAtsOnFailureFlag = false;

    SC_AppData.EnableHeaderUpdate = false;

    /* Set to return false in order to generate error message SC_ATS_CHKSUM_ERR_EID */
    ChecksumValid = false;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_InlineSwitch), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1");
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrSeq, SC_AtsId_ATSB);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrCmd, 1);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_FAILED_CHECKSUM);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_CHKSUM_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_ABT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ProcessAtpCmd_Test_ChecksumFailedAtsAContinue(void)
{
    SC_AtsEntryHeader_t *         Entry;
    CFE_SB_MsgId_t                TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t             FcnCode   = SC_SWITCH_ATS_CC;
    bool                          ChecksumValid;
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_AppData.NextCmdTime[SC_Process_ATP] = 0;
    SC_AppData.CurrentTime                 = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(SC_AtsId_ATSA);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    SC_OperData.HkPacket.Payload.ContinueAtsOnFailureFlag = SC_AtsCont_TRUE;

    SC_AppData.EnableHeaderUpdate = false;

    /* Set to return false in order to generate error message SC_ATS_CHKSUM_ERR_EID */
    ChecksumValid = false;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_InlineSwitch), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1");
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrSeq, SC_AtsId_ATSA);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrCmd, 1);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_FAILED_CHECKSUM);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_CHKSUM_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessAtpCmd_Test_CmdNumberMismatchAtsA(void)
{
    SC_AtsEntryHeader_t *         Entry;
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));

    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(3);

    SC_AppData.NextCmdTime[SC_Process_ATP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState   = SC_Status_EXECUTING;
    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(SC_AtsId_ATSA);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1");
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrSeq, SC_AtsId_ATSA);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrCmd, 1);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_SKIPPED);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_MSMTCH_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_ABT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ProcessAtpCmd_Test_CmdNumberMismatchAtsB(void)
{
    SC_AtsEntryHeader_t *         Entry;
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(1);
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(3);

    SC_AppData.NextCmdTime[SC_Process_ATP] = 0;
    SC_AppData.CurrentTime                 = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(SC_AtsId_ATSB);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1");
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrSeq, SC_AtsId_ATSB);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrCmd, 1);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_SKIPPED);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_MSMTCH_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_ABT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ProcessAtpCmd_Test_CmdNotLoaded(void)
{
    SC_AtsEntryHeader_t *         Entry;
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec     = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_AppData.NextCmdTime[SC_Process_ATP] = 0;
    SC_AppData.CurrentTime                 = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(SC_AtsId_ATSA);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    CmdOffsetRec->Offset = SC_ENTRY_OFFSET_FIRST;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 1");
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrSeq, SC_AtsId_ATSA);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrCmd, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_SKP_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessAtpCmd_Test_CompareAbsTime(void)
{
    SC_AtsEntryHeader_t *         Entry;
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_AppData.NextCmdTime[SC_Process_ATP] = 0;
    SC_AppData.CurrentTime                 = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(SC_AtsId_ATSA);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    CmdOffsetRec->Offset = SC_ENTRY_OFFSET_FIRST;

    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAtpCmd_Test_AtpState(void)
{
    SC_AtsEntryHeader_t *         Entry;
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec     = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_AppData.NextCmdTime[SC_Process_ATP] = 0;
    SC_AppData.CurrentTime                 = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_Status_EMPTY;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(SC_AtsId_ATSA);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    CmdOffsetRec->Offset = SC_ENTRY_OFFSET_FIRST;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAtpCmd_Test_CmdMid(void)
{
    SC_AtsEntryHeader_t *         Entry;
    CFE_SB_MsgId_t                TestMsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t             FcnCode   = SC_SWITCH_ATS_CC;
    SC_AtsIndex_t                 AtsIndex  = SC_ATS_IDX_C(0);
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_AtsIndexToNum(AtsIndex);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    SC_AppData.EnableHeaderUpdate = true;

    /* Set return value for CFE_TIME_Compare to make SC_CompareAbsTime return false, to satisfy first if-statement of
     * SC_ProcessAtpCmd, and for all other calls to CFE_TIME_Compare called from subfunctions reached by this test */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), Ut_CFE_TIME_CompareHookAlessthanB, NULL);

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_InlineSwitch), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 1, "SC_OperData.HkPacket.Payload.AtsCmdCtr == 1");
    UtAssert_True(SC_OperData.NumCmdsWakeup == 1, "SC_OperData.NumCmdsWakeup == 1");
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_EXECUTED);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRtpCommand_Test_Nominal(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
    RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;

    SC_AppData.EnableHeaderUpdate = true;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdCtr == 1, "SC_OperData.HkPacket.Payload.RtsCmdCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 0, "SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 0");
    UtAssert_True(SC_OperData.NumCmdsWakeup == 1, "SC_OperData.NumCmdsWakeup == 1");
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdCtr, 1);
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdErrCtr, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRtpCommand_Test_BadSoftwareBusReturn(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
    RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;

    SC_AppData.EnableHeaderUpdate = true;

    /* Set to return -1 in order to generate error message SC_RTS_DIST_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_TransmitMsg), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.RtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.NumCmdsWakeup == 1, "SC_OperData.NumCmdsWakeup == 1");
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdCtr, 0);
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdErrCtr, 1);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastRtsErrSeq, 1);
    SC_Assert_IDX_VALUE(SC_OperData.HkPacket.Payload.LastRtsErrCmd, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_DIST_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessRtpCommand_Test_BadChecksum(void)
{
    bool               ChecksumValid;
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
    RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;

    SC_AppData.EnableHeaderUpdate = false;

    /* Set to return false in order to generate error message SC_RTS_CHKSUM_ERR_EID */
    ChecksumValid = false;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdCtr == 0, "SC_OperData.HkPacket.Payload.RtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.NumCmdsWakeup == 1, "SC_OperData.NumCmdsWakeup == 1");
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdCtr, 0);
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdErrCtr, 1);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastRtsErrSeq, 1);
    SC_Assert_IDX_VALUE(SC_OperData.HkPacket.Payload.LastRtsErrCmd, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_CHKSUM_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessRtpCommand_Test_NextCmdTime(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 1;
    SC_AppData.CurrentTime                  = 0;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
    RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRtpCommand_Test_RtsNumZero(void)
{
    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RTS_NUM_C(0);

    /* CurrRtsNum > 0 will be false so nothing should happen, branch coverage */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRtpCommand_Test_RtsNumHigh(void)
{
    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS + 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRtpCommand_Test_RtsStatus(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
    RtsInfoPtr->RtsStatus                   = SC_Status_EMPTY;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_SendHkPacket_Test(void)
{
    uint8              i;
    int32              LastRtsHkIndex = 0;
    SC_RtsIndex_t      RtsIndex       = SC_RTS_IDX_C(SC_NUMBER_OF_RTS - 1);
    SC_RtsInfoEntry_t *RtsInfoPtr;
    SC_AtsInfoTable_t *AtsInfoPtr;

    SC_OperData.HkPacket.Payload.CmdErrCtr                = 1;
    SC_OperData.HkPacket.Payload.CmdCtr                   = 2;
    SC_OperData.HkPacket.Payload.RtsActiveErrCtr          = 3;
    SC_OperData.HkPacket.Payload.RtsActiveCtr             = 4;
    SC_OperData.HkPacket.Payload.AtsCmdCtr                = 5;
    SC_OperData.HkPacket.Payload.AtsCmdErrCtr             = 6;
    SC_OperData.HkPacket.Payload.RtsCmdCtr                = 7;
    SC_OperData.HkPacket.Payload.RtsCmdErrCtr             = 8;
    SC_OperData.HkPacket.Payload.LastAtsErrSeq            = SC_ATS_NUM_C(9);
    SC_OperData.HkPacket.Payload.LastAtsErrCmd            = SC_COMMAND_NUM_C(10);
    SC_OperData.HkPacket.Payload.LastRtsErrSeq            = SC_RTS_NUM_C(11);
    SC_OperData.HkPacket.Payload.LastRtsErrCmd            = SC_ENTRY_OFFSET_C(12);
    SC_OperData.HkPacket.Payload.AppendCmdArg             = SC_ATS_NUM_C(13);
    SC_OperData.HkPacket.Payload.AppendEntryCount         = 14;
    SC_AppData.AppendWordCount                            = 15;
    SC_OperData.HkPacket.Payload.AppendLoadCount          = 16;
    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum               = SC_ATS_NUM_C(17);
    SC_OperData.AtsCtrlBlckAddr->AtpState                 = (SC_Status_Enum_t)18;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber                = SC_COMMAND_NUM_C(19);
    SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag           = 0;
    SC_AppData.NextCmdTime[SC_Process_ATP]                = 0;
    SC_AppData.NextCmdTime[SC_Process_RTP]                = 0;
    SC_OperData.RtsCtrlBlckAddr->NumRtsActive             = 20;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum               = SC_RTS_NUM_C(21);
    SC_OperData.HkPacket.Payload.ContinueAtsOnFailureFlag = SC_AtsCont_TRUE;

    for (i = 0; i < SC_NUMBER_OF_ATS - 1; i++)
    {
        AtsInfoPtr = SC_GetAtsInfoObject(SC_ATS_IDX_C(i));

        AtsInfoPtr->AtsSize = 0;
    }

    for (i = 0; i < SC_NUMBER_OF_RTS - 1; i++)
    {
        RtsInfoPtr = SC_GetRtsInfoObject(SC_RTS_IDX_C(i));

        RtsInfoPtr->DisabledFlag = true;
        RtsInfoPtr->RtsStatus    = SC_Status_EXECUTING;
    }

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    RtsInfoPtr->DisabledFlag = false;
    RtsInfoPtr->RtsStatus    = SC_Status_EMPTY;

    LastRtsHkIndex = sizeof(SC_OperData.HkPacket.Payload.RtsExecutingStatus) /
                         sizeof(SC_OperData.HkPacket.Payload.RtsExecutingStatus[0]) -
                     1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_SendHkPacket());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 2, "SC_OperData.HkPacket.Payload.CmdCtr == 2");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveErrCtr == 3,
                  "SC_OperData.HkPacket.Payload.RtsActiveErrCtr == 3");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveCtr == 4, "SC_OperData.HkPacket.Payload.RtsActiveCtr == 4");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 5, "SCSC_OperData.HkPacket.Payload.AtsCmdCtr == 5");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 6, "SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 6");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdCtr == 7, "SC_OperData.HkPacket.Payload.RtsCmdCtr == 7");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 8, "SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 8");
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrSeq, 9);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastAtsErrCmd, 10);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.LastRtsErrSeq, 11);
    SC_Assert_IDX_VALUE(SC_OperData.HkPacket.Payload.LastRtsErrCmd, 12);
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.AppendCmdArg, 13);
    UtAssert_True(SC_OperData.HkPacket.Payload.AppendEntryCount == 14,
                  "SC_OperData.HkPacket.Payload.AppendEntryCount == 14");
    UtAssert_True(SC_OperData.HkPacket.Payload.AppendLoadCount == 16,
                  "SC_OperData.HkPacket.Payload.AppendLoadCount == 16");

    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.AtpFreeBytes[i], SC_ATS_BUFF_SIZE32 * SC_BYTES_IN_WORD);
    }

    UtAssert_UINT8_EQ(SC_OperData.HkPacket.Payload.CurrAtsId, 17);
    SC_Assert_CmdStatus(SC_OperData.HkPacket.Payload.AtpState, 18);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.AtpCmdNumber, 19);
    UtAssert_True(SC_OperData.HkPacket.Payload.SwitchPendFlag == 0, "SC_OperData.HkPacket.Payload.SwitchPendFlag == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.NextAtsTime == 0, "SC_OperData.HkPacket.Payload.NextAtsTime == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.NumRtsActive == 20, "SC_OperData.HkPacket.Payload.NumRtsActive == 20");
    SC_Assert_ID_VALUE(SC_OperData.HkPacket.Payload.RtsNum, 21);
    UtAssert_True(SC_OperData.HkPacket.Payload.NextRtsWakeupCnt == 0, "SC_OperData.HkPacket.Payload.NextRtsWakeupCnt == 0");
    UtAssert_BOOL_TRUE(SC_OperData.HkPacket.Payload.ContinueAtsOnFailureFlag);

    /* Check first element */
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsExecutingStatus[0] == 65535,
                  "SC_OperData.HkPacket.Payload.RtsExecutingStatus[0] == 65535");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsDisabledStatus[0] == 65535,
                  "SC_OperData.HkPacket.Payload.RtsDisabledStatus[0] == 65535");

    /* Check middle element */
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsExecutingStatus[2] == 65535,
                  "SC_OperData.HkPacket.Payload.RtsExecutingStatus[2] == 65535");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsDisabledStatus[2] == 65535,
                  "SC_OperData.HkPacket.Payload.RtsDisabledStatus[2] == 65535");

    /* Check last element */
    UtAssert_INT32_EQ(SC_OperData.HkPacket.Payload.RtsExecutingStatus[LastRtsHkIndex], 32767);
    UtAssert_INT32_EQ(SC_OperData.HkPacket.Payload.RtsDisabledStatus[LastRtsHkIndex], 32767);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_HkMID(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_SendHkCmd(&UT_CmdBuf.SendHkCmd));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_HkMIDAutoStartRts(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.AutoStartRTS = SC_RtsIndexToNum(RtsIndex);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_SendHkCmd(&UT_CmdBuf.SendHkCmd));

    /* Verify results */
    SC_Assert_ID_VALUE(SC_AppData.AutoStartRTS, 0);
    UtAssert_BOOL_FALSE(RtsInfoPtr->DisabledFlag);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_HkMIDAutoStartRtsLoaded(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.AutoStartRTS = SC_RtsIndexToNum(RtsIndex);

    RtsInfoPtr->RtsStatus = SC_Status_LOADED;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_SendHkCmd(&UT_CmdBuf.SendHkCmd));

    /* Verify results */
    SC_Assert_ID_VALUE(SC_AppData.AutoStartRTS, 0);
    UtAssert_BOOL_FALSE(RtsInfoPtr->DisabledFlag);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_WakeupNONE(void)
{
    SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = true;
    SC_AppData.NextCmdTime[SC_Process_ATP]      = 0;
    SC_AppData.CurrentTime                      = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_WakeupCmd(&UT_CmdBuf.WakeupCmd));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.NumCmdsWakeup, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_WakeupRtpNotExecutionTime(void)
{
    SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = false;
    SC_AppData.NextCmdTime[SC_Process_RTP]      = 2;
    SC_AppData.CurrentWakeupCount               = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_WakeupCmd(&UT_CmdBuf.WakeupCmd));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.NumCmdsWakeup, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_WakeupNoSwitchPending(void)
{
    SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = false;
    SC_AppData.NextCmdTime[SC_Process_ATP]      = 0;
    SC_AppData.CurrentTime                      = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_WakeupCmd(&UT_CmdBuf.WakeupCmd));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.NumCmdsWakeup, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_WakeupAtpNotExecutionTime(void)
{
    SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = true;
    SC_AppData.NextCmdTime[SC_Process_ATP]      = 1000;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_WakeupCmd(&UT_CmdBuf.WakeupCmd));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.NumCmdsWakeup, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_WakeupRtpExecutionTime(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentWakeupCount           = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
    RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;

    SC_AppData.EnableHeaderUpdate = true;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_WakeupCmd(&UT_CmdBuf.WakeupCmd));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_WakeupAtpExecutionTime(void)
{
    SC_AtsEntryHeader_t *         Entry;
    CFE_SB_MsgId_t                TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t             FcnCode   = SC_NOOP_CC;
    SC_AtsIndex_t                 AtsIndex  = SC_ATS_IDX_C(0);
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    SC_AppData.NextCmdTime[SC_Process_ATP] = 0;
    SC_AppData.CurrentTime                 = 0;
    SC_OperData.NumCmdsWakeup              = 0;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_AtsIndexToNum(AtsIndex);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(1);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    SC_AppData.EnableHeaderUpdate = true;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_WakeupCmd(&UT_CmdBuf.WakeupCmd));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.NumCmdsWakeup, 0);
}

void SC_ProcessRequest_Test_WakeupRtpExecutionTimeTooManyCmds(void)
{
    SC_AppData.EnableHeaderUpdate = true;

    SC_AppData.NextCmdTime[SC_Process_RTP] = 0;
    SC_AppData.NextCmdTime[SC_Process_ATP] = 0;
    SC_OperData.NumCmdsWakeup                 = 1000;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_Status_EXECUTING;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_WakeupCmd(&UT_CmdBuf.WakeupCmd));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.NumCmdsWakeup, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_WakeupAtpExecutionTimeTooManyCmds(void)
{
    SC_AppData.NextCmdTime[SC_Process_ATP] = 10;
    SC_AppData.CurrentTime                 = 100;
    SC_OperData.NumCmdsWakeup                 = 1000;
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_Status_IDLE;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_WakeupCmd(&UT_CmdBuf.WakeupCmd));

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.NumCmdsWakeup, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_NoOp(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_NoopCmd(&UT_CmdBuf.NoopCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 0, "SC_OperData.HkPacket.Payload.CmdErrCtr == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_NOOP_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_ResetCounters(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ResetCountersCmd(&UT_CmdBuf.ResetCountersCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 0, "CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 0, "CmdErrCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdCtr == 0, "AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.AtsCmdErrCtr == 0, "AtsCmdErrCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdCtr == 0, "RtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 0, "RtsCmdErrCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveCtr == 0, "RtsActiveCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveErrCtr == 0, "RtsActiveErrCtr == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RESET_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_StartAts(void)
{
    UT_CmdBuf.StartAtsCmd.Payload.AtsNum = SC_ATS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartAtsCmd(&UT_CmdBuf.StartAtsCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_StopAts(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopAtsCmd(&UT_CmdBuf.StopAtsCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_StartRts(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_StopRts(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsCmd(&UT_CmdBuf.StopRtsCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_DisableRts(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsCmd(&UT_CmdBuf.DisableRtsCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_EnableRts(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsCmd(&UT_CmdBuf.EnableRtsCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_SwitchAts(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_SwitchAtsCmd(&UT_CmdBuf.SwitchAtsCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_JumpAts(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_JumpAtsCmd(&UT_CmdBuf.JumpAtsCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_ContinueAtsOnFailure(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ContinueAtsOnFailureCmd(&UT_CmdBuf.ContinueAtsOnFailureCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_AppendAts(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AppendAtsCmd(&UT_CmdBuf.AppendAtsCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAtsTableNominal(void)
{
    SC_AtsEntryHeader_t *Entry;
    SC_AtsIndex_t        AtsIndex = SC_ATS_IDX_C(0);

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(0);

    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_ATS_0;

    /* Set to reach "SC_LoadAts(ArrayIndex)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAtsTableGetAddressError(void)
{
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_ATS_0;

    /* Set to generate error message SC_TABLE_MANAGE_ATS_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_TABLE_MANAGE_ATS_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_TableManageAtsTableID(void)
{
    SC_AtsEntryHeader_t *Entry;
    SC_AtsIndex_t        AtsIndex = SC_ATS_IDX_C(0);

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(0);

    /* test TableID >= SC_TBL_ID_ATS_0 */
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = 0;

    /* Set to reach "SC_LoadAts(ArrayIndex)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAtsTable_InvalidIndex(void)
{
    uint8 AtsIndex = SC_NUMBER_OF_ATS;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageAtsTable(AtsIndex));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_TABLE_MANAGE_ATS_INV_INDEX_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_TableManageAtsTableGetAddressNeverLoaded(void)
{
    SC_AtsEntryHeader_t *Entry;
    SC_AtsIndex_t        AtsIndex = SC_ATS_IDX_C(0);

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(0);

    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_ATS_0;

    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_ERR_NEVER_LOADED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAtsTableGetAddressSuccess(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/
    SC_AtsEntryHeader_t *Entry;
    SC_AtsIndex_t        AtsIndex = SC_ATS_IDX_C(0);

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(0);

    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_ATS_0;

    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAppendTableNominal(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    SC_AtsEntryHeader_t *Entry;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr;
    Entry->CmdNumber = SC_COMMAND_NUM_C(0);

    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_APPEND;

    /* Set to reach "SC_UpdateAppend()" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAppendTableGetAddressError(void)
{
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_APPEND;

    /* Set to generate error message SC_TABLE_MANAGE_APPEND_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_TABLE_MANAGE_APPEND_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_TableManageAppendTableGetAddressNeverLoaded(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    SC_AtsEntryHeader_t *Entry;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr;
    Entry->CmdNumber = SC_COMMAND_NUM_C(0);

    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_APPEND;

    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_ERR_NEVER_LOADED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAppendTableGetAddressSuccess(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    SC_AtsEntryHeader_t *Entry;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr;
    Entry->CmdNumber = SC_COMMAND_NUM_C(0);

    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_APPEND;

    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageRtsTableNominal(void)
{
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_RTS_0;

    /* Set to reach "SC_LoadRts(ArrayIndex)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageRtsTableGetAddressError(void)
{
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_RTS_0;

    /* Set to generate error message SC_TABLE_MANAGE_RTS_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_TABLE_MANAGE_RTS_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_TableManageRtsTableID(void)
{
    /* test TableID >= SC_TBL_ID_RTS_0 */
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = 0;

    /* Set to reach "SC_LoadRts(ArrayIndex)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageRtsTable_InvalidIndex(void)
{
    uint8 RtsIndex = SC_NUMBER_OF_RTS;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageRtsTable(RtsIndex));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_TABLE_MANAGE_RTS_INV_INDEX_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_TableManageRtsTableGetAddressNeverLoaded(void)
{
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_RTS_0;

    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_ERR_NEVER_LOADED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageRtsTableGetAddressSuccess(void)
{
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_RTS_0;

    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageRtsInfo(void)
{
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_RTS_INFO;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageRtpCtrl(void)
{
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_RTP_CTRL;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAtsInfo(void)
{
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_ATS_INFO;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAtpCtrl(void)
{
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_ATP_CTRL;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAtsCmdStatus(void)
{
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = SC_TBL_ID_ATS_CMD_0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageInvalidTableID(void)
{
    UT_CmdBuf.ManageTableCmd.Payload.Parameter = 999;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ManageTableCmd(&UT_CmdBuf.ManageTableCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_TABLE_MANAGE_ID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_StartRtsGrp(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_StopRtsGrp(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_DisableRtsGrp(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_EnableRtsGrp(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

/* Unreachable branches in sc_cmds.c SC_ProcessAtpCmd:236, 274, 310.
   There are only 2 ATS IDs defined, invalid IDs are already handled. */

void UtTest_Setup(void)
{
    UtTest_Add(SC_ProcessAtpCmd_Test_SwitchCmd, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessAtpCmd_Test_SwitchCmd");
    UtTest_Add(SC_ProcessAtpCmd_Test_NonSwitchCmd, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAtpCmd_Test_NonSwitchCmd");
    UtTest_Add(SC_ProcessAtpCmd_Test_InlineSwitchError, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAtpCmd_Test_InlineSwitchError");
    UtTest_Add(SC_ProcessAtpCmd_Test_SBErrorAtsA, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessAtpCmd_Test_SBErrorAtsA");
    UtTest_Add(SC_ProcessAtpCmd_Test_SBErrorAtsB, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessAtpCmd_Test_SBErrorAtsB");
    UtTest_Add(SC_ProcessAtpCmd_Test_ChecksumFailedAtsA, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAtpCmd_Test_ChecksumFailedAtsA");
    UtTest_Add(SC_ProcessAtpCmd_Test_ChecksumFailedAtsB, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAtpCmd_Test_ChecksumFailedAtsB");
    UtTest_Add(SC_ProcessAtpCmd_Test_ChecksumFailedAtsAContinue, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAtpCmd_Test_ChecksumFailedAtsAContinue");
    UtTest_Add(SC_ProcessAtpCmd_Test_CmdNumberMismatchAtsA, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAtpCmd_Test_CmdNumberMismatchAtsA");
    UtTest_Add(SC_ProcessAtpCmd_Test_CmdNumberMismatchAtsB, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAtpCmd_Test_CmdNumberMismatchAtsB");
    UtTest_Add(SC_ProcessAtpCmd_Test_CmdNotLoaded, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAtpCmd_Test_CmdNotLoaded");
    UtTest_Add(SC_ProcessAtpCmd_Test_CompareAbsTime, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAtpCmd_Test_CompareAbsTime");
    UtTest_Add(SC_ProcessAtpCmd_Test_AtpState, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessAtpCmd_Test_AtpState");
    UtTest_Add(SC_ProcessAtpCmd_Test_CmdMid, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessAtpCmd_Test_CmdMid");
    UtTest_Add(SC_ProcessRtpCommand_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessRtpCommand_Test_Nominal");
    UtTest_Add(SC_ProcessRtpCommand_Test_BadSoftwareBusReturn, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_BadSoftwareBusReturn");
    UtTest_Add(SC_ProcessRtpCommand_Test_BadChecksum, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_BadChecksum");
    UtTest_Add(SC_ProcessRtpCommand_Test_NextCmdTime, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_NextCmdTime");
    UtTest_Add(SC_ProcessRtpCommand_Test_RtsNumZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_RtsNumZero");
    UtTest_Add(SC_ProcessRtpCommand_Test_RtsNumHigh, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_RtsNumHigh");
    UtTest_Add(SC_ProcessRtpCommand_Test_RtsStatus, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_RtsStatus");
    UtTest_Add(SC_SendHkPacket_Test, SC_Test_Setup, SC_Test_TearDown, "SC_SendHkPacket_Test");
    UtTest_Add(SC_ProcessRequest_Test_HkMID, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessRequest_Test_HkMID");
    UtTest_Add(SC_ProcessRequest_Test_HkMIDAutoStartRts, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_HkMIDAutoStartRts");
    UtTest_Add(SC_ProcessRequest_Test_HkMIDAutoStartRtsLoaded, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_HkMIDAutoStartRtsLoaded");
    UtTest_Add(SC_ProcessRequest_Test_WakeupNONE, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupNONE");
    UtTest_Add(SC_ProcessRequest_Test_WakeupRtpNotExecutionTime, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupRtpNotExecutionTime");
    UtTest_Add(SC_ProcessRequest_Test_WakeupNoSwitchPending, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupNoSwitchPending");
    UtTest_Add(SC_ProcessRequest_Test_WakeupAtpNotExecutionTime, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupAtpNotExecutionTime");
    UtTest_Add(SC_ProcessRequest_Test_WakeupRtpExecutionTime, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupRtpExecutionTime");
    UtTest_Add(SC_ProcessRequest_Test_WakeupAtpExecutionTime, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupAtpExecutionTime");
    UtTest_Add(SC_ProcessRequest_Test_WakeupRtpExecutionTimeTooManyCmds, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupRtpExecutionTimeTooManyCmds");
    UtTest_Add(SC_ProcessRequest_Test_WakeupAtpExecutionTimeTooManyCmds, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_WakeupAtpExecutionTimeTooManyCmds");
    UtTest_Add(SC_ProcessCommand_Test_NoOp, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessCommand_Test_NoOp");
    UtTest_Add(SC_ProcessCommand_Test_ResetCounters, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_ResetCounters");
    UtTest_Add(SC_ProcessCommand_Test_StartAts, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessCommand_Test_StartAts");
    UtTest_Add(SC_ProcessCommand_Test_StopAts, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessCommand_Test_StopAts");
    UtTest_Add(SC_ProcessCommand_Test_StartRts, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessCommand_Test_StartRts");
    UtTest_Add(SC_ProcessCommand_Test_StopRts, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessCommand_Test_StopRts");
    UtTest_Add(SC_ProcessCommand_Test_DisableRts, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessCommand_Test_DisableRts");
    UtTest_Add(SC_ProcessCommand_Test_EnableRts, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessCommand_Test_EnableRts");
    UtTest_Add(SC_ProcessCommand_Test_SwitchAts, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessCommand_Test_SwitchAts");
    UtTest_Add(SC_ProcessCommand_Test_JumpAts, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessCommand_Test_JumpAts");
    UtTest_Add(SC_ProcessCommand_Test_ContinueAtsOnFailure, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_ContinueAtsOnFailure");
    UtTest_Add(SC_ProcessCommand_Test_AppendAts, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessCommand_Test_AppendAts");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAtsTableNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAtsTableNominal");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAtsTableGetAddressError, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAtsTableGetAddressError");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAtsTableID, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAtsTableID");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAtsTable_InvalidIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAtsTable_InvalidIndex");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAtsTableGetAddressNeverLoaded, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAtsTableGetAddressNeverLoaded");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAtsTableGetAddressSuccess, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAtsTableGetAddressSuccess");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAppendTableNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAppendTableNominal");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAppendTableGetAddressError, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAppendTableGetAddressError");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAppendTableGetAddressNeverLoaded, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAppendTableGetAddressNeverLoaded");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAppendTableGetAddressSuccess, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAppendTableGetAddressSuccess");
    UtTest_Add(SC_ProcessCommand_Test_TableManageRtsTableNominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageRtsTableNominal");
    UtTest_Add(SC_ProcessCommand_Test_TableManageRtsTableGetAddressError, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageRtsTableGetAddressError");
    UtTest_Add(SC_ProcessCommand_Test_TableManageRtsTableID, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageRtsTableID");
    UtTest_Add(SC_ProcessCommand_Test_TableManageRtsTable_InvalidIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageRtsTable_InvalidIndex");
    UtTest_Add(SC_ProcessCommand_Test_TableManageRtsTableGetAddressNeverLoaded, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageRtsTableGetAddressNeverLoaded");
    UtTest_Add(SC_ProcessCommand_Test_TableManageRtsTableGetAddressSuccess, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageRtsTableGetAddressSuccess");
    UtTest_Add(SC_ProcessCommand_Test_TableManageRtsInfo, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageRtsInfo");
    UtTest_Add(SC_ProcessCommand_Test_TableManageRtpCtrl, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageRtpCtrl");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAtsInfo, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAtsInfo");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAtpCtrl, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAtpCtrl");
    UtTest_Add(SC_ProcessCommand_Test_TableManageAtsCmdStatus, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageAtsCmdStatus");
    UtTest_Add(SC_ProcessCommand_Test_TableManageInvalidTableID, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageInvalidTableID");
    UtTest_Add(SC_ProcessCommand_Test_StartRtsGrp, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StartRtsGrp");
    UtTest_Add(SC_ProcessCommand_Test_StopRtsGrp, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessCommand_Test_StopRtsGrp");
    UtTest_Add(SC_ProcessCommand_Test_DisableRtsGrp, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_DisableRtsGrp");
    UtTest_Add(SC_ProcessCommand_Test_EnableRtsGrp, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_EnableRtsGrp");
}
