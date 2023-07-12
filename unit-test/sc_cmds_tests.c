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

CFE_TIME_Compare_t Ut_CFE_TIME_CompareHookAlessthanB(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                                     const UT_StubContext_t *Context)
{
    return CFE_TIME_A_LT_B;
}

uint8 SC_CMDS_TEST_SC_UpdateNextTimeHook_RunCount;
int32 Ut_SC_UpdateNextTimeHook(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    if (SC_CMDS_TEST_SC_UpdateNextTimeHook_RunCount++)
        SC_AppData.NextProcNumber = SC_NONE;

    return 0;
}

void SC_ProcessAtpCmd_Test_SwitchCmd(void)
{
    SC_AtsEntryHeader_t *Entry;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_SWITCH_ATS_CC;
    bool                 ChecksumValid;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 1;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[0][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[0][0]    = 0;

    /* Set return value for CFE_TIME_Compare to make SC_CompareAbsTime return false, to satisfy first if-statement of
     * SC_ProcessAtpCmd, and for all other calls to CFE_TIME_Compare called from subfunctions reached by this test */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), Ut_CFE_TIME_CompareHookAlessthanB, NULL);

    /* Set to return true in order to satisfy the if-statement from which the function is called */
    ChecksumValid = true;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_InlineSwitch), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 1, "SC_OperData.HkPacket.AtsCmdCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 0, "SC_OperData.HkPacket.AtsCmdErrCtr == 0");
    UtAssert_True(SC_OperData.NumCmdsSec == 1, "SC_OperData.NumCmdsSec == 1");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_EXECUTED,
                  "SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_EXECUTED");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAtpCmd_Test_NonSwitchCmd(void)
{
    SC_AtsEntryHeader_t *Entry;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_NOOP_CC;
    bool                 ChecksumValid;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 1;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[0][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[0][0]    = 0;

    /* Set return value for CFE_TIME_Compare to make SC_CompareAbsTime return false, to satisfy first if-statement of
     * SC_ProcessAtpCmd, and for all other calls to CFE_TIME_Compare called from subfunctions reached by this test */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), Ut_CFE_TIME_CompareHookAlessthanB, NULL);

    /* Set to return true in order to satisfy the if-statement from which the function is called */
    ChecksumValid = true;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_InlineSwitch), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 1, "SC_OperData.HkPacket.AtsCmdCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 0, "SC_OperData.HkPacket.AtsCmdErrCtr == 0");
    UtAssert_True(SC_OperData.NumCmdsSec == 1, "SC_OperData.NumCmdsSec == 1");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_EXECUTED,
                  "SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_EXECUTED");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAtpCmd_Test_InlineSwitchError(void)
{
    SC_AtsEntryHeader_t *Entry;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_SWITCH_ATS_CC;
    bool                 ChecksumValid;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 1;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[0][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[0][0]    = 0;

    /* Set return value for CFE_TIME_Compare to make SC_CompareAbsTime return false, to satisfy first if-statement of
     * SC_ProcessAtpCmd, and for all other calls to CFE_TIME_Compare called from subfunctions reached by this test */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), Ut_CFE_TIME_CompareHookAlessthanB, NULL);

    /* Set to return true in order to satisfy the if-statement from which the function is called */
    ChecksumValid = true;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_InlineSwitch), 1, false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "SC_OperData.HkPacket.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.AtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.NumCmdsSec == 1, "SC_OperData.NumCmdsSec == 1");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_FAILED_DISTRIB,
                  "SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_FAILED_DISTRIB");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrSeq == 1, "SC_OperData.HkPacket.LastAtsErrSeq == 1");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrCmd == 1, "SC_OperData.HkPacket.LastAtsErrCmd == 1");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAtpCmd_Test_SBErrorAtsA(void)
{
    SC_AtsEntryHeader_t *Entry;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_NOOP_CC;
    bool                 ChecksumValid;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSA;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[0][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[0][0]    = 0;

    /* Set to return true in order to satisfy the if-statement from which the function is called */
    ChecksumValid = true;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* Set to return -1 in order to generate error message SC_ATS_DIST_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_TransmitMsg), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "SC_OperData.HkPacket.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.AtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.NumCmdsSec == 1, "SC_OperData.NumCmdsSec == 1");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_FAILED_DISTRIB,
                  "SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_FAILED_DISTRIB");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSA, "SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSA");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrCmd == 1, "SC_OperData.HkPacket.LastAtsErrCmd == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_DIST_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_ABT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ProcessAtpCmd_Test_SBErrorAtsB(void)
{
    SC_AtsEntryHeader_t *Entry;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_NOOP_CC;
    bool                 ChecksumValid;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[1][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSB;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[1][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[1][0]    = 0;

    /* Set to return true in order to satisfy the if-statement from which the function is called */
    ChecksumValid = true;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* Set to return -1 in order to generate error message SC_ATS_DIST_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_TransmitMsg), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "SC_OperData.HkPacket.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.AtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.NumCmdsSec == 1, "SC_OperData.NumCmdsSec == 1");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[1][0] == SC_FAILED_DISTRIB,
                  "SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_FAILED_DISTRIB");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSB, "SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSB");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrCmd == 1, "SC_OperData.HkPacket.LastAtsErrCmd == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_DIST_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_ABT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ProcessAtpCmd_Test_ChecksumFailedAtsA(void)
{
    SC_AtsEntryHeader_t *Entry;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_SWITCH_ATS_CC;
    bool                 ChecksumValid;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSA;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[0][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[0][0]    = 0;

    SC_OperData.HkPacket.ContinueAtsOnFailureFlag = false;

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
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "SC_OperData.HkPacket.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.AtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSA, "SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSA");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrCmd == 1, "SC_OperData.HkPacket.LastAtsErrCmd == 1");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_FAILED_CHECKSUM,
                  "SC_OperData.AtsCmdStatusTblAddr[1][0] == SC_FAILED_CHECKSUM");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_CHKSUM_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_ABT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ProcessAtpCmd_Test_ChecksumFailedAtsB(void)
{
    SC_AtsEntryHeader_t *Entry;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_SWITCH_ATS_CC;
    bool                 ChecksumValid;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[1][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSB;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[1][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[1][0]    = 0;

    SC_OperData.HkPacket.ContinueAtsOnFailureFlag = false;

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
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "SC_OperData.HkPacket.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.AtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSB, "SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSB");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrCmd == 1, "SC_OperData.HkPacket.LastAtsErrCmd == 1");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[1][0] == SC_FAILED_CHECKSUM,
                  "SC_OperData.AtsCmdStatusTblAddr[1][0] == SC_FAILED_CHECKSUM");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_CHKSUM_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_ABT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ProcessAtpCmd_Test_ChecksumFailedAtsAContinue(void)
{
    SC_AtsEntryHeader_t *Entry;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_SWITCH_ATS_CC;
    bool                 ChecksumValid;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSA;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[0][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[0][0]    = 0;

    SC_OperData.HkPacket.ContinueAtsOnFailureFlag = true;

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
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "SC_OperData.HkPacket.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.AtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSA, "SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSA");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrCmd == 1, "SC_OperData.HkPacket.LastAtsErrCmd == 1");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_FAILED_CHECKSUM,
                  "SC_OperData.AtsCmdStatusTblAddr[1][0] == SC_FAILED_CHECKSUM");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_CHKSUM_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessAtpCmd_Test_CmdNumberMismatchAtsA(void)
{
    SC_AtsEntryHeader_t *Entry;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 3;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSA;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[0][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[0][0]    = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "SC_OperData.HkPacket.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.AtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSA, "SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSA");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrCmd == 1, "SC_OperData.HkPacket.LastAtsErrCmd == 1");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_SKIPPED,
                  "SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_SKIPPED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_MSMTCH_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_ABT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ProcessAtpCmd_Test_CmdNumberMismatchAtsB(void)
{
    SC_AtsEntryHeader_t *Entry;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[1][0];
    Entry->CmdNumber = 3;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSB;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[1][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[1][0]    = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "SC_OperData.HkPacket.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.AtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSB, "SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSB");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrCmd == 1, "SC_OperData.HkPacket.LastAtsErrCmd == 1");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[1][0] == SC_SKIPPED,
                  "SC_OperData.AtsCmdStatusTblAddr[1][0] == SC_SKIPPED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_MSMTCH_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_ATS_ABT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_ProcessAtpCmd_Test_CmdNotLoaded(void)
{
    SC_AtsEntryHeader_t *Entry;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSA;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_AppData.AtsCmdIndexBuffer[0][0] = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "SC_OperData.HkPacket.AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 1, "SC_OperData.HkPacket.AtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSA, "SC_OperData.HkPacket.LastAtsErrSeq == SC_ATSA");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrCmd == 1, "SC_OperData.HkPacket.LastAtsErrCmd == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_SKP_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessAtpCmd_Test_CompareAbsTime(void)
{
    SC_AtsEntryHeader_t *Entry;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSA;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_AppData.AtsCmdIndexBuffer[0][0] = 0;

    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "SC_OperData.HkPacket.AtsCmdCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAtpCmd_Test_NextProcNumber(void)
{
    SC_AtsEntryHeader_t *Entry;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_NONE;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSA;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_AppData.AtsCmdIndexBuffer[0][0] = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "SC_OperData.HkPacket.AtsCmdCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAtpCmd_Test_AtpState(void)
{
    SC_AtsEntryHeader_t *Entry;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EMPTY;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_ATSA;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_AppData.AtsCmdIndexBuffer[0][0] = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "SC_OperData.HkPacket.AtsCmdCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAtpCmd_Test_CmdMid(void)
{
    SC_AtsEntryHeader_t *Entry;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t    FcnCode   = SC_SWITCH_ATS_CC;
    bool                 ChecksumValid;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 1;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[0][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[0][0]    = 0;

    /* Set return value for CFE_TIME_Compare to make SC_CompareAbsTime return false, to satisfy first if-statement of
     * SC_ProcessAtpCmd, and for all other calls to CFE_TIME_Compare called from subfunctions reached by this test */
    UT_SetHookFunction(UT_KEY(CFE_TIME_Compare), Ut_CFE_TIME_CompareHookAlessthanB, NULL);

    /* Set to return true in order to satisfy the if-statement from which the function is called */
    ChecksumValid = true;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Set these two functions to return these values in order to statisfy the if-statement from which they are both
     * called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_InlineSwitch), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAtpCmd());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 1, "SC_OperData.HkPacket.AtsCmdCtr == 1");
    UtAssert_True(SC_OperData.NumCmdsSec == 1, "SC_OperData.NumCmdsSec == 1");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_EXECUTED,
                  "SC_OperData.AtsCmdStatusTblAddr[0][0] == SC_EXECUTED");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRtpCommand_Test_Nominal(void)
{
    bool ChecksumValid;

    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;

    /* Set to return true in order to satisfy the if-statement from which the function is called */
    ChecksumValid = true;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.RtsCmdCtr == 1, "SC_OperData.HkPacket.RtsCmdCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.RtsCmdErrCtr == 0, "SC_OperData.HkPacket.RtsCmdErrCtr == 0");
    UtAssert_True(SC_OperData.NumCmdsSec == 1, "SC_OperData.NumCmdsSec == 1");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[0].CmdCtr == 1, "SC_OperData.RtsInfoTblAddr[0].CmdCtr == 1");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[0].CmdErrCtr == 0, "SC_OperData.RtsInfoTblAddr[0].CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRtpCommand_Test_BadSoftwareBusReturn(void)
{
    bool ChecksumValid;

    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;

    /* Set to return true in order to satisfy the if-statement from which the function is called */
    ChecksumValid = true;

    /* Set to return -1 in order to generate error message SC_RTS_DIST_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_TransmitMsg), 1, -1);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.RtsCmdCtr == 0, "SC_OperData.HkPacket.RtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.RtsCmdErrCtr == 1, "SC_OperData.HkPacket.RtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.NumCmdsSec == 1, "SC_OperData.NumCmdsSec == 1");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[0].CmdCtr == 0, "SC_OperData.RtsInfoTblAddr[0].CmdCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[0].CmdErrCtr == 1, "SC_OperData.RtsInfoTblAddr[0].CmdErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.LastRtsErrSeq == 1, "SC_OperData.HkPacket.LastRtsErrSeq == 1");
    UtAssert_True(SC_OperData.HkPacket.LastRtsErrCmd == 0, "SC_OperData.HkPacket.LastRtsErrCmd == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_DIST_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessRtpCommand_Test_BadChecksum(void)
{
    bool ChecksumValid;

    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;

    /* Set to return false in order to generate error message SC_RTS_CHKSUM_ERR_EID */
    ChecksumValid = false;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.RtsCmdCtr == 0, "SC_OperData.HkPacket.RtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.RtsCmdErrCtr == 1, "SC_OperData.HkPacket.RtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.NumCmdsSec == 1, "SC_OperData.NumCmdsSec == 1");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[0].CmdCtr == 0, "SC_OperData.RtsInfoTblAddr[0].CmdCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[0].CmdErrCtr == 1, "SC_OperData.RtsInfoTblAddr[0].CmdErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.LastRtsErrSeq == 1, "SC_OperData.HkPacket.LastRtsErrSeq == 1");
    UtAssert_True(SC_OperData.HkPacket.LastRtsErrCmd == 0, "SC_OperData.HkPacket.LastRtsErrCmd == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_CHKSUM_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessRtpCommand_Test_NextCmdTime(void)
{
    SC_AppData.NextCmdTime[SC_RTP]                                                   = 1;
    SC_AppData.CurrentTime                                                           = 0;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRtpCommand_Test_ProcNumber(void)
{
    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_NONE;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRtpCommand_Test_RtsNumberZero(void)
{
    SC_AppData.NextCmdTime[SC_RTP]         = 0;
    SC_AppData.CurrentTime                 = 1;
    SC_AppData.NextProcNumber              = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber = 0;

    /* RtsNumber > 0 will be false so nothing should happen, branch coverage */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRtpCommand_Test_RtsNumberHigh(void)
{
    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = SC_NUMBER_OF_RTS + 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRtpCommand_Test_RtsStatus(void)
{
    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EMPTY;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRtpCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_SendHkPacket_Test(void)
{
    uint8 i;
    int32 LastRtsHkIndex = 0;

    SC_OperData.HkPacket.CmdErrCtr                = 1;
    SC_OperData.HkPacket.CmdCtr                   = 2;
    SC_OperData.HkPacket.RtsActiveErrCtr          = 3;
    SC_OperData.HkPacket.RtsActiveCtr             = 4;
    SC_OperData.HkPacket.AtsCmdCtr                = 5;
    SC_OperData.HkPacket.AtsCmdErrCtr             = 6;
    SC_OperData.HkPacket.RtsCmdCtr                = 7;
    SC_OperData.HkPacket.RtsCmdErrCtr             = 8;
    SC_OperData.HkPacket.LastAtsErrSeq            = 9;
    SC_OperData.HkPacket.LastAtsErrCmd            = 10;
    SC_OperData.HkPacket.LastRtsErrSeq            = 11;
    SC_OperData.HkPacket.LastRtsErrCmd            = 12;
    SC_OperData.HkPacket.AppendCmdArg             = 13;
    SC_OperData.HkPacket.AppendEntryCount         = 14;
    SC_AppData.AppendWordCount                    = 15;
    SC_OperData.HkPacket.AppendLoadCount          = 16;
    SC_OperData.AtsInfoTblAddr[0].AtsSize         = 0;
    SC_OperData.AtsInfoTblAddr[1].AtsSize         = 0;
    SC_OperData.AtsCtrlBlckAddr->AtsNumber        = 17;
    SC_OperData.AtsCtrlBlckAddr->AtpState         = 18;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber        = 19;
    SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag   = 0;
    SC_AppData.NextCmdTime[0]                     = 0;
    SC_AppData.NextCmdTime[1]                     = 0;
    SC_OperData.RtsCtrlBlckAddr->NumRtsActive     = 20;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber        = 21;
    SC_OperData.HkPacket.ContinueAtsOnFailureFlag = 1;

    for (i = 0; i < SC_NUMBER_OF_RTS - 1; i++)
    {
        SC_OperData.RtsInfoTblAddr[i].DisabledFlag = true;
        SC_OperData.RtsInfoTblAddr[i].RtsStatus    = SC_EXECUTING;
    }

    SC_OperData.RtsInfoTblAddr[SC_NUMBER_OF_RTS - 1].DisabledFlag = 0;
    SC_OperData.RtsInfoTblAddr[SC_NUMBER_OF_RTS - 1].RtsStatus    = 0;

    LastRtsHkIndex =
        sizeof(SC_OperData.HkPacket.RtsExecutingStatus) / sizeof(SC_OperData.HkPacket.RtsExecutingStatus[0]) - 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_SendHkPacket());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 2, "SC_OperData.HkPacket.CmdCtr == 2");
    UtAssert_True(SC_OperData.HkPacket.RtsActiveErrCtr == 3, "SC_OperData.HkPacket.RtsActiveErrCtr == 3");
    UtAssert_True(SC_OperData.HkPacket.RtsActiveCtr == 4, "SC_OperData.HkPacket.RtsActiveCtr == 4");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 5, "SCSC_OperData.HkPacket.AtsCmdCtr == 5");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 6, "SC_OperData.HkPacket.AtsCmdErrCtr == 6");
    UtAssert_True(SC_OperData.HkPacket.RtsCmdCtr == 7, "SC_OperData.HkPacket.RtsCmdCtr == 7");
    UtAssert_True(SC_OperData.HkPacket.RtsCmdErrCtr == 8, "SC_OperData.HkPacket.RtsCmdErrCtr == 8");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrSeq == 9, "SC_OperData.HkPacket.LastAtsErrSeq == 9");
    UtAssert_True(SC_OperData.HkPacket.LastAtsErrCmd == 10, "SC_OperData.HkPacket.LastAtsErrCmd == 10");
    UtAssert_True(SC_OperData.HkPacket.LastRtsErrSeq == 11, "SC_OperData.HkPacket.LastRtsErrSeq == 11");
    UtAssert_True(SC_OperData.HkPacket.LastRtsErrCmd == 12, "SC_OperData.HkPacket.LastRtsErrCmd == 12");
    UtAssert_True(SC_OperData.HkPacket.AppendCmdArg == 13, "SC_OperData.HkPacket.AppendCmdArg == 13");
    UtAssert_True(SC_OperData.HkPacket.AppendEntryCount == 14, "SC_OperData.HkPacket.AppendEntryCount == 14");
    UtAssert_True(SC_OperData.HkPacket.AppendLoadCount == 16, "SC_OperData.HkPacket.AppendLoadCount == 16");
    UtAssert_True(SC_OperData.HkPacket.AtpFreeBytes[0] ==
                      (SC_ATS_BUFF_SIZE32 * SC_BYTES_IN_WORD) -
                          (SC_OperData.AtsInfoTblAddr[0].AtsSize * SC_BYTES_IN_WORD),
                  "SC_OperData.HkPacket.AtpFreeBytes[0] == (SC_ATS_BUFF_SIZE32 * SC_BYTES_IN_WORD)");
    UtAssert_True(SC_OperData.HkPacket.AtpFreeBytes[1] ==
                      (SC_ATS_BUFF_SIZE32 * SC_BYTES_IN_WORD) -
                          (SC_OperData.AtsInfoTblAddr[1].AtsSize * SC_BYTES_IN_WORD),
                  "SC_OperData.HkPacket.AtpFreeBytes[1] == (SC_ATS_BUFF_SIZE32 * SC_BYTES_IN_WORD)");
    UtAssert_True(SC_OperData.HkPacket.AtsNumber == 17, "SC_OperData.HkPacket.AtsNumber == 17");
    UtAssert_True(SC_OperData.HkPacket.AtpState == 18, "SC_OperData.HkPacket.AtpState == 18");
    UtAssert_True(SC_OperData.HkPacket.AtpCmdNumber == 19, "SC_OperData.HkPacket.AtpCmdNumber == 19");
    UtAssert_True(SC_OperData.HkPacket.SwitchPendFlag == 0, "SC_OperData.HkPacket.SwitchPendFlag == 0");
    UtAssert_True(SC_OperData.HkPacket.NextAtsTime == 0, "SC_OperData.HkPacket.NextAtsTime == 0");
    UtAssert_True(SC_OperData.HkPacket.NumRtsActive == 20, "SC_OperData.HkPacket.NumRtsActive == 20");
    UtAssert_True(SC_OperData.HkPacket.RtsNumber == 21, "SC_OperData.HkPacket.RtsNumber == 21");
    UtAssert_True(SC_OperData.HkPacket.NextRtsTime == 0, "SC_OperData.HkPacket.NextRtsTime == 0");
    UtAssert_True(SC_OperData.HkPacket.ContinueAtsOnFailureFlag == 1,
                  "SC_OperData.HkPacket.ContinueAtsOnFailureFlag == 1");

    /* Check first element */
    UtAssert_True(SC_OperData.HkPacket.RtsExecutingStatus[0] == 65535,
                  "SC_OperData.HkPacket.RtsExecutingStatus[0] == 65535");
    UtAssert_True(SC_OperData.HkPacket.RtsDisabledStatus[0] == 65535,
                  "SC_OperData.HkPacket.RtsDisabledStatus[0] == 65535");

    /* Check middle element */
    UtAssert_True(SC_OperData.HkPacket.RtsExecutingStatus[2] == 65535,
                  "SC_OperData.HkPacket.RtsExecutingStatus[2] == 65535");
    UtAssert_True(SC_OperData.HkPacket.RtsDisabledStatus[2] == 65535,
                  "SC_OperData.HkPacket.RtsDisabledStatus[2] == 65535");

    /* Check last element */
    UtAssert_INT32_EQ(SC_OperData.HkPacket.RtsExecutingStatus[LastRtsHkIndex], 32767);
    UtAssert_INT32_EQ(SC_OperData.HkPacket.RtsDisabledStatus[LastRtsHkIndex], 32767);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_CmdMID(void)
{
    /**
     **  Test case: SC_CMD_MID
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_NOOP_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessRequest_Test_HkMID(void)
{
    /**
     **  Test case: SC_SEND_HK_MID
     **/

    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_SEND_HK_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_HkMIDNoVerifyCmdLength(void)
{
    /**
     **  Test case: SC_SEND_HK_MID
     **/

    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_SEND_HK_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_HkMIDAutoStartRts(void)
{
    /**
     **  Test case: SC_SEND_HK_MID
     **/

    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_SEND_HK_MID);

    SC_AppData.AutoStartRTS = 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_AppData.AutoStartRTS == 0, "SC_AppData.AutoStartRTS == 0");
    UtAssert_BOOL_FALSE(SC_OperData.RtsInfoTblAddr[SC_AppData.AutoStartRTS].DisabledFlag);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_HkMIDAutoStartRtsLoaded(void)
{
    /**
     **  Test case: SC_SEND_HK_MID
     **/

    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_SEND_HK_MID);

    SC_AppData.AutoStartRTS                                           = 1;
    SC_OperData.RtsInfoTblAddr[SC_AppData.AutoStartRTS - 1].RtsStatus = SC_LOADED;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_AppData.AutoStartRTS == 0, "SC_AppData.AutoStartRTS == 0");
    UtAssert_BOOL_FALSE(SC_OperData.RtsInfoTblAddr[SC_AppData.AutoStartRTS].DisabledFlag);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_1HzWakeupNONE(void)
{
    /**
     **  Test case: SC_1HZ_WAKEUP_MID with SC_AppData.NextProcNumber == SC_NONE
     **/
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_1HZ_WAKEUP_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = true;
    SC_AppData.NextProcNumber                   = SC_NONE;
    SC_AppData.NextCmdTime[SC_ATP]              = 0;
    SC_AppData.CurrentTime                      = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.NumCmdsSec == 0, "SC_OperData.NumCmdsSec == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_1HzWakeupNoSwitchPending(void)
{
    /**
     **  Test case: SC_1HZ_WAKEUP_MID with SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == false
     **/
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_1HZ_WAKEUP_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = false;
    SC_AppData.NextProcNumber                   = SC_NONE;
    SC_AppData.NextCmdTime[SC_ATP]              = 0;
    SC_AppData.CurrentTime                      = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.NumCmdsSec == 0, "SC_OperData.NumCmdsSec == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_1HzWakeupAtpNotExecutionTime(void)
{
    /**
     **  Test case: SC_1HZ_WAKEUP_MID with a pending ATP command that should not execute yet
     **/
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_1HZ_WAKEUP_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = true;
    SC_AppData.NextProcNumber                   = SC_ATP;
    SC_AppData.NextCmdTime[SC_ATP]              = 1000;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.NumCmdsSec == 0, "SC_OperData.NumCmdsSec == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_1HzWakeupRtpExecutionTime(void)
{
    /**
     **  Test case: SC_1HZ_WAKEUP_MID with a pending RTP command that needs to execute immediately
     **/
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_1HZ_WAKEUP_MID);
    SC_AtsEntryHeader_t *Entry;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* required to exit processing loop after 2 iterations */
    /* second iteration tests "IsThereAnotherCommandToExecute" */

    SC_CMDS_TEST_SC_UpdateNextTimeHook_RunCount = 0;
    UT_SetHookFunction(UT_KEY(SC_UpdateNextTime), Ut_SC_UpdateNextTimeHook, NULL);

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextProcNumber             = SC_RTP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING; /* Causes switch to ATP */
    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_OperData.NumCmdsSec                = 3;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 1;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[0][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[0][0]    = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_1HzWakeupRtpExecutionTimeTooManyCmds(void)
{
    bool ChecksumValid;

    /**
     **  Test case: SC_1HZ_WAKEUP_MID with a pending RTP command that needs to execute immediately, but too many
     *commands are being sent at once
     **/
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_1HZ_WAKEUP_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    ChecksumValid = true;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_ValidateChecksum), &ChecksumValid, sizeof(ChecksumValid), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    SC_AppData.NextProcNumber             = SC_RTP;
    SC_AppData.NextCmdTime[SC_RTP]        = 0;
    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_OperData.NumCmdsSec                = 1000;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.NumCmdsSec == 0, "SC_OperData.NumCmdsSec == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessRequest_Test_MIDError(void)
{
    /**
     **  Test case: SC_MID_ERR_EID
     **/

    CFE_SB_MsgId_t TestMsgId = SC_UT_MID_1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessRequest(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_MID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_NoOp(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_NOOP_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_NoOpNoVerifyCmdLength(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_ResetCounters(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_RESET_COUNTERS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "CmdErrCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdCtr == 0, "AtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.AtsCmdErrCtr == 0, "AtsCmdErrCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.RtsCmdCtr == 0, "RtsCmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.RtsCmdErrCtr == 0, "RtsCmdErrCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.RtsActiveCtr == 0, "RtsActiveCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.RtsActiveErrCtr == 0, "RtsActiveErrCtr == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RESET_DEB_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_ResetCountersNoVerifyCmdLength(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_RESET_COUNTERS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_StartAts(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    UT_CmdBuf.StartAtsCmd.AtsId = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_StopAts(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_STOP_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_StartRts(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_RTS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_StopRts(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_STOP_RTS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_DisableRts(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_DISABLE_RTS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_EnableRts(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_ENABLE_RTS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_SwitchAts(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_SWITCH_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_JumpAts(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_JUMP_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_ContinueAtsOnFailure(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_CONTINUE_ATS_ON_FAILURE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_AppendAts(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_APPEND_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAtsTableNominal(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    SC_AtsEntryHeader_t *Entry;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 0;

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_ATS_0;

    /* Set to reach "SC_LoadAts(ArrayIndex)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAtsTableGetAddressError(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_ATS_0;

    /* Set to generate error message SC_TABLE_MANAGE_ATS_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_TABLE_MANAGE_ATS_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_TableManageAtsTableID(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    SC_AtsEntryHeader_t *Entry;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 0;

    /* test TableID >= SC_TBL_ID_ATS_0 */
    UT_CmdBuf.NotifyCmd.Payload.Parameter = 0;

    /* Set to reach "SC_LoadAts(ArrayIndex)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

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
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    SC_AtsEntryHeader_t *Entry;
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 0;

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_ATS_0;

    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_ERR_NEVER_LOADED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

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
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 0;

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_ATS_0;

    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

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
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr;
    Entry->CmdNumber = 0;

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_APPEND;

    /* Set to reach "SC_UpdateAppend()" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAppendTableGetAddressError(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_APPEND;

    /* Set to generate error message SC_TABLE_MANAGE_APPEND_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

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
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr;
    Entry->CmdNumber = 0;

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_APPEND;

    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_ERR_NEVER_LOADED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

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
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t    FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr;
    Entry->CmdNumber = 0;

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_APPEND;

    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageRtsTableNominal(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_RTS_0;

    /* Set to reach "SC_LoadRts(ArrayIndex)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageRtsTableGetAddressError(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_RTS_0;

    /* Set to generate error message SC_TABLE_MANAGE_RTS_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_TABLE_MANAGE_RTS_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_TableManageRtsTableID(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* test TableID >= SC_TBL_ID_RTS_0 */
    UT_CmdBuf.NotifyCmd.Payload.Parameter = 0;

    /* Set to reach "SC_LoadRts(ArrayIndex)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

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
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_RTS_0;

    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_ERR_NEVER_LOADED);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageRtsTableGetAddressSuccess(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_RTS_0;

    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageRtsInfo(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_RTS_INFO;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageRtpCtrl(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_RTP_CTRL;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAtsInfo(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_ATS_INFO;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAtpCtrl(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_ATP_CTRL;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageAtsCmdStatus(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    UT_CmdBuf.NotifyCmd.Payload.Parameter = SC_TBL_ID_ATS_CMD_0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_TableManageInvalidTableID(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    UT_CmdBuf.NotifyCmd.Payload.Parameter = 999;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_TABLE_MANAGE_ID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_ProcessCommand_Test_StartRtsGrp(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_RTS_GRP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_StopRtsGrp(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_STOP_RTS_GRP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_DisableRtsGrp(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_DISABLE_RTS_GRP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}

void SC_ProcessCommand_Test_EnableRtsGrp(void)
{
    /**
     **  Note: This test does not follow the standard test guideline to only test what's directly in the
     *function-under-test.
     **  Since the code for reaching each branch in SC_ProcessCommand is so trivial and non-verifiable, it was decided
     *to
     **  combine the tests for each command with the tests for reaching the command from SC_ProcessCommand.
     **/

    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_ENABLE_RTS_GRP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(SC_VerifyCmdLength), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* This function is already verified to work correctly in another file, so no verifications here. */
}


void SC_ProcessCommand_Test_StartAtsInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_StopAtsInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_STOP_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_StartRtsInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_RTS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_StopRtsInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_STOP_RTS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_DisableRtsInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_DISABLE_RTS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_EnableRtsInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_ENABLE_RTS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_GroundSwitchInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_SWITCH_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_JumpAtsInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_JUMP_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_ContinueAtsOnFailureInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_CONTINUE_ATS_ON_FAILURE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_AppendAtsInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_APPEND_ATS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_TableManageInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_MANAGE_TABLE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_StartRtsGrpInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_START_RTS_GRP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_StopRtsGrpInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_STOP_RTS_GRP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_DisableRtsGrpInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_DISABLE_RTS_GRP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessCommand_Test_EnableRtsGrpInvalidCmdLength(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = SC_ENABLE_RTS_GRP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    SC_ProcessCommand(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
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

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessCommand(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_INVLD_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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
    UtTest_Add(SC_ProcessAtpCmd_Test_NextProcNumber, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAtpCmd_Test_NextProcNumber");
    UtTest_Add(SC_ProcessAtpCmd_Test_AtpState, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessAtpCmd_Test_AtpState");
    UtTest_Add(SC_ProcessAtpCmd_Test_CmdMid, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessAtpCmd_Test_CmdMid");
    UtTest_Add(SC_ProcessRtpCommand_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessRtpCommand_Test_Nominal");
    UtTest_Add(SC_ProcessRtpCommand_Test_BadSoftwareBusReturn, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_BadSoftwareBusReturn");
    UtTest_Add(SC_ProcessRtpCommand_Test_BadChecksum, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_BadChecksum");
    UtTest_Add(SC_ProcessRtpCommand_Test_NextCmdTime, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_NextCmdTime");
    UtTest_Add(SC_ProcessRtpCommand_Test_ProcNumber, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_ProcNumber");
    UtTest_Add(SC_ProcessRtpCommand_Test_RtsNumberZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_RtsNumberZero");
    UtTest_Add(SC_ProcessRtpCommand_Test_RtsNumberHigh, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_RtsNumberHigh");
    UtTest_Add(SC_ProcessRtpCommand_Test_RtsStatus, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRtpCommand_Test_RtsStatus");
    UtTest_Add(SC_SendHkPacket_Test, SC_Test_Setup, SC_Test_TearDown, "SC_SendHkPacket_Test");
    UtTest_Add(SC_ProcessRequest_Test_CmdMID, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessRequest_Test_CmdMID");
    UtTest_Add(SC_ProcessRequest_Test_HkMID, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessRequest_Test_HkMID");
    UtTest_Add(SC_ProcessRequest_Test_HkMIDNoVerifyCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_HkMIDNoVerifyCmdLength");
    UtTest_Add(SC_ProcessRequest_Test_HkMIDAutoStartRts, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_HkMIDAutoStartRts");
    UtTest_Add(SC_ProcessRequest_Test_HkMIDAutoStartRtsLoaded, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_HkMIDAutoStartRtsLoaded");
    UtTest_Add(SC_ProcessRequest_Test_1HzWakeupNONE, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_1HzWakeupNONE");
    UtTest_Add(SC_ProcessRequest_Test_1HzWakeupNoSwitchPending, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_1HzWakeupNoSwitchPending");
    UtTest_Add(SC_ProcessRequest_Test_1HzWakeupAtpNotExecutionTime, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_1HzWakeupAtpNotExecutionTime");
    UtTest_Add(SC_ProcessRequest_Test_1HzWakeupRtpExecutionTime, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_1HzWakeupRtpExecutionTime");
    UtTest_Add(SC_ProcessRequest_Test_1HzWakeupRtpExecutionTimeTooManyCmds, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessRequest_Test_1HzWakeupRtpExecutionTimeTooManyCmds");
    UtTest_Add(SC_ProcessRequest_Test_MIDError, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessRequest_Test_MIDError");
    UtTest_Add(SC_ProcessCommand_Test_NoOp, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessCommand_Test_NoOp");
    UtTest_Add(SC_ProcessCommand_Test_NoOpNoVerifyCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_NoOpNoVerifyCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_ResetCounters, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_ResetCounters");
    UtTest_Add(SC_ProcessCommand_Test_ResetCountersNoVerifyCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_ResetCountersNoVerifyCmdLength");
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
    UtTest_Add(SC_ProcessCommand_Test_StartAtsInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StartAtsInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_StopAtsInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StopAtsInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_StartRtsInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StartRtsInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_StopRtsInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StopRtsInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_DisableRtsInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_DisableRtsInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_EnableRtsInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_EnableRtsInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_GroundSwitchInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_GroundSwitchInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_JumpAtsInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_JumpAtsInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_ContinueAtsOnFailureInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_ContinueAtsOnFailureInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_AppendAtsInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_AppendAtsInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_TableManageInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_TableManageInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_StartRtsGrpInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StartRtsGrpInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_StopRtsGrpInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_StopRtsGrpInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_DisableRtsGrpInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_DisableRtsGrpInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_EnableRtsGrpInvalidCmdLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_EnableRtsGrpInvalidCmdLength");
    UtTest_Add(SC_ProcessCommand_Test_InvalidCmdError, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessCommand_Test_InvalidCmdError");
}
