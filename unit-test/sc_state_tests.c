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
    SC_OperData.RtsInfoTblAddr[0].RtsStatus       = SC_EXECUTING;
    SC_OperData.RtsInfoTblAddr[0].NextCommandTime = SC_MAX_TIME;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsTime());

    /* Verify results */
    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->RtsNumber == 1, "SC_OperData.RtsCtrlBlckAddr->RtsNumber == 1");
    UtAssert_True(SC_AppData.NextCmdTime[1] == SC_MAX_TIME, "SC_AppData.NextCmdTime[1] == SC_MAX_TIME");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsTime_Test_InvalidRtsNumber(void)
{
    uint8 i;

    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        SC_OperData.RtsInfoTblAddr[i].RtsStatus = -1;
    }

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsTime());

    /* Verify results */
    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->RtsNumber == SC_INVALID_RTS_NUMBER,
                  "SC_OperData.RtsCtrlBlckAddr->RtsNumber == SC_INVALID_RTS_NUMBER");
    UtAssert_True(SC_AppData.NextCmdTime[SC_RTP] == SC_MAX_TIME, "SC_AppData.NextCmdTime[SC_RTP] == SC_MAX_TIME");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsTime_Test_RtsPriority(void)
{
    SC_OperData.RtsInfoTblAddr[0].RtsStatus       = SC_EXECUTING;
    SC_OperData.RtsInfoTblAddr[0].NextCommandTime = SC_MAX_TIME;

    SC_OperData.RtsInfoTblAddr[1].RtsStatus       = SC_EXECUTING;
    SC_OperData.RtsInfoTblAddr[1].NextCommandTime = SC_MAX_TIME - 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsTime());

    /* Verify results */
    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->RtsNumber == 2, "SC_OperData.RtsCtrlBlckAddr->RtsNumber == 2 ");
    UtAssert_True(SC_AppData.NextCmdTime[1] == SC_MAX_TIME - 1, "SC_AppData.NextCmdTime[1] == SC_MAX_TIME - 1");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_UpdateNextTime_Test_Atp(void)
{
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_UpdateNextTime());

    /* Verify results */
    UtAssert_True(SC_AppData.NextProcNumber == SC_ATP, "SC_AppData.NextProcNumber == SC_ATP");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_UpdateNextTime_Test_Atp2(void)
{
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_EXECUTING;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber = SC_NUMBER_OF_RTS + 1;
    SC_AppData.NextCmdTime[SC_RTP]         = 0;
    SC_AppData.NextCmdTime[SC_ATP]         = 10;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_UpdateNextTime());

    /* Verify results */
    UtAssert_True(SC_AppData.NextProcNumber == SC_ATP, "SC_AppData.NextProcNumber == SC_ATP");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_UpdateNextTime_Test_Rtp(void)
{
    SC_OperData.RtsCtrlBlckAddr->RtsNumber = 10;
    SC_AppData.NextCmdTime[SC_RTP]         = 0;
    SC_AppData.NextCmdTime[SC_ATP]         = 10;

    SC_OperData.RtsInfoTblAddr[0].RtsStatus       = SC_EXECUTING;
    SC_OperData.RtsInfoTblAddr[0].NextCommandTime = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_UpdateNextTime());

    /* Verify results */
    UtAssert_True(SC_AppData.NextProcNumber == SC_RTP, "SC_AppData.NextProcNumber == SC_RTP");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_UpdateNextTime_Test_RtpAtpPriority(void)
{
    SC_OperData.RtsCtrlBlckAddr->RtsNumber = 0;
    SC_AppData.NextCmdTime[SC_RTP]         = 0;
    SC_AppData.NextCmdTime[SC_ATP]         = 0;

    SC_OperData.RtsInfoTblAddr[SC_NUMBER_OF_RTS - 1].RtsStatus       = SC_EXECUTING;
    SC_OperData.RtsInfoTblAddr[SC_NUMBER_OF_RTS - 1].NextCommandTime = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_UpdateNextTime());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsCommand_Test_GetNextCommand(void)
{
    size_t MsgSize;

    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands                                   = 1;

    /* Give the packet the minimum possible size, to ensure that (CmdOffset < SC_RTS_HDR_WORDS) is met */
    MsgSize = SC_PACKET_MIN_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[0].NextCommandPtr ==
                      (SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 1) / SC_BYTES_IN_WORD,
                  "SC_OperData.RtsInfoTblAddr[0].NextCommandPtr == (SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 1) / "
                  "SC_BYTES_IN_WORD");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsCommand_Test_RtsNumberZero(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsCommand_Test_RtsNumberMax(void)
{
    size_t MsgSize;

    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = SC_NUMBER_OF_RTS;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands                                   = 1;

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

void SC_GetNextRtsCommand_Test_RtsNumberOverMax(void)
{
    size_t MsgSize;

    SC_AppData.NextCmdTime[SC_RTP]                 = 0;
    SC_AppData.CurrentTime                         = 1;
    SC_AppData.NextProcNumber                      = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber         = SC_NUMBER_OF_RTS + 1;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

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
    size_t MsgSize;

    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = SC_NUMBER_OF_RTS;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_IDLE;
    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands                                   = 1;

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

    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;

    Entry = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[0][0];

    /* Set to generate error message SC_RTS_LNGTH_ERR_EID */
    CFE_MSG_Init((CFE_MSG_Message_t *)Entry, CFE_SB_ValueToMsgId(SC_CMD_MID), SC_PACKET_MAX_SIZE);
    MsgSize1 = SC_PACKET_MIN_SIZE;
    MsgSize2 = SC_PACKET_MAX_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    SC_OperData.RtsInfoTblAddr[0].NextCommandPtr =
        (SC_RTS_BUFF_SIZE32 - (SC_RTS_HDR_WORDS)) - ((SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[0].CmdErrCtr == 1, "SC_OperData.RtsInfoTblAddr[0].CmdErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.LastRtsErrSeq == SC_OperData.RtsCtrlBlckAddr->RtsNumber,
                  "SC_OperData.HkPacket.Payload.LastRtsErrSeq == SC_OperData.RtsCtrlBlckAddr->RtsNumber");

    UtAssert_True(SC_OperData.HkPacket.Payload.LastRtsErrCmd ==
                      SC_OperData.RtsInfoTblAddr[0].NextCommandPtr +
                          ((SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD),
                  "SC_OperData.HkPacket.Payload.LastRtsErrCmd == SC_OperData.RtsInfoTblAddr[0].NextCommandPtr + "
                  "((SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD)");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LNGTH_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetNextRtsCommand_Test_CommandLengthError(void)
{
    SC_RtsEntryHeader_t *Entry;
    size_t               MsgSize1;
    size_t               MsgSize2;

    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;

    Entry = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[0][0];

    /* Set to generate error message SC_RTS_LNGTH_ERR_EID */
    CFE_MSG_Init((CFE_MSG_Message_t *)Entry, CFE_SB_ValueToMsgId(SC_CMD_MID), SC_PACKET_MAX_SIZE);
    MsgSize1 = SC_PACKET_MIN_SIZE;
    MsgSize2 = SC_PACKET_MAX_SIZE + 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    SC_OperData.RtsInfoTblAddr[0].NextCommandPtr =
        (SC_RTS_BUFF_SIZE32 - (SC_RTS_HDR_WORDS)) - ((SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 1, "SC_OperData.HkPacket.Payload.RtsCmdErrCtr == 1");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[0].CmdErrCtr == 1, "SC_OperData.RtsInfoTblAddr[0].CmdErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.LastRtsErrSeq == SC_OperData.RtsCtrlBlckAddr->RtsNumber,
                  "SC_OperData.HkPacket.Payload.LastRtsErrSeq == SC_OperData.RtsCtrlBlckAddr->RtsNumber");

    UtAssert_True(SC_OperData.HkPacket.Payload.LastRtsErrCmd ==
                      SC_OperData.RtsInfoTblAddr[0].NextCommandPtr +
                          ((SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD),
                  "SC_OperData.HkPacket.Payload.LastRtsErrCmd == SC_OperData.RtsInfoTblAddr[0].NextCommandPtr + "
                  "((SC_PACKET_MIN_SIZE + SC_RTS_HEADER_SIZE + 3) / SC_BYTES_IN_WORD)");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_CMD_LNGTH_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetNextRtsCommand_Test_ZeroCommandLength(void)
{
    SC_RtsEntryHeader_t *Entry;
    size_t               MsgSize1;
    size_t               MsgSize2;

    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = SC_LAST_RTS_WITH_EVENTS;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;

    Entry = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[SC_LAST_RTS_WITH_EVENTS - 1][0];

    /* Set to generate error message SC_RTS_LNGTH_ERR_EID */
    CFE_MSG_Init((CFE_MSG_Message_t *)Entry, CFE_SB_ValueToMsgId(SC_CMD_MID), SC_LAST_RTS_WITH_EVENTS - 1);
    MsgSize1 = SC_PACKET_MIN_SIZE - 1;
    MsgSize2 = SC_PACKET_MIN_SIZE - 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    SC_OperData.RtsInfoTblAddr[0].NextCommandPtr = (SC_RTS_BUFF_SIZE - (SC_RTS_HDR_WORDS)) -
                                                   ((SC_PACKET_MAX_SIZE + SC_RTS_HEADER_SIZE + 1) / SC_BYTES_IN_WORD) -
                                                   1;

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

    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = SC_LAST_RTS_WITH_EVENTS + 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;

    Entry = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[SC_LAST_RTS_WITH_EVENTS][0];

    /* Set to generate error message SC_RTS_LNGTH_ERR_EID */
    CFE_MSG_Init((CFE_MSG_Message_t *)Entry, CFE_SB_ValueToMsgId(SC_CMD_MID), SC_PACKET_MIN_SIZE - 1);
    MsgSize1 = SC_PACKET_MIN_SIZE - 1;
    MsgSize2 = SC_PACKET_MIN_SIZE - 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    SC_OperData.RtsInfoTblAddr[0].NextCommandPtr = (SC_RTS_BUFF_SIZE - (SC_RTS_HDR_WORDS)) -
                                                   ((SC_PACKET_MAX_SIZE + SC_RTS_HEADER_SIZE + 1) / SC_BYTES_IN_WORD) -
                                                   1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextRtsCommand_Test_EndOfBuffer(void)
{
    SC_RtsEntryHeader_t *Entry;
    size_t               MsgSize;

    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;

    Entry = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[0][0];

    /* Set to generate error message SC_RTS_LNGTH_ERR_EID */
    CFE_MSG_Init((CFE_MSG_Message_t *)Entry, CFE_SB_ValueToMsgId(SC_CMD_MID), 2 * SC_RTS_BUFF_SIZE);
    MsgSize = 2 * SC_RTS_BUFF_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    SC_OperData.RtsInfoTblAddr[0].NextCommandPtr = (SC_RTS_BUFF_SIZE - (SC_RTS_HDR_WORDS)) -
                                                   ((SC_PACKET_MAX_SIZE + SC_RTS_HEADER_SIZE + 1) / SC_BYTES_IN_WORD) -
                                                   1;

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

    SC_AppData.NextCmdTime[SC_RTP]                                                   = 0;
    SC_AppData.CurrentTime                                                           = 1;
    SC_AppData.NextProcNumber                                                        = SC_RTP;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber                                           = SC_LAST_RTS_WITH_EVENTS + 1;
    SC_OperData.RtsInfoTblAddr[SC_OperData.RtsCtrlBlckAddr->RtsNumber - 1].RtsStatus = SC_EXECUTING;

    Entry = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[SC_LAST_RTS_WITH_EVENTS][0];

    /* Set to generate error message SC_RTS_LNGTH_ERR_EID */
    CFE_MSG_Init((CFE_MSG_Message_t *)Entry, CFE_SB_ValueToMsgId(SC_CMD_MID), 2 * SC_RTS_BUFF_SIZE);
    MsgSize = 2 * SC_RTS_BUFF_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set so checksum will pass in SC_ProcessRtpCommand */
    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_ValidateChecksum), 1, true);

    SC_OperData.AtsInfoTblAddr[1].NumberOfCommands = 1;

    SC_OperData.RtsInfoTblAddr[0].NextCommandPtr = (SC_RTS_BUFF_SIZE - (SC_RTS_HDR_WORDS)) -
                                                   ((SC_PACKET_MAX_SIZE + SC_RTS_HEADER_SIZE + 1) / SC_BYTES_IN_WORD) -
                                                   1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextRtsCommand());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextAtsCommand_Test_Starting(void)
{
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_STARTING;

    /* Execute the function being tested */
    /* NOTE: Calling SC_ProcessRtpCommand instead of SC_GetNextRtsCommand - SC_ProcessRtpCommand calls
     * SC_GetNextRtsCommand, and it's much easier to test this way. */
    UtAssert_VOIDCALL(SC_GetNextAtsCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING,
                  "SC_OperData.AtsCtrlBlckAddr -> AtpState == SC_EXECUTING");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextAtsCommand_Test_Idle(void)
{
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_IDLE;

    /* Execute the function being tested */
    /* NOTE: Calling SC_ProcessRtpCommand instead of SC_GetNextRtsCommand - SC_ProcessRtpCommand calls
     * SC_GetNextRtsCommand, and it's much easier to test this way. */
    UtAssert_VOIDCALL(SC_GetNextAtsCommand());

    /* Verify results */
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_IDLE,
                  "SC_OperData.AtsCtrlBlckAddr -> AtpState == SC_IDLE");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextAtsCommand_Test_GetNextCommand(void)
{
    SC_AtsEntryHeader_t *Entry;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 1;
    SC_AppData.AtsTimeIndexBuffer[0][0]    = 1;
    SC_AppData.AtsTimeIndexBuffer[0][1]    = 2;

    SC_OperData.AtsCmdStatusTblAddr[0][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[0][0]    = 0;

    SC_OperData.AtsInfoTblAddr[SC_ATP].NumberOfCommands = 100;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextAtsCommand());

    /* Verify results */
    UtAssert_INT32_EQ(SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr, 1);
    UtAssert_INT32_EQ(SC_OperData.AtsCtrlBlckAddr->CmdNumber, SC_AppData.AtsTimeIndexBuffer[0][1]);
    UtAssert_INT32_EQ(SC_AppData.NextCmdTime[SC_ATP], 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetNextAtsCommand_Test_ExecutionACompleted(void)
{
    SC_AtsEntryHeader_t *Entry;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 2;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 0;

    SC_OperData.AtsCmdStatusTblAddr[0][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[0][0]    = 0;

    SC_OperData.AtsInfoTblAddr[SC_ATP].NumberOfCommands = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextAtsCommand());

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_COMPL_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetNextAtsCommand_Test_ExecutionBCompleted(void)
{
    SC_AtsEntryHeader_t *Entry;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[0][0];
    Entry->CmdNumber = 1;

    SC_AppData.NextCmdTime[SC_ATP]        = 0;
    SC_AppData.CurrentTime                = 1;
    SC_AppData.NextProcNumber             = SC_ATP;
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 1;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = 0;

    SC_OperData.AtsCmdStatusTblAddr[1][0] = SC_LOADED;
    SC_AppData.AtsCmdIndexBuffer[1][0]    = 0;

    SC_OperData.AtsInfoTblAddr[SC_ATP].NumberOfCommands = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetNextAtsCommand());

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ATS_COMPL_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void UtTest_Setup(void)
{
    UtTest_Add(SC_GetNextRtsTime_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_GetNextRtsTime_Test_Nominal");
    UtTest_Add(SC_GetNextRtsTime_Test_InvalidRtsNumber, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsTime_Test_InvalidRtsNumber");
    UtTest_Add(SC_GetNextRtsTime_Test_RtsPriority, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsTime_Test_RtsPriority");
    UtTest_Add(SC_UpdateNextTime_Test_Atp, SC_Test_Setup, SC_Test_TearDown, "SC_UpdateNextTime_Test_Atp");
    UtTest_Add(SC_UpdateNextTime_Test_Atp2, SC_Test_Setup, SC_Test_TearDown, "SC_UpdateNextTime_Test_Atp2");
    UtTest_Add(SC_UpdateNextTime_Test_Rtp, SC_Test_Setup, SC_Test_TearDown, "SC_UpdateNextTime_Test_Rtp");
    UtTest_Add(SC_UpdateNextTime_Test_RtpAtpPriority, SC_Test_Setup, SC_Test_TearDown,
               "SC_UpdateNextTime_Test_RtpAtpPriority");
    UtTest_Add(SC_GetNextRtsCommand_Test_GetNextCommand, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_GetNextCommand");
    UtTest_Add(SC_GetNextRtsCommand_Test_RtsNumberZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_RtsNumberZero");
    UtTest_Add(SC_GetNextRtsCommand_Test_RtsNumberMax, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_RtsNumberMax");
    UtTest_Add(SC_GetNextRtsCommand_Test_RtsNumberOverMax, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_RtsNumberOverMax");
    UtTest_Add(SC_GetNextRtsCommand_Test_RtsNotExecuting, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_RtsNotExecuting");
    UtTest_Add(SC_GetNextRtsCommand_Test_RtsLengthError, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_RtsLengthError");
    UtTest_Add(SC_GetNextRtsCommand_Test_CommandLengthError, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_CommandLengthError");
    UtTest_Add(SC_GetNextRtsCommand_Test_ZeroCommandLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetNextRtsCommand_Test_ZeroCommandLength");

    /* Only run if SC_LAST_RTS_WITH_EVENTS < SC_NUMBER_OF_RTS */
    if (SC_LAST_RTS_WITH_EVENTS < SC_NUMBER_OF_RTS)
    {
        UtTest_Add(SC_GetNextRtsCommand_Test_ZeroCommandLengthLastRts, SC_Test_Setup, SC_Test_TearDown,
                   "SC_GetNextRtsCommand_Test_ZeroCommandLengthLastRts");
        UtTest_Add(SC_GetNextRtsCommand_Test_EndOfBufferLastRts, SC_Test_Setup, SC_Test_TearDown,
                   "SC_GetNextRtsCommand_Test_EndOfBufferLastRts");
    }

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
