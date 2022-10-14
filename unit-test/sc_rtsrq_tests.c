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

/* sc_rtsrq_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

/*
 * Function Definitions
 */

void SC_StartRtsCmd_Test_Nominal(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    SC_RtpControlBlock_t RtsCtrlBlck;
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "RTS Number %%03d Started");

    SC_InitTables();

    memset(&RtsCtrlBlck, 0, sizeof(RtsCtrlBlck));

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    SC_OperData.RtsCtrlBlckAddr      = &RtsCtrlBlck;

    Entry          = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag = 0;

    UT_CmdBuf.RtsCmd.RtsId = 1;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus    = SC_LOADED;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    MsgSize = sizeof(SC_RtsCmd_t);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_EXECUTING,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_EXECUTING");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr == 0, "SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr == 0,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr == 0,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr == 1, "SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr == 1");

    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 1, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 1");
    UtAssert_True(SC_OperData.HkPacket.RtsActiveCtr == 1, "SC_OperData.HkPacket.RtsActiveCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_START_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsCmd_Test_StartRtsNoEvents(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    SC_RtpControlBlock_t RtsCtrlBlck;
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Start RTS #%%d command");

    SC_InitTables();

    UT_CmdBuf.RtsCmd.RtsId = SC_NUMBER_OF_RTS;

    RtsIndex = UT_CmdBuf.RtsCmd.RtsId - 1;

    memset(&RtsCtrlBlck, 0, sizeof(RtsCtrlBlck));
    memset(&RtsTable, 0, sizeof(RtsTable));

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    SC_OperData.RtsCtrlBlckAddr      = &RtsCtrlBlck;

    Entry          = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag = 0;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus    = SC_LOADED;
    SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr       = 0;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    MsgSize = sizeof(SC_RtsCmd_t);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_EXECUTING,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_EXECUTING");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr == 0, "SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr == 0,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr == 0,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr == 1, "SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr == 1");

    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 1, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 1");
    UtAssert_True(SC_OperData.HkPacket.RtsActiveCtr == 1, "SC_OperData.HkPacket.RtsActiveCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_DBG_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsCmd_Test_InvalidCommandLength1(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    SC_RtpControlBlock_t RtsCtrlBlck;
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Start RTS %%03d Rejected: Invld Len Field for 1st Cmd in Sequence. Invld Cmd Length = %%d");

    SC_InitTables();

    memset(&RtsCtrlBlck, 0, sizeof(RtsCtrlBlck));

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    SC_OperData.RtsCtrlBlckAddr      = &RtsCtrlBlck;

    Entry          = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag = 0;

    UT_CmdBuf.RtsCmd.RtsId = 1;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus    = SC_LOADED;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    MsgSize = 0;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_INVLD_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsCmd_Test_InvalidCommandLength2(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    SC_RtpControlBlock_t RtsCtrlBlck;
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Start RTS %%03d Rejected: Invld Len Field for 1st Cmd in Sequence. Invld Cmd Length = %%d");

    SC_InitTables();

    memset(&RtsCtrlBlck, 0, sizeof(RtsCtrlBlck));

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    SC_OperData.RtsCtrlBlckAddr      = &RtsCtrlBlck;

    Entry          = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag = 0;

    UT_CmdBuf.RtsCmd.RtsId = 1;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus    = SC_LOADED;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    MsgSize = SC_PACKET_MAX_SIZE + 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_INVLD_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsCmd_Test_RtsNotLoadedOrInUse(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    SC_RtpControlBlock_t RtsCtrlBlck;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Start RTS %%03d Rejected: RTS Not Loaded or In Use, Status: %%d");

    SC_InitTables();

    memset(&RtsCtrlBlck, 0, sizeof(RtsCtrlBlck));

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    SC_OperData.RtsCtrlBlckAddr      = &RtsCtrlBlck;

    Entry          = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag = 0;

    UT_CmdBuf.RtsCmd.RtsId = 1;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus    = SC_IDLE;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_NOT_LDED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsCmd_Test_RtsDisabled(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    SC_RtpControlBlock_t RtsCtrlBlck;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Start RTS %%03d Rejected: RTS Disabled");

    SC_InitTables();

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    SC_OperData.RtsCtrlBlckAddr      = &RtsCtrlBlck;

    Entry          = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag = 0;

    UT_CmdBuf.RtsCmd.RtsId = 1;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = true;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus    = SC_LOADED;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_DISABLED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsCmd_Test_InvalidRtsId(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Start RTS %%03d Rejected: Invalid RTS ID");

    SC_InitTables();

    UT_CmdBuf.RtsCmd.RtsId = SC_NUMBER_OF_RTS * 2;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_INVALID_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsCmd_Test_InvalidRtsIdZero(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Start RTS %%03d Rejected: Invalid RTS ID");

    SC_InitTables();

    UT_CmdBuf.RtsCmd.RtsId = 0;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_INVALID_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsCmd_Test_NoVerifyLength(void)
{
    /* Execute the function being tested */
    SC_StartRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.RtsActiveErrCtr == 1, "SC_OperData.HkPacket.RtsActiveErrCtr == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsGrpCmd_Test_Nominal(void)
{
    uint8                RtsIndex = 0;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    SC_RtpControlBlock_t RtsCtrlBlck;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Start RTS group: FirstID=%%d, LastID=%%d, Modified=%%d");

    SC_InitTables();

    memset(&RtsCtrlBlck, 0, sizeof(RtsCtrlBlck));
    memset(&RtsTable, 0, sizeof(RtsTable));

    SC_OperData.RtsTblAddr[RtsIndex]               = &RtsTable[0];
    SC_OperData.RtsCtrlBlckAddr                    = &RtsCtrlBlck;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus = SC_LOADED;
    SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr    = 0;

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_EXECUTING,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_EXECUTING");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr == 0, "SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr == 0,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr == 0,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr == 1, "SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr == 1");

    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 1, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 1");
    UtAssert_True(SC_OperData.HkPacket.RtsActiveCtr == 1, "SC_OperData.HkPacket.RtsActiveCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsGrpCmd_Test_StartRtsGroupError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Start RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = SC_NUMBER_OF_RTS * 2;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = SC_NUMBER_OF_RTS * 2;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsGrpCmd_Test_NoVerifyLength(void)
{
    /* Execute the function being tested */
    SC_StartRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsGrpCmd_Test_FirstRtsIndex(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Start RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = SC_NUMBER_OF_RTS + 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsGrpCmd_Test_FirstRtsIndexZero(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Start RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 0;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsGrpCmd_Test_LastRtsIndex(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Start RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = SC_NUMBER_OF_RTS + 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsGrpCmd_Test_LastRtsIndexZero(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Start RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 0;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsGrpCmd_Test_FirstLastRtsIndex(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Start RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 2;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsGrpCmd_Test_DisabledFlag(void)
{
    uint8                RtsIndex = 0;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    SC_RtpControlBlock_t RtsCtrlBlck;
    int32                strCmpResult;
    char                 ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Start RTS group error: rejected RTS ID %%03d, RTS Disabled");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Start RTS group: FirstID=%%d, LastID=%%d, Modified=%%d");

    SC_InitTables();

    memset(&RtsCtrlBlck, 0, sizeof(RtsCtrlBlck));
    memset(&RtsTable, 0, sizeof(RtsTable));

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    SC_OperData.RtsCtrlBlckAddr      = &RtsCtrlBlck;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag   = true;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus      = SC_EXECUTING;
    SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr         = 0;
    SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr         = 0;
    SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr = 0;

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr == 0, "SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr == 0,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr == 0,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr == 0, "SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr == 0");

    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0");
    UtAssert_True(SC_OperData.HkPacket.RtsActiveCtr == 0, "SC_OperData.HkPacket.RtsActiveCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.RtsActiveErrCtr == 1, "SC_OperData.HkPacket.RtsActiveErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_DISABLED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_STARTRTSGRP_CMD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StartRtsGrpCmd_Test_RtsStatus(void)
{
    uint8                RtsIndex = 0;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    SC_RtpControlBlock_t RtsCtrlBlck;
    int32                strCmpResult;
    char                 ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Start RTS group error: rejected RTS ID %%03d, RTS Not Loaded or In Use, Status: %%d");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Start RTS group: FirstID=%%d, LastID=%%d, Modified=%%d");

    SC_InitTables();

    memset(&RtsCtrlBlck, 0, sizeof(RtsCtrlBlck));
    memset(&RtsTable, 0, sizeof(RtsTable));

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    SC_OperData.RtsCtrlBlckAddr      = &RtsCtrlBlck;

    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus      = SC_EXECUTING;
    SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr         = 0;
    SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr         = 0;
    SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr = 0;

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StartRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_EXECUTING,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_EXECUTING");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr == 0, "SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr == 0,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr == 0,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr == 0, "SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr == 0");

    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0");
    UtAssert_True(SC_OperData.HkPacket.RtsActiveCtr == 0, "SC_OperData.HkPacket.RtsActiveCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.RtsActiveErrCtr == 1, "SC_OperData.HkPacket.RtsActiveErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_NOT_LDED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_STARTRTSGRP_CMD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StopRtsCmd_Test_Nominal(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "RTS %%03d Aborted");

    SC_InitTables();

    UT_CmdBuf.RtsCmd.RtsId = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StopRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTS_CMD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StopRtsCmd_Test_InvalidRts(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Stop RTS %%03d rejected: Invalid RTS ID");

    SC_InitTables();

    UT_CmdBuf.RtsCmd.RtsId = SC_NUMBER_OF_RTS * 2;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StopRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTS_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StopRtsCmd_Test_NoVerifyLength(void)
{
    /* Execute the function being tested */
    SC_StopRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StopRtsGrpCmd_Test_Nominal(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Stop RTS group: FirstID=%%d, LastID=%%d, Modified=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StopRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StopRtsGrpCmd_Test_Error(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Stop RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = SC_NUMBER_OF_RTS * 2;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = SC_NUMBER_OF_RTS * 2;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StopRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StopRtsGrpCmd_Test_NoVerifyLength(void)
{
    /* Execute the function being tested */
    SC_StopRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StopRtsGrpCmd_Test_NotExecuting(void)
{
    uint8                RtsIndex = 0;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    SC_RtpControlBlock_t RtsCtrlBlck;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Stop RTS group: FirstID=%%d, LastID=%%d, Modified=%%d");

    memset(&RtsTable, 0, sizeof(RtsTable));
    memset(&RtsCtrlBlck, 0, sizeof(RtsCtrlBlck));

    SC_InitTables();

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    SC_OperData.RtsCtrlBlckAddr      = &RtsCtrlBlck;

    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus = SC_EXECUTING;

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StopRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StopRtsGrpCmd_Test_FirstRtsIndex(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Stop RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = SC_NUMBER_OF_RTS + 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StopRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StopRtsGrpCmd_Test_FirstRtsIndexZero(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Stop RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 0;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StopRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StopRtsGrpCmd_Test_LastRtsIndex(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Stop RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = SC_NUMBER_OF_RTS + 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StopRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StopRtsGrpCmd_Test_LastRtsIndexZero(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Stop RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 0;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StopRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_StopRtsGrpCmd_Test_FirstLastRtsIndex(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Stop RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 2;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_StopRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_DisableRtsCmd_Test_Nominal(void)
{
    uint8 RtsIndex = 0;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Disabled RTS %%03d");

    SC_InitTables();

    UT_CmdBuf.RtsCmd.RtsId = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_DisableRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISABLE_RTS_DEB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_DisableRtsCmd_Test_InvalidRtsID(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Disable RTS %%03d Rejected: Invalid RTS ID");

    SC_InitTables();

    UT_CmdBuf.RtsCmd.RtsId = SC_NUMBER_OF_RTS * 2;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_DisableRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTS_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_DisableRtsCmd_Test_NoVerifyLength(void)
{
    /* Execute the function being tested */
    SC_DisableRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_DisableRtsGrpCmd_Test_Nominal(void)
{
    uint8 RtsIndex = 0; /* RtsId - 1 */
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Disable RTS group: FirstID=%%d, LastID=%%d, Modified=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_DisableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_DisableRtsGrpCmd_Test_Error(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Disable RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = SC_NUMBER_OF_RTS * 2;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = SC_NUMBER_OF_RTS * 2;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_DisableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_DisableRtsGrpCmd_Test_NoVerifyLength(void)
{
    /* Execute the function being tested */
    SC_DisableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_DisableRtsGrpCmd_Test_FirstRtsIndex(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Disable RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = SC_NUMBER_OF_RTS + 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_DisableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_DisableRtsGrpCmd_Test_FirstRtsIndexZero(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Disable RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 0;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_DisableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_DisableRtsGrpCmd_Test_LastRtsIndex(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Disable RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = SC_NUMBER_OF_RTS + 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_DisableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_DisableRtsGrpCmd_Test_LastRtsIndexZero(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Disable RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 0;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_DisableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_DisableRtsGrpCmd_Test_FirstLastRtsIndex(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Disable RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 2;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_DisableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_DisableRtsGrpCmd_Test_DisabledFlag(void)
{
    uint8 RtsIndex = 0; /* RtsId - 1 */
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Disable RTS group: FirstID=%%d, LastID=%%d, Modified=%%d");

    SC_InitTables();

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = true;

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_DisableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsCmd_Test_Nominal(void)
{
    uint8 RtsIndex = 0;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Enabled RTS %%03d");

    SC_InitTables();

    UT_CmdBuf.RtsCmd.RtsId = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_EnableRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENABLE_RTS_DEB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsCmd_Test_InvalidRtsID(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Enable RTS %%03d Rejected: Invalid RTS ID");

    SC_InitTables();

    UT_CmdBuf.RtsCmd.RtsId = SC_NUMBER_OF_RTS * 2;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_EnableRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTS_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsCmd_Test_InvalidRtsIDZero(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Enable RTS %%03d Rejected: Invalid RTS ID");

    SC_InitTables();

    UT_CmdBuf.RtsCmd.RtsId = 0;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_EnableRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTS_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsCmd_Test_NoVerifyLength(void)
{
    /* Execute the function being tested */
    SC_EnableRtsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsGrpCmd_Test_Nominal(void)
{
    uint8 RtsIndex = 0; /* RtsId - 1 */
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Enable RTS group: FirstID=%%d, LastID=%%d, Modified=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_EnableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsGrpCmd_Test_Error(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Enable RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = SC_NUMBER_OF_RTS * 2;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = SC_NUMBER_OF_RTS * 2;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_EnableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsGrpCmd_Test_NoVerifyLength(void)
{
    /* Execute the function being tested */
    SC_EnableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsGrpCmd_Test_FirstRtsIndex(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Enable RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = SC_NUMBER_OF_RTS + 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_EnableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsGrpCmd_Test_FirstRtsIndexZero(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Enable RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 0;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_EnableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsGrpCmd_Test_LastRtsIndex(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Enable RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = SC_NUMBER_OF_RTS + 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_EnableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsGrpCmd_Test_LastRtsIndexZero(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Enable RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 0;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_EnableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsGrpCmd_Test_FirstLastRtsIndex(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Enable RTS group error: FirstID=%%d, LastID=%%d");

    SC_InitTables();

    UT_CmdBuf.RtsGrpCmd.FirstRtsId = 2;
    UT_CmdBuf.RtsGrpCmd.LastRtsId  = 1;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_EnableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_EnableRtsGrpCmd_Test_DisabledFlag(void)
{
    uint8 RtsIndex = 0; /* RtsId - 1 */
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Enable RTS group: FirstID=%%d, LastID=%%d, Modified=%%d");

    SC_InitTables();

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
    SC_OperData.RtsInfoTblAddr[1].DisabledFlag        = true;
    UT_CmdBuf.RtsGrpCmd.FirstRtsId                    = 1;
    UT_CmdBuf.RtsGrpCmd.LastRtsId                     = 2;

    /* Set message size so SC_VerifyCmdLength will return true, to satisfy first if-statement */
    UT_SetDeferredRetcode(UT_KEY(SC_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    SC_EnableRtsGrpCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 1, "SC_OperData.HkPacket.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_KillRts_Test(void)
{
    uint8                RtsIndex = 0;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    SC_RtpControlBlock_t RtsCtrlBlck;

    SC_InitTables();

    memset(&RtsCtrlBlck, 0, sizeof(RtsCtrlBlck));

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    SC_OperData.RtsCtrlBlckAddr      = &RtsCtrlBlck;

    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus = SC_EXECUTING;
    SC_OperData.RtsCtrlBlckAddr->NumRtsActive      = 1;

    /* Execute the function being tested */
    SC_KillRts(RtsIndex);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_LOADED,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_LOADED");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime == SC_MAX_TIME,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime == SC_MAX_TIME");
    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_KillRts_Test_NoActiveRts(void)
{
    uint8                RtsIndex = 0;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    SC_RtpControlBlock_t RtsCtrlBlck;

    SC_InitTables();

    memset(&RtsCtrlBlck, 0, sizeof(RtsCtrlBlck));

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    SC_OperData.RtsCtrlBlckAddr      = &RtsCtrlBlck;

    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus = SC_EXECUTING;
    SC_OperData.RtsCtrlBlckAddr->NumRtsActive      = 0;

    /* Execute the function being tested */
    SC_KillRts(RtsIndex);

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_LOADED,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_LOADED");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime == SC_MAX_TIME,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime == SC_MAX_TIME");
    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_KillRts_Test_InvalidIndex(void)
{
    uint8 RtsIndex = SC_NUMBER_OF_RTS;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "RTS kill error: invalid RTS index %%d");

    /* Execute the function being tested */
    SC_KillRts(RtsIndex);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_KILLRTS_INV_INDEX_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_AutoStartRts_Test_Nominal(void)
{
    uint8 RtsId = 1;

    /* Execute the function being tested */
    SC_AutoStartRts(RtsId);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void SC_AutoStartRts_Test_InvalidId(void)
{
    uint8 RtsId = SC_NUMBER_OF_RTS + 1;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "RTS autostart error: invalid RTS ID %%d");

    /* Execute the function being tested */
    SC_AutoStartRts(RtsId);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_AUTOSTART_RTS_INV_ID_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void SC_AutoStartRts_Test_InvalidIdZero(void)
{
    uint8 RtsId = 0;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "RTS autostart error: invalid RTS ID %%d");

    /* Execute the function being tested */
    SC_AutoStartRts(RtsId);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_AUTOSTART_RTS_INV_ID_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void UtTest_Setup(void)
{
    UtTest_Add(SC_StartRtsCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_StartRtsCmd_Test_Nominal");
    UtTest_Add(SC_StartRtsCmd_Test_StartRtsNoEvents, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsCmd_Test_StartRtsNoEvents");
    UtTest_Add(SC_StartRtsCmd_Test_InvalidCommandLength1, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsCmd_Test_InvalidCommandLength1");
    UtTest_Add(SC_StartRtsCmd_Test_InvalidCommandLength2, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsCmd_Test_InvalidCommandLength2");
    UtTest_Add(SC_StartRtsCmd_Test_RtsNotLoadedOrInUse, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsCmd_Test_RtsNotLoadedOrInUse");
    UtTest_Add(SC_StartRtsCmd_Test_RtsDisabled, SC_Test_Setup, SC_Test_TearDown, "SC_StartRtsCmd_Test_RtsDisabled");
    UtTest_Add(SC_StartRtsCmd_Test_InvalidRtsId, SC_Test_Setup, SC_Test_TearDown, "SC_StartRtsCmd_Test_InvalidRtsId");

    UtTest_Add(SC_StartRtsCmd_Test_InvalidRtsIdZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsCmd_Test_InvalidRtsIdZero");
    UtTest_Add(SC_StartRtsCmd_Test_NoVerifyLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsCmd_Test_NoVerifyLength");
    UtTest_Add(SC_StartRtsGrpCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_StartRtsGrpCmd_Test_Nominal");
    UtTest_Add(SC_StartRtsGrpCmd_Test_StartRtsGroupError, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsGrpCmd_Test_StartRtsGroupError");
    UtTest_Add(SC_StartRtsGrpCmd_Test_NoVerifyLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsGrpCmd_Test_NoVerifyLength");
    UtTest_Add(SC_StartRtsGrpCmd_Test_FirstRtsIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsGrpCmd_Test_FirstRtsIndex");
    UtTest_Add(SC_StartRtsGrpCmd_Test_FirstRtsIndexZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsGrpCmd_Test_FirstRtsIndexZero");
    UtTest_Add(SC_StartRtsGrpCmd_Test_LastRtsIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsGrpCmd_Test_LastRtsIndex");
    UtTest_Add(SC_StartRtsGrpCmd_Test_LastRtsIndexZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsGrpCmd_Test_LastRtsIndexZero");
    UtTest_Add(SC_StartRtsGrpCmd_Test_FirstLastRtsIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsGrpCmd_Test_FirstLastRtsIndex");
    UtTest_Add(SC_StartRtsGrpCmd_Test_DisabledFlag, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsGrpCmd_Test_DisabledFlag");
    UtTest_Add(SC_StartRtsGrpCmd_Test_RtsStatus, SC_Test_Setup, SC_Test_TearDown, "SC_StartRtsGrpCmd_Test_RtsStatus");
    UtTest_Add(SC_StopRtsCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_StopRtsCmd_Test_Nominal");
    UtTest_Add(SC_StopRtsCmd_Test_InvalidRts, SC_Test_Setup, SC_Test_TearDown, "SC_StopRtsCmd_Test_InvalidRts");
    UtTest_Add(SC_StopRtsCmd_Test_NoVerifyLength, SC_Test_Setup, SC_Test_TearDown, "SC_StopRtsCmd_Test_NoVerifyLength");
    UtTest_Add(SC_StopRtsGrpCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_StopRtsGrpCmd_Test_Nominal");
    UtTest_Add(SC_StopRtsGrpCmd_Test_Error, SC_Test_Setup, SC_Test_TearDown, "SC_StopRtsGrpCmd_Test_Error");
    UtTest_Add(SC_StopRtsGrpCmd_Test_NoVerifyLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_StopRtsGrpCmd_Test_NoVerifyLength");
    UtTest_Add(SC_StopRtsGrpCmd_Test_NotExecuting, SC_Test_Setup, SC_Test_TearDown,
               "SC_StopRtsGrpCmd_Test_NotExecuting");
    UtTest_Add(SC_StopRtsGrpCmd_Test_FirstRtsIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_StopRtsGrpCmd_Test_FirstRtsIndex");
    UtTest_Add(SC_StopRtsGrpCmd_Test_FirstRtsIndexZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_StopRtsGrpCmd_Test_FirstRtsIndexZero");
    UtTest_Add(SC_StopRtsGrpCmd_Test_LastRtsIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_StopRtsGrpCmd_Test_LastRtsIndex");
    UtTest_Add(SC_StopRtsGrpCmd_Test_LastRtsIndexZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_StopRtsGrpCmd_Test_LastRtsIndexZero");
    UtTest_Add(SC_StopRtsGrpCmd_Test_FirstLastRtsIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_StopRtsGrpCmd_Test_FirstLastRtsIndex");
    UtTest_Add(SC_DisableRtsCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_DisableRtsCmd_Test_Nominal");
    UtTest_Add(SC_DisableRtsCmd_Test_InvalidRtsID, SC_Test_Setup, SC_Test_TearDown,
               "SC_DisableRtsCmd_Test_InvalidRtsID");
    UtTest_Add(SC_DisableRtsCmd_Test_NoVerifyLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_DisableRtsCmd_Test_NoVerifyLength");
    UtTest_Add(SC_DisableRtsGrpCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_DisableRtsGrpCmd_Test_Nominal");
    UtTest_Add(SC_DisableRtsGrpCmd_Test_Error, SC_Test_Setup, SC_Test_TearDown, "SC_DisableRtsGrpCmd_Test_Error");
    UtTest_Add(SC_DisableRtsGrpCmd_Test_NoVerifyLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_DisableRtsGrpCmd_Test_NoVerifyLength");
    UtTest_Add(SC_DisableRtsGrpCmd_Test_FirstRtsIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_DisableRtsGrpCmd_Test_FirstRtsIndex");
    UtTest_Add(SC_DisableRtsGrpCmd_Test_FirstRtsIndexZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_DisableRtsGrpCmd_Test_FirstRtsIndexZero");
    UtTest_Add(SC_DisableRtsGrpCmd_Test_LastRtsIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_DisableRtsGrpCmd_Test_LastRtsIndex");
    UtTest_Add(SC_DisableRtsGrpCmd_Test_LastRtsIndexZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_DisableRtsGrpCmd_Test_LastRtsIndexZero");
    UtTest_Add(SC_DisableRtsGrpCmd_Test_FirstLastRtsIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_DisableRtsGrpCmd_Test_FirstLastRtsIndex");
    UtTest_Add(SC_DisableRtsGrpCmd_Test_DisabledFlag, SC_Test_Setup, SC_Test_TearDown,
               "SC_DisableRtsGrpCmd_Test_DisabledFlag");
    UtTest_Add(SC_EnableRtsCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_EnableRtsCmd_Test_Nominal");
    UtTest_Add(SC_EnableRtsCmd_Test_InvalidRtsID, SC_Test_Setup, SC_Test_TearDown, "SC_EnableRtsCmd_Test_InvalidRtsID");
    UtTest_Add(SC_EnableRtsCmd_Test_InvalidRtsIDZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_EnableRtsCmd_Test_InvalidRtsIDZero");
    UtTest_Add(SC_EnableRtsCmd_Test_NoVerifyLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_EnableRtsCmd_Test_NoVerifyLength");
    UtTest_Add(SC_EnableRtsGrpCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_EnableRtsGrpCmd_Test_Nominal");
    UtTest_Add(SC_EnableRtsGrpCmd_Test_Error, SC_Test_Setup, SC_Test_TearDown, "SC_EnableRtsGrpCmd_Test_Error");
    UtTest_Add(SC_EnableRtsGrpCmd_Test_NoVerifyLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_EnableRtsGrpCmd_Test_NoVerifyLength");
    UtTest_Add(SC_EnableRtsGrpCmd_Test_FirstRtsIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_EnableRtsGrpCmd_Test_FirstRtsIndex");
    UtTest_Add(SC_EnableRtsGrpCmd_Test_FirstRtsIndexZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_EnableRtsGrpCmd_Test_FirstRtsIndexZero");
    UtTest_Add(SC_EnableRtsGrpCmd_Test_LastRtsIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_EnableRtsGrpCmd_Test_LastRtsIndex");
    UtTest_Add(SC_EnableRtsGrpCmd_Test_LastRtsIndexZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_EnableRtsGrpCmd_Test_LastRtsIndexZero");
    UtTest_Add(SC_EnableRtsGrpCmd_Test_FirstLastRtsIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_EnableRtsGrpCmd_Test_FirstLastRtsIndex");
    UtTest_Add(SC_EnableRtsGrpCmd_Test_DisabledFlag, SC_Test_Setup, SC_Test_TearDown,
               "SC_EnableRtsGrpCmd_Test_DisabledFlag");
    UtTest_Add(SC_KillRts_Test, SC_Test_Setup, SC_Test_TearDown, "SC_KillRts_Test");
    UtTest_Add(SC_KillRts_Test_NoActiveRts, SC_Test_Setup, SC_Test_TearDown, "SC_KillRts_Test_NoActiveRts");
    UtTest_Add(SC_KillRts_Test_InvalidIndex, SC_Test_Setup, SC_Test_TearDown, "SC_KillRts_Test_InvalidIndex");
    UtTest_Add(SC_AutoStartRts_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_AutoStartRts_Test_Nominal");
    UtTest_Add(SC_AutoStartRts_Test_InvalidId, SC_Test_Setup, SC_Test_TearDown, "SC_AutoStartRts_Test_InvalidId");
    UtTest_Add(SC_AutoStartRts_Test_InvalidIdZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_AutoStartRts_Test_InvalidIdZero");
}
