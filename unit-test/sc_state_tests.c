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
uint8 SC_STATE_TEST_GetTotalMsgLengthHook_RunCount;
int32 SC_STATE_TEST_CFE_SB_GetTotalMsgLengthHook(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                                 const UT_StubContext_t *Context)
{
    SC_STATE_TEST_GetTotalMsgLengthHook_RunCount += 1;

    if (SC_STATE_TEST_GetTotalMsgLengthHook_RunCount == 1)
        return SC_PACKET_MAX_SIZE;
    else
        return SC_PACKET_MAX_SIZE + 100;
}

void SC_GetNextRtsTime_Test_Nominal(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    RtsInfoPtr->RtsStatus       = SC_Status_EXECUTING;
    RtsInfoPtr->NextCommandTgtWakeup = SC_MAX_WAKEUP_CNT;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsTime());

    /* Verify results */
    SC_Assert_ID_VALUE(SC_OperData.RtsCtrlBlckAddr->CurrRtsNum, 1);
    UtAssert_True(SC_AppData.NextCmdTime[SC_Process_RTP] == SC_MAX_WAKEUP_CNT, "SC_AppData.NextCmdTime[SC_Process_RTP] == SC_MAX_WAKEUP_CNT");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsTime_Test_InvalidRtsNum(void)
{
    uint8              i;
    SC_RtsInfoEntry_t *RtsInfoPtr;

    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        RtsInfoPtr = SC_GetRtsInfoObject(SC_RTS_IDX_C(i));

        RtsInfoPtr->RtsStatus = (SC_Status_Enum_t)(-1);
    }

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsTime());

    /* Verify results */
    SC_Assert_ID_EQ(SC_OperData.RtsCtrlBlckAddr->CurrRtsNum, SC_RTS_NUM_NULL);
    UtAssert_True(SC_AppData.NextCmdTime[SC_Process_RTP] == SC_MAX_WAKEUP_CNT,
                  "SC_AppData.NextCmdTime[SC_Process_RTP] == SC_MAX_WAKEUP_CNT");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsTime_Test_RtsPriority(void)
{
    SC_RtsInfoEntry_t *RtsInfoPtr0;
    SC_RtsInfoEntry_t *RtsInfoPtr1;

    RtsInfoPtr0 = SC_GetRtsInfoObject(SC_RTS_IDX_C(0));
    RtsInfoPtr1 = SC_GetRtsInfoObject(SC_RTS_IDX_C(1));

    RtsInfoPtr0->RtsStatus       = SC_Status_EXECUTING;
    RtsInfoPtr0->NextCommandTgtWakeup = SC_MAX_WAKEUP_CNT;

    RtsInfoPtr1->RtsStatus       = SC_Status_EXECUTING;
    RtsInfoPtr1->NextCommandTgtWakeup = SC_MAX_WAKEUP_CNT - 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsTime());

    /* Verify results */
    SC_Assert_ID_VALUE(SC_OperData.RtsCtrlBlckAddr->CurrRtsNum, 2);
    UtAssert_True(SC_AppData.NextCmdTime[SC_Process_RTP] == SC_MAX_WAKEUP_CNT - 1, "SC_AppData.NextCmdTime[SC_Process_RTP] == SC_MAX_WAKEUP_CNT - 1");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsCommand_Test_GetNextCommand(void)
{
    size_t             MsgSize;
    SC_AtsIndex_t      AtsIndex = SC_ATS_IDX_C(1);
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;
    SC_AtsInfoTable_t *AtsInfoPtr;

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);
    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RTS_NUM_C(1);
    RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;
    AtsInfoPtr->NumberOfCommands            = 1;

    /* Give the packet the minimum possible size, to ensure that (CmdOffset < SC_RTS_HDR_WORDS) is met */
    MsgSize = SC_PACKET_MIN_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    SC_Assert_IDX_VALUE(RtsInfoPtr->NextCommandPtr, (SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 1) / SC_BYTES_IN_WORD);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsCommand_Test_RtsNumZero(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsCommand_Test_RtsNumMax(void)
{
    size_t             MsgSize;
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(SC_NUMBER_OF_RTS - 1);
    SC_AtsIndex_t      AtsIndex = SC_ATS_IDX_C(1);
    SC_RtsInfoEntry_t *RtsInfoPtr;
    SC_AtsInfoTable_t *AtsInfoPtr;

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);
    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
    RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;
    AtsInfoPtr->NumberOfCommands            = 1;

    /* Give the packet the minimum possible size, to ensure that (CmdOffset < SC_RTS_HDR_WORDS) is met */
    MsgSize = SC_PACKET_MIN_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsCommand_Test_RtsNumOverMax(void)
{
    size_t             MsgSize;
    SC_AtsIndex_t      AtsIndex = SC_ATS_IDX_C(1);
    SC_AtsInfoTable_t *AtsInfoPtr;

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS + 1);
    AtsInfoPtr->NumberOfCommands            = 1;

    /* Give the packet the minimum possible size, to ensure that (CmdOffset < SC_RTS_HDR_WORDS) is met */
    MsgSize = SC_PACKET_MIN_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsCommand_Test_RtsNotExecuting(void)
{
    size_t             MsgSize;
    SC_AtsIndex_t      AtsIndex = SC_ATS_IDX_C(1);
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(SC_NUMBER_OF_RTS - 1);
    SC_RtsInfoEntry_t *RtsInfoPtr;
    SC_AtsInfoTable_t *AtsInfoPtr;

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);
    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
    RtsInfoPtr->RtsStatus                   = SC_Status_IDLE;
    AtsInfoPtr->NumberOfCommands            = 1;

    /* Give the packet the minimum possible size, to ensure that (CmdOffset < SC_RTS_HDR_WORDS) is met */
    MsgSize = SC_PACKET_MIN_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsCommand_Test_RtsLengthError(void)
{
    SC_RtsEntryHeader_t *Entry;
    size_t               MsgSize1;
    size_t               MsgSize2;
    uint32               EntryOffsetVal;
    SC_AtsIndex_t        AtsIndex = SC_ATS_IDX_C(1);
    SC_RtsIndex_t        RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *  RtsInfoPtr;
    SC_AtsInfoTable_t *  AtsInfoPtr;

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);
    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
    RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;

    Entry = (SC_RtsEntryHeader_t *)SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST);
    EntryOffsetVal =
        SC_RTS_BUFF_SIZE32 - SC_RTS_HDR_WORDS - ((SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD);

    /* Set to generate error message SC_RTS_LNGTH_ERR_EID */
    CFE_MSG_Init((CFE_MSG_Message_t *)Entry, CFE_SB_ValueToMsgId(SC_CMD_MID), SC_PACKET_MAX_SIZE);
    MsgSize1 = SC_PACKET_MIN_SIZE;
    MsgSize2 = SC_PACKET_MAX_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    AtsInfoPtr->NumberOfCommands = 1;
    RtsInfoPtr->NextCommandPtr   = SC_ENTRY_OFFSET_C(EntryOffsetVal);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 1");
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdErrCtr, 1);
    SC_Assert_ID_EQ(SC_OperData.HkPacket.Payload.LastRtsErrSeq, SC_OperData.RtsCtrlBlckAddr->CurrRtsNum);

    EntryOffsetVal += (SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD;
    SC_Assert_IDX_VALUE(SC_OperData.HkPacket.Payload.LastRtsErrCmd, EntryOffsetVal);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LNGTH_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetNextRtsCommand_Test_CommandLengthError(void)
{
    SC_RtsEntryHeader_t *Entry;
    size_t               MsgSize1;
    size_t               MsgSize2;
    uint32               EntryOffsetVal;
    SC_AtsIndex_t        AtsIndex = SC_ATS_IDX_C(1);
    SC_RtsIndex_t        RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *  RtsInfoPtr;
    SC_AtsInfoTable_t *  AtsInfoPtr;

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);
    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
    RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;

    Entry = (SC_RtsEntryHeader_t *)SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST);
    EntryOffsetVal =
        SC_RTS_BUFF_SIZE32 - SC_RTS_HDR_WORDS - ((SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD);

    /* Set to generate error message SC_RTS_LNGTH_ERR_EID */
    CFE_MSG_Init((CFE_MSG_Message_t *)Entry, CFE_SB_ValueToMsgId(SC_CMD_MID), SC_PACKET_MAX_SIZE);
    MsgSize1 = SC_PACKET_MIN_SIZE;
    MsgSize2 = SC_PACKET_MAX_SIZE + 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    AtsInfoPtr->NumberOfCommands = 1;
    RtsInfoPtr->NextCommandPtr   = SC_ENTRY_OFFSET_C(EntryOffsetVal);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 1");
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdErrCtr, 1);
    SC_Assert_ID_EQ(SC_OperData.HkPacket.Payload.LastRtsErrSeq, SC_OperData.RtsCtrlBlckAddr->CurrRtsNum);

    EntryOffsetVal += (SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD;
    SC_Assert_IDX_VALUE(SC_OperData.HkPacket.Payload.LastRtsErrCmd, EntryOffsetVal);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_CMD_LNGTH_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetNextRtsCommand_Test_ZeroCommandLength(void)
{
    SC_RtsEntryHeader_t *Entry;
    size_t               MsgSize1;
    size_t               MsgSize2;
    uint32               EntryOffsetVal;
    SC_AtsIndex_t        AtsIndex = SC_ATS_IDX_C(1);
    SC_RtsIndex_t        RtsIndex = SC_RTS_IDX_C(SC_LAST_RTS_WITH_EVENTS - 1);
    SC_RtsInfoEntry_t *  RtsInfoPtr;
    SC_AtsInfoTable_t *  AtsInfoPtr;

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);
    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
    RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;

    Entry = (SC_RtsEntryHeader_t *)SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST);
    EntryOffsetVal =
        SC_RTS_BUFF_SIZE32 - SC_RTS_HDR_WORDS - ((SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD) - 1;

    /* Set to generate error message SC_RTS_LNGTH_ERR_EID */
    CFE_MSG_Init((CFE_MSG_Message_t *)Entry, CFE_SB_ValueToMsgId(SC_CMD_MID), SC_PACKET_MIN_SIZE - 1);
    MsgSize1 = SC_PACKET_MIN_SIZE - 1;
    MsgSize2 = SC_PACKET_MIN_SIZE - 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    AtsInfoPtr->NumberOfCommands = 1;
    RtsInfoPtr->NextCommandPtr   = SC_ENTRY_OFFSET_C(EntryOffsetVal);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_COMPL_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetNextRtsCommand_Test_ZeroCommandLengthLastRts(void)
{
    SC_RtsEntryHeader_t *Entry;
    size_t               MsgSize1;
    size_t               MsgSize2;
    uint32               EntryOffsetVal;
    SC_AtsIndex_t        AtsIndex = SC_ATS_IDX_C(1);
    SC_RtsIndex_t        RtsIndex = SC_RTS_IDX_C(SC_LAST_RTS_WITH_EVENTS);
    SC_RtsInfoEntry_t *  RtsInfoPtr;
    SC_AtsInfoTable_t *  AtsInfoPtr;

    /* This test intended to test the first event suppressed RTS, skip if none are suppressed */
    if (SC_LAST_RTS_WITH_EVENTS != SC_NUMBER_OF_RTS)
    {
        AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);
        RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

        SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
        SC_AppData.CurrentTime                  = 1;
        SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
        RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;

        Entry = (SC_RtsEntryHeader_t *)SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST);
        EntryOffsetVal =
            SC_RTS_BUFF_SIZE32 - SC_RTS_HDR_WORDS - ((SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD) - 1;

        /* Set to generate error message SC_RTS_LNGTH_ERR_EID */
        CFE_MSG_Init((CFE_MSG_Message_t *)Entry, CFE_SB_ValueToMsgId(SC_CMD_MID), SC_PACKET_MIN_SIZE - 1);
        MsgSize1 = SC_PACKET_MIN_SIZE - 1;
        MsgSize2 = SC_PACKET_MIN_SIZE - 1;

        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

        /* Set so checksum will pass in SC_ProcessRtpCommand */
        UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

        AtsInfoPtr->NumberOfCommands = 1;
        RtsInfoPtr->NextCommandPtr   = SC_ENTRY_OFFSET_C(EntryOffsetVal);

        /* Execute the function being tested */
        UtAssert_VOIDCALL(SC_GetNextRtsCommand());

        /* Verify results */
        UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    }
}

void SC_GetNextRtsCommand_Test_EndOfBuffer(void)
{
    SC_RtsEntryHeader_t *Entry;
    size_t               MsgSize;
    uint32               EntryOffsetVal;
    SC_AtsIndex_t        AtsIndex = SC_ATS_IDX_C(1);
    SC_RtsIndex_t        RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *  RtsInfoPtr;
    SC_AtsInfoTable_t *  AtsInfoPtr;

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);
    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
    SC_AppData.CurrentTime                  = 1;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
    RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;

    Entry = (SC_RtsEntryHeader_t *)SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST);
    EntryOffsetVal =
        SC_RTS_BUFF_SIZE32 - SC_RTS_HDR_WORDS - ((SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD) - 1;

    /* Set to generate error message SC_RTS_LNGTH_ERR_EID */
    CFE_MSG_Init((CFE_MSG_Message_t *)Entry, CFE_SB_ValueToMsgId(SC_CMD_MID), 2 * SC_RTS_BUFF_SIZE);
    MsgSize = 2 * SC_RTS_BUFF_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    AtsInfoPtr->NumberOfCommands = 1;
    RtsInfoPtr->NextCommandPtr   = SC_ENTRY_OFFSET_C(EntryOffsetVal);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_COMPL_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetNextRtsCommand_Test_EndOfBufferLastRts(void)
{
    SC_RtsEntryHeader_t *Entry;
    size_t               MsgSize;
    SC_AtsIndex_t        AtsIndex = SC_ATS_IDX_C(1);
    uint32               EntryOffsetVal;
    SC_RtsIndex_t        RtsIndex = SC_RTS_IDX_C(SC_LAST_RTS_WITH_EVENTS);
    SC_RtsInfoEntry_t *  RtsInfoPtr;
    SC_AtsInfoTable_t *  AtsInfoPtr;

    /* This test intended to test the first event suppressed RTS, skip if none are suppressed */
    if (SC_LAST_RTS_WITH_EVENTS != SC_NUMBER_OF_RTS)
    {
        AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);
        RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

        SC_AppData.NextCmdTime[SC_Process_RTP]  = 0;
        SC_AppData.CurrentTime                  = 1;
        SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(RtsIndex);
        RtsInfoPtr->RtsStatus                   = SC_Status_EXECUTING;

        Entry = (SC_RtsEntryHeader_t *)SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST);
        EntryOffsetVal =
            SC_RTS_BUFF_SIZE32 - SC_RTS_HDR_WORDS - ((SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD) - 1;

        /* Set to generate error message SC_RTS_LNGTH_ERR_EID */
        CFE_MSG_Init((CFE_MSG_Message_t *)Entry, CFE_SB_ValueToMsgId(SC_CMD_MID), 2 * SC_RTS_BUFF_SIZE);
        MsgSize = 2 * SC_RTS_BUFF_SIZE;

        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

        /* Set so checksum will pass in SC_ProcessRtpCommand */
        UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

        AtsInfoPtr->NumberOfCommands = 1;
        RtsInfoPtr->NextCommandPtr   = SC_ENTRY_OFFSET_C(EntryOffsetVal);

        /* Execute the function being tested */
        UtAssert_VOIDCALL(SC_GetNextRtsCommand());

        /* Verify results */
        UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    }
}

void SC_GetNextAtsCommand_Test_Starting(void)
{
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_STARTING;

    /* Execute the function being tested */
    /* NOTE: Calling SC_ProcessRtpCommand instead of SC_GetNextRtsCommand - SC_ProcessRtpCommand calls
     * SC_GetNextRtsCommand, and it's much easier to test this way. */
    UtAssert_VOIDCALL(SC_GetNextAtsCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_Status_EXECUTING,
                  "SC_OperData.AtsCtrlBlckAddr -> AtpState == SC_Status_EXECUTING");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextAtsCommand_Test_Idle(void)
{
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_IDLE;

    /* Execute the function being tested */
    /* NOTE: Calling SC_ProcessRtpCommand instead of SC_GetNextRtsCommand - SC_ProcessRtpCommand calls
     * SC_GetNextRtsCommand, and it's much easier to test this way. */
    UtAssert_VOIDCALL(SC_GetNextAtsCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_Status_IDLE,
                  "SC_OperData.AtsCtrlBlckAddr -> AtpState == SC_Status_IDLE");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextAtsCommand_Test_GetNextCommand(void)
{
    SC_AtsEntryHeader_t *         Entry;
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    AtsInfoPtr     = SC_GetAtsInfoObject(AtsIndex);
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    Entry            = (SC_AtsEntryHeader_t *)SC_GetAtsEntryAtOffset(AtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->CmdNumber = SC_COMMAND_NUM_C(1);

    SC_AppData.NextCmdTime[SC_Process_ATP]                           = 0;
    SC_AppData.CurrentTime                                           = 1;
    SC_OperData.AtsCtrlBlckAddr->AtpState                            = SC_Status_EXECUTING;
    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum                          = SC_AtsIndexToNum(AtsIndex);
    SC_GetAtsCommandNumAtSeq(AtsIndex, SC_SEQUENCE_IDX_C(0))->CmdNum = SC_COMMAND_NUM_C(1);
    SC_GetAtsCommandNumAtSeq(AtsIndex, SC_SEQUENCE_IDX_C(1))->CmdNum = SC_COMMAND_NUM_C(2);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    AtsInfoPtr->NumberOfCommands = 100;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextAtsCommand());

    /* Verify results */
    SC_Assert_IDX_VALUE(SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr, 1);
    SC_Assert_ID_EQ(SC_OperData.AtsCtrlBlckAddr->CmdNumber,
                    SC_GetAtsCommandNumAtSeq(AtsIndex, SC_SEQUENCE_IDX_C(1))->CmdNum);
    UtAssert_INT32_EQ(SC_AppData.NextCmdTime[SC_Process_ATP], 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextAtsCommand_Test_ExecutionACompleted(void)
{
    SC_AtsEntryHeader_t *         Entry;
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

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(2);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(0);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextAtsCommand());

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_COMPL_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetNextAtsCommand_Test_ExecutionBCompleted(void)
{
    SC_AtsEntryHeader_t *         Entry;
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

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(1);
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_COMMAND_NUM_C(0);

    StatusEntryPtr->Status = SC_Status_LOADED;
    CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_FIRST;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextAtsCommand());

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_COMPL_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void UtTest_Setup(void)
{
    UtTest_Add(SC_GetNextRtsTime_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_GetNextRtsTime_Test_Nominal");
    UtTest_Add(SC_GetNextRtsTime_Test_InvalidRtsNum, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsTime_Test_InvalidRtsNum");
    UtTest_Add(SC_GetNextRtsTime_Test_RtsPriority, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsTime_Test_RtsPriority");
    UtTest_Add(SC_GetNextRtsCommand_Test_GetNextCommand, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_GetNextCommand");
    UtTest_Add(SC_GetNextRtsCommand_Test_RtsNumZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_RtsNumZero");
    UtTest_Add(SC_GetNextRtsCommand_Test_RtsNumMax, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_RtsNumMax");
    UtTest_Add(SC_GetNextRtsCommand_Test_RtsNumOverMax, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_RtsNumOverMax");
    UtTest_Add(SC_GetNextRtsCommand_Test_RtsNotExecuting, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_RtsNotExecuting");
    UtTest_Add(SC_GetNextRtsCommand_Test_RtsLengthError, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_RtsLengthError");
    UtTest_Add(SC_GetNextRtsCommand_Test_CommandLengthError, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_CommandLengthError");
    UtTest_Add(SC_GetNextRtsCommand_Test_ZeroCommandLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_ZeroCommandLength");
    UtTest_Add(SC_GetNextRtsCommand_Test_ZeroCommandLengthLastRts, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_ZeroCommandLengthLastRts");
    UtTest_Add(SC_GetNextRtsCommand_Test_EndOfBufferLastRts, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_EndOfBufferLastRts");
    UtTest_Add(SC_GetNextRtsCommand_Test_EndOfBuffer, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_EndOfBuffer");
    UtTest_Add(SC_GetNextAtsCommand_Test_Starting, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextAtsCommand_Test_Starting");
    UtTest_Add(SC_GetNextAtsCommand_Test_Idle, SC_Test_Setup, SC_Test_TearDown, "SC_GetNextAtsCommand_Test_Idle");
    UtTest_Add(SC_GetNextAtsCommand_Test_GetNextCommand, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextAtsCommand_Test_GetNextCommand");
    UtTest_Add(SC_GetNextAtsCommand_Test_ExecutionACompleted, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextAtsCommand_Test_ExecutionACompleted");
    UtTest_Add(SC_GetNextAtsCommand_Test_ExecutionBCompleted, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextAtsCommand_Test_ExecutionBCompleted");
}
