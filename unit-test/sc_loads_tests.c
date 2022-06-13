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

/* sc_loads_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

uint32 SC_APP_TEST_GlobalAtsCmdStatusTbl[SC_NUMBER_OF_ATS * SC_MAX_ATS_CMDS];

SC_AtpControlBlock_t SC_APP_TEST_GlobalAtsCtrlBlck;

/*
 * Function Definitions
 */

uint8 SC_LOADS_TEST_GetTotalMsgLengthHook_RunCount;
int32 SC_LOADS_TEST_CFE_MSG_GetSizeHook1(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                         const UT_StubContext_t *Context)
{
    SC_LOADS_TEST_GetTotalMsgLengthHook_RunCount += 1;

    if (SC_LOADS_TEST_GetTotalMsgLengthHook_RunCount == 1)
        SC_OperData.AtsCmdStatusTblAddr[0][1] = SC_LOADED;

    return CFE_SUCCESS;
} /* end SC_LOADS_TEST_CFE_MSG_GetSizeHook1 */

CFE_TIME_Compare_t UT_SC_Insert_CompareHookAgreaterthanB(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                                         const UT_StubContext_t *Context)
{
    return CFE_TIME_A_GT_B;
} /* end CFE_TIME_Compare_t UT_SC_StartAtsRq_CompareHookAgreaterthanB */

void SC_LoadAts_Test_Nominal(void)
{
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    size_t               MsgSize   = sizeof(SC_NoArgsCmd_t);
    SC_AtsEntryHeader_t *Entry;
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    uint8                AtsIndex = 0;

    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));
    memset(&AtsTable, 0, sizeof(AtsTable));

    SC_InitTables();

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] = SC_EMPTY;

    /* Set to satisfy the conditions of if-statement below comment "if the length of the command is valid", along
     * with the if-statement immediately after */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    printf("DONE WITH TEST SETUP\n");
    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);
    printf("AFTER FUNCTION RUN\n");
    /* Verify results */
    // UtAssert_True(SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == 0, "SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == 0");
    // UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_LOADED,
    //              "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_LOADED");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 1,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_LoadAts_Test_Nominal */

void SC_LoadAts_Test_CmdRunOffEndOfBuffer(void)
{
    SC_AtsEntryHeader_t *Entry;
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    uint8                AtsIndex = 0;
    size_t               MsgSize;
    int                  BufEntrySize;
    int                  MaxBufEntries;
    int                  i, j;

    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));

    SC_InitTables();

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;

    /* Causes CFE_MSG_GetSize to satisfy the conditions of if-statement below comment "if the length of the command is
     * valid", but NOT the if-statement immediately after */
    MsgSize       = SC_PACKET_MAX_SIZE;
    BufEntrySize  = (MsgSize + SC_ROUND_UP_BYTES + SC_ATS_HEADER_SIZE) / SC_BYTES_IN_WORD;
    MaxBufEntries = SC_ATS_BUFF_SIZE32 / BufEntrySize + 1;

    for (i = 0, j = 0; i < MaxBufEntries; i++, j += BufEntrySize)
    {
        Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][j];
        Entry->CmdNumber = i + 1;

        SC_OperData.AtsCmdStatusTblAddr[AtsIndex][j] = SC_EMPTY;

        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    }

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    UtAssert_True(SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == SC_ERROR,
                  "SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == SC_ERROR");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_EMPTY,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_EMPTY");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_LoadAts_Test_CmdRunOffEndOfBuffer */

void SC_LoadAts_Test_CmdLengthInvalid(void)
{
    size_t               MsgSize;
    SC_AtsEntryHeader_t *Entry;
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    uint8                AtsIndex = 0;

    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));

    SC_InitTables();

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] = SC_EMPTY;

    /* Set to make the if-statement below comment "if the length of the command is valid" fail */
    MsgSize = SC_PACKET_MAX_SIZE + 1;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    UtAssert_True(SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == SC_ERROR,
                  "SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == SC_ERROR");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_EMPTY,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_EMPTY");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_LoadAts_Test_CmdLengthInvalid */

void SC_LoadAts_Test_CmdLengthZero(void)
{
    size_t               MsgSize;
    SC_AtsEntryHeader_t *Entry;
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    uint8                AtsIndex = 0;

    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));

    SC_InitTables();

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber = 1;

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] = SC_EMPTY;

    /* Set to make the if-statement below comment "if the length of the command is valid" fail */
    MsgSize = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    UtAssert_True(SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == SC_ERROR,
                  "SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == SC_ERROR");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_EMPTY,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_EMPTY");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_LoadAts_Test_CmdLengthZero */

void SC_LoadAts_Test_CmdNumberInvalid(void)
{
    size_t               MsgSize;
    SC_AtsEntryHeader_t *Entry;
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    uint8                AtsIndex = 0;

    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));

    SC_InitTables();

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber = SC_MAX_ATS_CMDS * 2;

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] = SC_EMPTY;

    /* Set to make the if-statement below comment "if the length of the command is valid" fail */
    MsgSize = SC_PACKET_MAX_SIZE + 1;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    UtAssert_True(SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == SC_ERROR,
                  "SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == SC_ERROR");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_EMPTY,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_EMPTY");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_LoadAts_Test_CmdNumberInvalid */

void SC_LoadAts_Test_EndOfLoadReached(void)
{
    size_t               MsgSize;
    SC_AtsEntryHeader_t *Entry;
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    uint8                AtsIndex = 0;

    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));

    SC_InitTables();

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber = 0;

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] = SC_EMPTY;

    /* Set to make the if-statement below comment "if the length of the command is valid" fail */
    MsgSize = SC_PACKET_MAX_SIZE + 1;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    UtAssert_True(SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == SC_ERROR,
                  "SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == SC_ERROR");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_EMPTY,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_EMPTY");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_LoadAts_Test_EndOfLoadReached */

void SC_LoadAts_Test_AtsBufferTooSmall(void)
{
    SC_AtsEntryHeader_t *Entry;
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    uint8                AtsIndex = 0;
    size_t               MsgSize1;
    size_t               MsgSize2;
    int                  BufEntrySize;
    int                  MaxBufEntries;
    int                  i, j;

    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));

    SC_InitTables();

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;

    /* Set to reach block of code starting with comment "even the smallest command will not fit in the buffer" */
    MsgSize1      = SC_PACKET_MAX_SIZE;
    BufEntrySize  = ((MsgSize1 + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD) + SC_ATS_HDR_NOPKT_WORDS;
    MaxBufEntries = SC_ATS_BUFF_SIZE32 / BufEntrySize;

    for (i = 0, j = 0; i < MaxBufEntries; i++, j += BufEntrySize)
    {
        Entry                                        = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][j];
        Entry->CmdNumber                             = i + 1;
        SC_OperData.AtsCmdStatusTblAddr[AtsIndex][j] = SC_EMPTY;
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);
    }

    /* Next entry should not leave enough buffer space for an ATS command header */
    Entry                                        = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][j];
    Entry->CmdNumber                             = i++ + 1;
    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][j] = SC_EMPTY;

    /* Use the remaining buffer space to calculate the final message size */
    MsgSize2 = (SC_ATS_BUFF_SIZE32 - SC_ATS_HDR_WORDS - j) * SC_BYTES_IN_WORD;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Set up final entry that will create error condition */
    j += ((MsgSize2 + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD) + SC_ATS_HDR_NOPKT_WORDS;
    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][j];
    Entry->CmdNumber = i + 1;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][j] = SC_EMPTY;

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    UtAssert_True(SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == SC_ERROR,
                  "SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == SC_ERROR");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_EMPTY,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_EMPTY");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_LoadAts_Test_AtsBufferTooSmall */

void SC_LoadAts_Test_AtsEntryOverflow(void)
{
    SC_AtsEntryHeader_t *Entry;
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    uint8                AtsIndex = 0;
    size_t               MsgSize1;
    size_t               MsgSize2;
    int                  BufEntrySize;
    int                  MaxBufEntries;
    int                  i, j;

    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));
    memset(&AtsTable, 0, sizeof(AtsTable));

    SC_InitTables();

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;

    MsgSize1      = SC_PACKET_MAX_SIZE;
    BufEntrySize  = ((MsgSize1 + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD) + SC_ATS_HDR_NOPKT_WORDS;
    MaxBufEntries = SC_ATS_BUFF_SIZE32 / BufEntrySize;

    for (i = 0, j = 0; i < MaxBufEntries; i++, j += BufEntrySize)
    {
        Entry                                        = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][j];
        Entry->CmdNumber                             = i + 1;
        SC_OperData.AtsCmdStatusTblAddr[AtsIndex][j] = SC_EMPTY;
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);
    }

    /* Next entry should not leave enough buffer space for an ATS command header */
    Entry                                        = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][j];
    Entry->CmdNumber                             = i++ + 1;
    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][j] = SC_EMPTY;

    /* Use the remaining buffer space to calculate the final message size */
    MsgSize2 = (SC_ATS_BUFF_SIZE32 - SC_ATS_HDR_WORDS + 4 - j) * SC_BYTES_IN_WORD;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Set up final entry that will create condition */
    j += ((MsgSize2 + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD) + SC_ATS_HDR_WORDS;
    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][j];
    Entry->CmdNumber = i + 1;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][j] = SC_EMPTY;

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_LoadAts_Test_AtsEntryOverflow */

void SC_LoadAts_Test_LoadExactlyBufferLength(void)
{
    SC_AtsEntryHeader_t *Entry;
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    uint8                AtsIndex = 0;
    size_t               MsgSize1;
    size_t               MsgSize2;
    int                  BufEntrySize;
    int                  MaxBufEntries;
    int                  i, j;

    SC_InitTables();

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;

    /* Set to reach block of code starting with comment "we encountered a load exactly as long as the buffer" */
    MsgSize1      = SC_PACKET_MAX_SIZE;
    BufEntrySize  = (MsgSize1 + SC_ROUND_UP_BYTES + SC_ATS_HEADER_SIZE) / SC_BYTES_IN_WORD;
    MaxBufEntries = SC_ATS_BUFF_SIZE32 / BufEntrySize;

    for (i = 0, j = 0; i < MaxBufEntries; i++, j += BufEntrySize)
    {
        Entry                                        = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][j];
        Entry->CmdNumber                             = i + 1;
        SC_OperData.AtsCmdStatusTblAddr[AtsIndex][j] = SC_EMPTY;
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);
    }

    /* Next entry should not leave enough buffer space for an ATS command header */
    Entry                                        = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][j];
    Entry->CmdNumber                             = i++ + 1;
    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][j] = SC_EMPTY;

    /* Use the remaining buffer space to calculate the final message size */
    MsgSize2 = ((SC_ATS_BUFF_SIZE32 - SC_ATS_HDR_NOPKT_WORDS - j) * SC_BYTES_IN_WORD);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_LoadAts_Test_LoadExactlyBufferLength */

void SC_LoadAts_Test_CmdNotEmpty(void)
{
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    size_t               MsgSize   = sizeof(SC_NoArgsCmd_t);
    SC_AtsEntryHeader_t *Entry;
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    uint8                AtsIndex = 0;
    uint8                EntryLoc;

    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));
    memset(&AtsTable, 0, sizeof(AtsTable));

    SC_InitTables();

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber = 1;

    EntryLoc         = ((MsgSize + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD) + SC_ATS_HDR_NOPKT_WORDS;
    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][EntryLoc];
    Entry->CmdNumber = 2;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* This hook will set CmdNumber 2 SC_OperData.AtsCmdStatusTblAddr[AtsIndex][1] from SC_EMPTY to SC_LOADED */
    SC_LOADS_TEST_GetTotalMsgLengthHook_RunCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_MSG_GetSize), SC_LOADS_TEST_CFE_MSG_GetSizeHook1, NULL);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    UtAssert_UINT32_EQ(SC_AppData.AtsCmdIndexBuffer[AtsIndex][0], SC_ERROR);
    UtAssert_UINT32_EQ(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0], SC_EMPTY);
    UtAssert_UINT32_EQ(SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands, 0);
    UtAssert_UINT32_EQ(SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

} /* end SC_LoadAts_Test_CmdNotEmpty */

void SC_LoadAts_Test_InvalidIndex(void)
{
    /* Pass in invalid index */
    SC_LoadAts(SC_NUMBER_OF_ATS);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_LOADATS_INV_INDEX_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);

} /* end SC_LoadAts_Test_InvalidIndex */

void SC_BuildTimeIndexTable_Test_InvalidIndex(void)
{
    uint8 AtsIndex = SC_NUMBER_OF_ATS;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Build time index table error: invalid ATS index %%d");

    SC_InitTables();

    /* Execute the function being tested */
    SC_BuildTimeIndexTable(AtsIndex);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_BUILD_TIME_IDXTBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_BuildTimeIndexTable_Test_InvalidIndex */

void SC_Insert_Test(void)
{
    uint8 AtsIndex    = 0;
    uint8 ListLength  = 1;
    uint8 NewCmdIndex = 0;

    SC_InitTables();

    SC_AppData.AtsTimeIndexBuffer[AtsIndex][0] = 1;

    /* Execute the function being tested */
    SC_Insert(AtsIndex, NewCmdIndex, ListLength);

    /* Verify results */
    UtAssert_True(SC_AppData.AtsTimeIndexBuffer[AtsIndex][1] == SC_AppData.AtsTimeIndexBuffer[AtsIndex][0],
                  "SC_AppData.AtsTimeIndexBuffer[AtsIndex][1] == SC_AppData.AtsTimeIndexBuffer[AtsIndex][0]");
    UtAssert_True(SC_AppData.AtsTimeIndexBuffer[AtsIndex][1] == NewCmdIndex + 1,
                  "SC_AppData.AtsTimeIndexBuffer[AtsIndex][1] == NewCmdIndex");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_Insert_Test */

void SC_Insert_Test_MiddleOfList(void)
{
    uint8 AtsIndex    = 0;
    uint8 ListLength  = 1;
    uint8 NewCmdIndex = 0;

    /* Set to cause SC_CompareAbsTime to return false, in order to reach block starting with
      "new cmd will execute at same time or after this list entry" */
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, false);

    SC_InitTables();

    SC_AppData.AtsTimeIndexBuffer[AtsIndex][0] = 1;

    /* Execute the function being tested */
    SC_Insert(AtsIndex, NewCmdIndex, ListLength);

    /* Verify results */
    UtAssert_True(SC_AppData.AtsTimeIndexBuffer[AtsIndex][1] == SC_AppData.AtsTimeIndexBuffer[AtsIndex][0],
                  "SC_AppData.AtsTimeIndexBuffer[AtsIndex][1] == SC_AppData.AtsTimeIndexBuffer[AtsIndex][0]");
    UtAssert_True(SC_AppData.AtsTimeIndexBuffer[AtsIndex][1] == NewCmdIndex + 1,
                  "SC_AppData.AtsTimeIndexBuffer[AtsIndex][1] == NewCmdIndex+1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_Insert_Test_MiddleOfList */

void SC_Insert_Test_MiddleOfListCompareAbsTimeTrue(void)
{
    uint8 AtsIndex    = 0;
    uint8 ListLength  = 1;
    uint8 NewCmdIndex = 0;

    /* Set to cause SC_CompareAbsTime to return false, in order to reach block starting with
      "new cmd will execute at same time or after this list entry" */
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    SC_InitTables();

    SC_AppData.AtsTimeIndexBuffer[AtsIndex][0] = 1;

    /* Execute the function being tested */
    SC_Insert(AtsIndex, NewCmdIndex, ListLength);

    /* Verify results */
    UtAssert_True(SC_AppData.AtsTimeIndexBuffer[AtsIndex][1] == SC_AppData.AtsTimeIndexBuffer[AtsIndex][0],
                  "SC_AppData.AtsTimeIndexBuffer[AtsIndex][1] == SC_AppData.AtsTimeIndexBuffer[AtsIndex][0]");
    UtAssert_True(SC_AppData.AtsTimeIndexBuffer[AtsIndex][1] == NewCmdIndex + 1,
                  "SC_AppData.AtsTimeIndexBuffer[AtsIndex][1] == NewCmdIndex+1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_Insert_Test_MiddleOfListCompareAbsTimeTrue */

void SC_Insert_Test_InvalidIndex(void)
{
    uint8 AtsIndex    = SC_NUMBER_OF_ATS;
    uint8 ListLength  = 1;
    uint8 NewCmdIndex = 0;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "ATS insert error: invalid ATS index %%d");

    SC_InitTables();

    /* Execute the function being tested */
    SC_Insert(AtsIndex, NewCmdIndex, ListLength);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_INSERTATS_INV_INDEX_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_Insert_Test_InvalidIndex */

void SC_InitAtsTables_Test_InvalidIndex(void)
{
    uint8 AtsIndex = SC_NUMBER_OF_ATS;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "ATS table init error: invalid ATS index %%d");

    /* Execute the function being tested */
    SC_InitAtsTables(AtsIndex);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_INIT_ATSTBL_INV_INDEX_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_InitAtsTables_Test_InvalidIndex */

void SC_ValidateAts_Test(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    int16                Result;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];

    SC_InitTables();

    SC_OperData.AtsTblAddr[AtsIndex] = &AtsTable[0];
    Entry                            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber                 = 0;

    /* Execute the function being tested */
    Result = SC_ValidateAts((uint16 *)(SC_OperData.AtsTblAddr[AtsIndex]));

    /* Verify results */
    UtAssert_True(Result == SC_ERROR, "Result == SC_ERROR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ValidateAts_Test */

void SC_ValidateAppend_Test(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    int16                Result;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];

    SC_InitTables();

    SC_OperData.AtsTblAddr[AtsIndex] = &AtsTable[0];
    Entry                            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber                 = 0;

    /* Execute the function being tested */
    Result = SC_ValidateAppend((SC_OperData.AtsTblAddr[AtsIndex]));

    /* Verify results */
    UtAssert_True(Result == SC_ERROR, "Result == SC_ERROR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ValidateAppend_Test */

void SC_ValidateRts_Test(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    int16                Result;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_INVALID_MSG_ID;
    size_t               MsgSize   = SC_PACKET_MIN_SIZE;

    SC_InitTables();

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    Entry                            = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag                   = 1;

    /* The MsgId and MsgSize are here to satisfy TSF */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_ValidateRts((uint16 *)(SC_OperData.RtsTblAddr[RtsIndex]));

    /* Verify results */
    UtAssert_True(Result == SC_ERROR, "Result == SC_ERROR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ValidateRts_Test */

void SC_ValidateRts_Test_ParseRts(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    int16                Result;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_INVALID_MSG_ID;
    size_t               MsgSize   = SC_PACKET_MIN_SIZE;

    SC_InitTables();

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    Entry                            = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag                   = 0;

    /* The MsgId and MsgSize are here to satisfy TSF */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_ValidateRts((uint16 *)(SC_OperData.RtsTblAddr[RtsIndex]));

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ValidateRts_Test_ParseRts */

void SC_LoadRts_Test_Nominal(void)
{
    uint8 AtsIndex = 0;

    SC_InitTables();

    /* Execute the function being tested */
    SC_LoadRts(AtsIndex);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_LoadRts_Test_Nominal */

void SC_LoadRts_Test_InvalidIndex(void)
{
    uint8 RtsIndex = SC_NUMBER_OF_RTS;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "RTS table init error: invalid RTS index %%d");

    /* Execute the function being tested */
    SC_LoadRts(RtsIndex);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_LOADRTS_INV_INDEX_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_LoadRts_Test_InvalidIndex */

void SC_ParseRts_Test_EndOfFile(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    int16                Result;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_INVALID_MSG_ID;
    size_t               MsgSize   = SC_PACKET_MIN_SIZE;

    SC_InitTables();

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    Entry                            = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag                   = 0;

    /* Set these to satisfy if-statement to reach line with comment "assumed end of file" */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_ParseRts(SC_OperData.RtsTblAddr[RtsIndex]);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ParseRts_Test_EndOfFile */

void SC_ParseRts_Test_InvalidMsgId(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    int16                Result;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    CFE_SB_MsgId_t       TestMsgId = CFE_SB_INVALID_MSG_ID;
    size_t               MsgSize   = SC_PACKET_MIN_SIZE;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "RTS cmd loaded with invalid MID at %%d");

    SC_InitTables();

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    Entry                            = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag                   = 1;

    /* Set to generate error message SC_RTS_INVLD_MID_ERR_EID */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_ParseRts(SC_OperData.RtsTblAddr[RtsIndex]);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_INVLD_MID_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ParseRts_Test_InvalidMsgId */

void SC_ParseRts_Test_LengthErrorTooShort(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    int16                Result;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    CFE_SB_MsgId_t       TestMsgId = SC_UT_MID_1;
    size_t               MsgSize   = 0;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "RTS cmd loaded with invalid length at %%d, len: %%d");

    SC_InitTables();

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    Entry                            = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag                   = 1;

    /* Set to generate error message SC_RTS_INVLD_MID_ERR_EID */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_ParseRts(SC_OperData.RtsTblAddr[RtsIndex]);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ParseRts_Test_LengthErrorTooShort */

void SC_ParseRts_Test_LengthErrorTooLong(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    int16                Result;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    CFE_SB_MsgId_t       TestMsgId = SC_UT_MID_1;
    size_t               MsgSize   = SC_PACKET_MAX_SIZE + 1;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "RTS cmd loaded with invalid length at %%d, len: %%d");

    SC_InitTables();

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    Entry                            = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag                   = 1;

    /* Set to generate error message SC_RTS_LEN_ERR_EID as a result of length being too long */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_ParseRts(SC_OperData.RtsTblAddr[RtsIndex]);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ParseRts_Test_LengthErrorTooLong */

void SC_ParseRts_Test_CmdRunsOffEndOfBuffer(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    int16                Result;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    CFE_SB_MsgId_t       TestMsgId = SC_UT_MID_1;
    size_t               MsgSize   = SC_PACKET_MAX_SIZE;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(&RtsTable, 0, sizeof(RtsTable));

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "RTS cmd at %%d runs off end of buffer");

    SC_InitTables();

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    Entry                            = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag                   = 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_ParseRts(SC_OperData.RtsTblAddr[RtsIndex]);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LEN_BUFFER_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ParseRts_Test_CmdRunsOffEndOfBuffer */

void SC_ParseRts_Test_CmdLengthEqualsBufferLength(void)
{
    /* Also tests the case where CmdLength is less than the buffer length */

    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;
    int16                Result;
    uint32               RtsTable[SC_RTS_BUFF_SIZE32];
    CFE_SB_MsgId_t       TestMsgId = SC_UT_MID_1;
    size_t               MsgSize1;
    size_t               MsgSize2;
    int                  BufEntrySize;

    memset(&RtsTable, 0, sizeof(RtsTable));

    SC_InitTables();

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];
    Entry                            = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag                   = 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    MsgSize1     = SC_PACKET_MAX_SIZE;
    BufEntrySize = (MsgSize1 + SC_ROUND_UP_BYTES + SC_RTS_HEADER_SIZE) / SC_BYTES_IN_WORD;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);

    /* Use the remaining buffer space to calculate the final message size */
    MsgSize2 = ((SC_RTS_BUFF_SIZE32 - SC_RTS_HEADER_SIZE / SC_BYTES_IN_WORD - BufEntrySize) * SC_BYTES_IN_WORD);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Execute the function being tested */
    Result = SC_ParseRts(SC_OperData.RtsTblAddr[RtsIndex]);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ParseRts_Test_CmdLengthEqualsBufferLength */

void SC_ParseRts_Test_CmdDoesNotFitBufferEmpty(void)
{
    uint8          RtsIndex = 0;
    uint32         RtsTable[SC_RTS_BUFF_SIZE32];
    CFE_SB_MsgId_t TestMsgId = SC_UT_MID_1;
    size_t         MsgSize1  = SC_PACKET_MAX_SIZE;
    size_t         MsgSize2;
    int            BufEntrySize;

    SC_InitTables();

    memset(&RtsTable, 0, sizeof(RtsTable));

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    /* Maximum size first command */
    BufEntrySize = (MsgSize1 + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD + SC_RTS_HDR_NOPKT_WORDS;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);

    /* All but last 32 bits for 2nd command */
    MsgSize2 = ((SC_RTS_BUFF_SIZE32 - SC_RTS_HDR_NOPKT_WORDS - BufEntrySize - 1) * SC_BYTES_IN_WORD);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(SC_ParseRts(SC_OperData.RtsTblAddr[RtsIndex]));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

} /* end SC_ParseRts_Test_CmdDoesNotFitBufferEmpty */

void SC_ParseRts_Test_CmdDoesNotFitBufferNotEmpty(void)
{
    uint8          RtsIndex = 0;
    uint32         RtsTable[SC_RTS_BUFF_SIZE32];
    CFE_SB_MsgId_t TestMsgId = SC_UT_MID_1;
    size_t         MsgSize1  = SC_PACKET_MAX_SIZE;
    size_t         MsgSize2;
    int            BufEntrySize;
    int32          strCmpResult;
    char           ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    /* Filling the table so it is considered used wherever checked */
    memset(&RtsTable, 0xff, sizeof(RtsTable));

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "RTS cmd loaded won't fit in buffer at %%d");

    SC_InitTables();

    SC_OperData.RtsTblAddr[RtsIndex] = &RtsTable[0];

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    /* Maximum size first command */
    BufEntrySize = (MsgSize1 + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD + SC_RTS_HDR_NOPKT_WORDS;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);

    /* All but last 32 bits for 2nd command */
    MsgSize2 = ((SC_RTS_BUFF_SIZE32 - SC_RTS_HDR_NOPKT_WORDS - BufEntrySize - 1) * SC_BYTES_IN_WORD);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_ParseRts(SC_OperData.RtsTblAddr[RtsIndex]));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LEN_TOO_LONG_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end SC_ParseRts_Test_CmdDoesNotFitBufferNotEmpty */

void SC_UpdateAppend_Test_Nominal(void)
{
    /* Also tests the case where CmdLength is less than the buffer length */

    SC_AtsEntryHeader_t *Entry;
    uint8                EntryIndex = 0;
    uint32               AtsAppendTable[SC_APPEND_BUFF_SIZE32];
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Update Append ATS Table: load count = %%d, command count = %%d, byte count = %%d");

    SC_InitTables();

    memset(&AtsAppendTable, 0, sizeof(AtsAppendTable));
    SC_OperData.AppendTblAddr = &AtsAppendTable[0];
    Entry                     = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr[EntryIndex];
    Entry->CmdNumber          = 1;

    /* Set to reach code block starting with comment "Compute buffer index for next Append ATS table entry" */
    MsgSize = 50;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    SC_UpdateAppend();

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AppendLoadCount == 1, "SC_OperData.HkPacket.AppendLoadCount == 1");
    UtAssert_True(SC_OperData.HkPacket.AppendEntryCount == 1, "SC_OperData.HkPacket.AppendEntryCount == 1");
    UtAssert_True(SC_AppData.AppendWordCount == 15, "SC_AppData.AppendWordCount == 15");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_UpdateAppend_Test_Nominal */

void SC_UpdateAppend_Test_CmdDoesNotFitBuffer(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                EntryIndex = 0;
    uint32               AtsAppendTable[SC_APPEND_BUFF_SIZE32];
    size_t               MsgSize;
    int                  BufEntrySize;
    int                  MaxBufEntries;
    int                  j;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Update Append ATS Table: load count = %%d, command count = %%d, byte count = %%d");

    SC_InitTables();

    memset(&AtsAppendTable, 0, sizeof(AtsAppendTable));
    SC_OperData.AppendTblAddr = &AtsAppendTable[0];

    /* Set to reach code block starting with comment "Compute buffer index for next Append ATS table entry" */
    MsgSize       = SC_PACKET_MAX_SIZE;
    BufEntrySize  = (MsgSize + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD + SC_ATS_HDR_NOPKT_WORDS;
    MaxBufEntries = SC_APPEND_BUFF_SIZE32 / BufEntrySize;

    for (EntryIndex = 0, j = 0; EntryIndex <= MaxBufEntries; EntryIndex++, j += BufEntrySize)
    {
        Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr[j];
        Entry->CmdNumber = EntryIndex + 1;
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    }

    /* Execute the function being tested */
    SC_UpdateAppend();

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AppendLoadCount == 1, "SC_OperData.HkPacket.AppendLoadCount == 1");
    UtAssert_True(SC_OperData.HkPacket.AppendEntryCount == 30, "SC_OperData.HkPacket.AppendEntryCount == 30");
    UtAssert_True(SC_AppData.AppendWordCount == 1980, "SC_AppData.AppendWordCount == 1980");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_UpdateAppend_Test_CmdDoesNotFitBuffer */

void SC_UpdateAppend_Test_InvalidCmdLengthTooLow(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                EntryIndex = 0;
    uint32               AtsAppendTable[SC_APPEND_BUFF_SIZE32];
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Update Append ATS Table: load count = %%d, command count = %%d, byte count = %%d");

    SC_InitTables();

    memset(&AtsAppendTable, 0, sizeof(AtsAppendTable));
    SC_OperData.AppendTblAddr = &AtsAppendTable[0];
    Entry                     = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr[EntryIndex];
    Entry->CmdNumber          = 1;

    /* Set to satisfy condition "(CommandBytes < SC_PACKET_MIN_SIZE)" */
    MsgSize = SC_PACKET_MIN_SIZE - 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    SC_UpdateAppend();

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AppendLoadCount == 1, "SC_OperData.HkPacket.AppendLoadCount == 1");
    UtAssert_True(SC_OperData.HkPacket.AppendEntryCount == 0, "SC_OperData.HkPacket.AppendEntryCount == 0");
    UtAssert_True(SC_AppData.AppendWordCount == 0, "SC_AppData.AppendWordCount == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_UpdateAppend_Test_InvalidCmdLengthTooLow */

void SC_UpdateAppend_Test_InvalidCmdLengthTooHigh(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                EntryIndex = 0;
    uint32               AtsAppendTable[SC_APPEND_BUFF_SIZE32];
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Update Append ATS Table: load count = %%d, command count = %%d, byte count = %%d");

    SC_InitTables();

    memset(&AtsAppendTable, 0, sizeof(AtsAppendTable));
    SC_OperData.AppendTblAddr = &AtsAppendTable[0];
    Entry                     = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr[EntryIndex];
    Entry->CmdNumber          = 1;

    /* Set to satisfy condition "(CommandBytes > SC_PACKET_MAX_SIZE)" */
    MsgSize = SC_PACKET_MAX_SIZE * 2;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    SC_UpdateAppend();

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AppendLoadCount == 1, "SC_OperData.HkPacket.AppendLoadCount == 1");
    UtAssert_True(SC_OperData.HkPacket.AppendEntryCount == 0, "SC_OperData.HkPacket.AppendEntryCount == 0");
    UtAssert_True(SC_AppData.AppendWordCount == 0, "SC_AppData.AppendWordCount == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_UpdateAppend_Test_InvalidCmdLengthTooHigh */

void SC_UpdateAppend_Test_EndOfBuffer(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                EntryIndex = 0;
    uint32               AtsAppendTable[SC_APPEND_BUFF_SIZE32];
    size_t               MsgSize1;
    size_t               MsgSize2;
    int                  BufEntrySize;
    int                  MaxBufEntries;
    int                  j;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Update Append ATS Table: load count = %%d, command count = %%d, byte count = %%d");

    SC_InitTables();

    memset(&AtsAppendTable, 0, sizeof(AtsAppendTable));
    SC_OperData.AppendTblAddr = &AtsAppendTable[0];

    /* Cause condition to be met: "(EntryIndex >= SC_APPEND_BUFF_SIZE)" */
    MsgSize1      = SC_PACKET_MAX_SIZE;
    BufEntrySize  = (MsgSize1 + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD + SC_ATS_HDR_NOPKT_WORDS;
    MaxBufEntries = SC_APPEND_BUFF_SIZE32 / BufEntrySize;

    for (EntryIndex = 0, j = 0; EntryIndex <= MaxBufEntries; EntryIndex++, j += BufEntrySize)
    {
        Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr[j];
        Entry->CmdNumber = EntryIndex + 1;
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize1, sizeof(MsgSize1), false);
        if (EntryIndex == (MaxBufEntries - 1))
        {
            MsgSize2 = 72;
            UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize2, sizeof(MsgSize2), false);
        }
    }

    /* Execute the function being tested */
    SC_UpdateAppend();

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AppendLoadCount == 1, "SC_OperData.HkPacket.AppendLoadCount == 1");
    UtAssert_True(SC_OperData.HkPacket.AppendEntryCount == 31, "SC_OperData.HkPacket.AppendEntryCount == 31");
    UtAssert_True(SC_AppData.AppendWordCount == 2000, "SC_AppData.AppendWordCount == 2000");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_UpdateAppend_Test_EndOfBuffer */

void SC_UpdateAppend_Test_CmdNumberZero(void)
{
    /* Also tests the case where CmdLength is less than the buffer length */

    SC_AtsEntryHeader_t *Entry;
    uint8                EntryIndex = 0;
    uint32               AtsAppendTable[SC_APPEND_BUFF_SIZE32];
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Update Append ATS Table: load count = %%d, command count = %%d, byte count = %%d");

    SC_InitTables();

    memset(&AtsAppendTable, 0, sizeof(AtsAppendTable));
    SC_OperData.AppendTblAddr = &AtsAppendTable[0];
    Entry                     = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr[EntryIndex];
    Entry->CmdNumber          = 0;

    /* Cause condition to be met: "(Entry->CmdNumber == 0)" */
    MsgSize = 50;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    SC_UpdateAppend();

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AppendLoadCount == 1, "SC_OperData.HkPacket.AppendLoadCount == 1");
    UtAssert_True(SC_OperData.HkPacket.AppendEntryCount == 0, "SC_OperData.HkPacket.AppendEntryCount == 0");
    UtAssert_True(SC_AppData.AppendWordCount == 0, "SC_AppData.AppendWordCount == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_UpdateAppend_Test_CmdNumberZero */

void SC_UpdateAppend_Test_CmdNumberTooHigh(void)
{
    /* Also tests the case where CmdLength is less than the buffer length */

    SC_AtsEntryHeader_t *Entry;
    uint8                EntryIndex = 0;
    uint32               AtsAppendTable[SC_APPEND_BUFF_SIZE32];
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Update Append ATS Table: load count = %%d, command count = %%d, byte count = %%d");

    SC_InitTables();

    memset(&AtsAppendTable, 0, sizeof(AtsAppendTable));
    SC_OperData.AppendTblAddr = &AtsAppendTable[0];
    Entry                     = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr[EntryIndex];
    Entry->CmdNumber          = SC_MAX_ATS_CMDS + 1;
    ;

    /* Cause condition to be met: "(Entry->CmdNumber > SC_MAX_ATS_CMDS)" */
    MsgSize = 50;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    SC_UpdateAppend();

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.AppendLoadCount == 1, "SC_OperData.HkPacket.AppendLoadCount == 1");
    UtAssert_True(SC_OperData.HkPacket.AppendEntryCount == 0, "SC_OperData.HkPacket.AppendEntryCount == 0");
    UtAssert_True(SC_AppData.AppendWordCount == 0, "SC_AppData.AppendWordCount == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_UpdateAppend_Test_CmdNumberTooHigh */

void SC_ProcessAppend_Test(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsAppendTable[SC_APPEND_BUFF_SIZE32];
    size_t               MsgSize;

    SC_InitTables();

    memset(&AtsTable, 0, sizeof(AtsTable));
    memset(&AtsAppendTable, 0, sizeof(AtsAppendTable));
    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;
    SC_OperData.AtsCtrlBlckAddr               = &SC_APP_TEST_GlobalAtsCtrlBlck;
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AppendTblAddr                 = &AtsAppendTable[0];

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr[AtsIndex];
    Entry->CmdNumber = 1;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber = 1;

    SC_AppData.AppendWordCount            = 1;
    SC_OperData.HkPacket.AppendEntryCount = 1;

    SC_AppData.AtsCmdIndexBuffer[0][0] = 0;

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] = SC_EMPTY;
    SC_OperData.AtsCtrlBlckAddr->AtpState        = SC_EXECUTING;
    SC_OperData.AtsCtrlBlckAddr->AtsNumber       = 1;

    MsgSize = SC_PACKET_MIN_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* restart ATS */
    UT_SetDeferredRetcode(UT_KEY(SC_BeginAts), 1, true);

    /* Execute the function being tested */
    SC_ProcessAppend(AtsIndex);

    /* Verify results */
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 1,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 1");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 1,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 1");
    UtAssert_True(SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == 0, "SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == 0");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_LOADED,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_LOADED");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING,
                  "SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ProcessAppend_Test */

void SC_ProcessAppend_Test_CmdLoaded(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsAppendTable[SC_APPEND_BUFF_SIZE32];
    size_t               MsgSize;

    SC_InitTables();

    memset(&AtsTable, 0, sizeof(AtsTable));
    memset(&AtsAppendTable, 0, sizeof(AtsAppendTable));
    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;
    SC_OperData.AtsCtrlBlckAddr               = &SC_APP_TEST_GlobalAtsCtrlBlck;
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AppendTblAddr                 = &AtsAppendTable[0];

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr[AtsIndex];
    Entry->CmdNumber = 1;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber = 1;

    SC_AppData.AppendWordCount            = 1;
    SC_OperData.HkPacket.AppendEntryCount = 1;

    SC_AppData.AtsCmdIndexBuffer[0][0] = 0;

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] = SC_LOADED;
    SC_OperData.AtsCtrlBlckAddr->AtpState        = SC_EXECUTING;
    SC_OperData.AtsCtrlBlckAddr->AtsNumber       = 1;

    MsgSize = SC_PACKET_MIN_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    SC_ProcessAppend(AtsIndex);

    /* Verify results */
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 1,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 1");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 0");
    UtAssert_True(SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == 0, "SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == 0");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_LOADED,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_LOADED");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING,
                  "SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ProcessAppend_Test_CmdLoaded */

void SC_ProcessAppend_Test_NotExecuting(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsAppendTable[SC_APPEND_BUFF_SIZE32];
    size_t               MsgSize;

    SC_InitTables();

    memset(&AtsTable, 0, sizeof(AtsTable));
    memset(&AtsAppendTable, 0, sizeof(AtsAppendTable));
    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;
    SC_OperData.AtsCtrlBlckAddr               = &SC_APP_TEST_GlobalAtsCtrlBlck;
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AppendTblAddr                 = &AtsAppendTable[0];

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr[AtsIndex];
    Entry->CmdNumber = 1;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber = 1;

    SC_AppData.AppendWordCount            = 1;
    SC_OperData.HkPacket.AppendEntryCount = 1;

    SC_AppData.AtsCmdIndexBuffer[0][0] = 0;

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] = SC_EMPTY;
    SC_OperData.AtsCtrlBlckAddr->AtpState        = SC_IDLE;
    SC_OperData.AtsCtrlBlckAddr->AtsNumber       = 1;

    MsgSize = SC_PACKET_MIN_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    SC_ProcessAppend(AtsIndex);

    /* Verify results */
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 1,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 1");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 1,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 1");
    UtAssert_True(SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == 0, "SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == 0");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_LOADED,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_LOADED");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_IDLE, "SC_OperData.AtsCtrlBlckAddr->AtpState = SC_IDLE");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ProcessAppend_Test_NotExecuting */

void SC_ProcessAppend_Test_AtsNumber(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    SC_AtsInfoTable_t    AtsInfoTbl;
    uint32               AtsAppendTable[SC_APPEND_BUFF_SIZE32];
    size_t               MsgSize;

    SC_InitTables();

    memset(&AtsTable, 0, sizeof(AtsTable));
    memset(&AtsAppendTable, 0, sizeof(AtsAppendTable));
    memset(&AtsInfoTbl, 0, sizeof(AtsInfoTbl));

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex] = &SC_APP_TEST_GlobalAtsCmdStatusTbl[0];
    SC_OperData.AtsInfoTblAddr                = &AtsInfoTbl;
    SC_OperData.AtsCtrlBlckAddr               = &SC_APP_TEST_GlobalAtsCtrlBlck;
    SC_OperData.AtsTblAddr[AtsIndex]          = &AtsTable[0];
    SC_OperData.AppendTblAddr                 = &AtsAppendTable[0];

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AppendTblAddr[AtsIndex];
    Entry->CmdNumber = 1;

    Entry            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber = 1;

    SC_AppData.AppendWordCount            = 1;
    SC_OperData.HkPacket.AppendEntryCount = 1;

    SC_AppData.AtsCmdIndexBuffer[0][0] = 0;

    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] = SC_EMPTY;
    SC_OperData.AtsCtrlBlckAddr->AtpState        = SC_EXECUTING;
    SC_OperData.AtsCtrlBlckAddr->AtsNumber       = 0;

    MsgSize = SC_PACKET_MIN_SIZE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    SC_ProcessAppend(AtsIndex);

    /* Verify results */
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 1,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize == 1");
    UtAssert_True(SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 1,
                  "SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands == 1");
    UtAssert_True(SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == 0, "SC_AppData.AtsCmdIndexBuffer[AtsIndex][0] == 0");
    UtAssert_True(SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_LOADED,
                  "SC_OperData.AtsCmdStatusTblAddr[AtsIndex][0] == SC_LOADED");
    UtAssert_True(SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING,
                  "SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ProcessAppend_Test_AtsNumber */

void SC_ProcessAppend_Test_InvalidIndex(void)
{
    uint8 AtsIndex = SC_NUMBER_OF_ATS;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ATS process append error: invalid ATS index %%d");

    /* Execute the function being tested */
    SC_ProcessAppend(AtsIndex);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_PROCESS_APPEND_INV_INDEX_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_ProcessAppend_Test_InvalidIndex */

void SC_VerifyAtsTable_Test_Nominal(void)
{
    SC_AtsEntryHeader_t *Entry1;
    SC_AtsEntryHeader_t *Entry2;
    uint8                AtsIndex = 0;
    int16                Result;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(&AtsTable, 0, sizeof(AtsTable));

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Verify ATS Table: command count = %%d, byte count = %%d");

    SC_InitTables();

    SC_OperData.AtsTblAddr[AtsIndex] = &AtsTable[0];

    Entry1            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry1->CmdNumber = 1;

    Entry2            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][128];
    Entry2->CmdNumber = 0;

    /* Set to call to SC_VerifyAtsEntry will return SC_PACKET_MAX_SIZE, which will cause execution of
     * code block starting with comment "Result is size (in words) of this entry" */
    MsgSize = SC_PACKET_MAX_SIZE;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_VerifyAtsTable((SC_OperData.AtsTblAddr[AtsIndex]), SC_ATS_BUFF_SIZE);

    /* Verify results */

    /* Test element 1.  Note: element 0 is modified in call to SC_VerifyAtsEntry and so it does not need to be verified
     * in this test */
    UtAssert_True(SC_OperData.AtsDupTestArray[1] == SC_DUP_TEST_UNUSED,
                  "SC_OperData.AtsDupTestArray[1] == SC_DUP_TEST_UNUSED");

    /* Test middle element.  Note: element 0 is modified in call to SC_VerifyAtsEntry */
    UtAssert_True(SC_OperData.AtsDupTestArray[SC_MAX_ATS_CMDS / 2] == SC_DUP_TEST_UNUSED,
                  "SC_OperData.AtsDupTestArray[SC_MAX_ATS_CMDS / 2] == SC_DUP_TEST_UNUSED");

    /* Test last element */
    UtAssert_True(SC_OperData.AtsDupTestArray[SC_MAX_ATS_CMDS - 1] == SC_DUP_TEST_UNUSED,
                  "SC_OperData.AtsDupTestArray[SC_MAX_ATS_CMDS - 1] == SC_DUP_TEST_UNUSED");

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyAtsTable_Test_Nominal */

void SC_VerifyAtsTable_Test_InvalidEntry(void)
{
    SC_AtsEntryHeader_t *Entry1;
    SC_AtsEntryHeader_t *Entry2;
    uint8                AtsIndex = 0;
    int16                Result;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];

    SC_InitTables();

    SC_OperData.AtsTblAddr[AtsIndex] = &AtsTable[0];

    Entry1            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry1->CmdNumber = 5000;

    Entry2            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][128];
    Entry2->CmdNumber = 0;

    /* Execute the function being tested */
    Result = SC_VerifyAtsTable((SC_OperData.AtsTblAddr[AtsIndex]), SC_ATS_BUFF_SIZE);

    /* Verify results */
    UtAssert_True(Result == SC_ERROR, "Result == SC_ERROR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyAtsTable_Test_InvalidEntry */

void SC_VerifyAtsTable_Test_EmptyTable(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    int16                Result;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Verify ATS Table error: table is empty");

    SC_InitTables();

    SC_OperData.AtsTblAddr[AtsIndex] = &AtsTable[0];
    Entry                            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber                 = 0;

    /* Execute the function being tested */
    Result = SC_VerifyAtsTable((SC_OperData.AtsTblAddr[AtsIndex]), SC_ATS_BUFF_SIZE);

    /* Verify results */
    UtAssert_True(Result == SC_ERROR, "Result == SC_ERROR");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_MPT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyAtsTable_Test_EmptyTable */

void SC_VerifyAtsEntry_Test_Nominal(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    int16                Result;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    size_t               MsgSize;

    SC_InitTables();

    SC_OperData.AtsTblAddr[AtsIndex] = &AtsTable[0];
    Entry                            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber                 = 1;

    SC_OperData.AtsDupTestArray[0] = SC_DUP_TEST_UNUSED;

    /* Set to reach code block starting with comment "Compute length (in words) for this ATS table entry" */
    MsgSize = SC_PACKET_MAX_SIZE;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_VerifyAtsEntry((SC_OperData.AtsTblAddr[AtsIndex]), AtsIndex, SC_ATS_BUFF_SIZE);

    /* Verify results */
    UtAssert_True(Result == SC_ATS_HDR_NOPKT_WORDS + ((SC_PACKET_MAX_SIZE + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD),
                  "Result == SC_ATS_HDR_NOPKT_WORDS + (SC_PACKET_MAX_SIZE + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD");
    UtAssert_True(SC_OperData.AtsDupTestArray[0] == 0, "SC_OperData.AtsDupTestArray[0] == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyAtsEntry_Test_Nominal */

void SC_VerifyAtsEntry_Test_EndOfBuffer(void)
{
    uint16 AtsIndex = 10000;
    int16  Result;

    SC_InitTables();

    /* Execute the function being tested */
    Result = SC_VerifyAtsEntry((SC_OperData.AtsTblAddr[0]), AtsIndex, SC_ATS_BUFF_SIZE);

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyAtsEntry_Test_EndOfBuffer */

void SC_VerifyAtsEntry_Test_InvalidCmdNumber(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    int16                Result;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Verify ATS Table error: invalid command number: buf index = %%d, cmd num = %%d");

    SC_InitTables();

    SC_OperData.AtsTblAddr[AtsIndex] = &AtsTable[0];
    Entry                            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber                 = 5000;

    /* Execute the function being tested */
    Result = SC_VerifyAtsEntry((SC_OperData.AtsTblAddr[AtsIndex]), AtsIndex, SC_ATS_BUFF_SIZE);

    /* Verify results */
    UtAssert_True(Result == SC_ERROR, "Result == SC_ERROR");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_NUM_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyAtsEntry_Test_InvalidCmdNumber */

void SC_VerifyAtsEntry_Test_BufferFull(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    int16                Result;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Verify ATS Table error: buffer full: buf index = %%d, cmd num = %%d, buf words = %%d");

    SC_InitTables();

    SC_OperData.AtsTblAddr[AtsIndex] = &AtsTable[0];
    Entry                            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber                 = 1;

    /* Execute the function being tested */
    Result = SC_VerifyAtsEntry((SC_OperData.AtsTblAddr[AtsIndex]), AtsIndex, 2);

    /* Verify results */
    UtAssert_True(Result == SC_ERROR, "Result == SC_ERROR");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_END_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyAtsEntry_Test_BufferFull */

void SC_VerifyAtsEntry_Test_InvalidCmdLengthTooLow(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    int16                Result;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Verify ATS Table error: invalid length: buf index = %%d, cmd num = %%d, pkt len = %%d");

    SC_InitTables();

    SC_OperData.AtsTblAddr[AtsIndex] = &AtsTable[0];
    Entry                            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber                 = 1;

    /* Set to generate error message SC_VERIFY_ATS_PKT_ERR_EID by satisfying condition "(CommandBytes <
     * SC_PACKET_MIN_SIZE)" */
    MsgSize = SC_PACKET_MIN_SIZE - 1;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_VerifyAtsEntry((SC_OperData.AtsTblAddr[AtsIndex]), AtsIndex, SC_ATS_BUFF_SIZE);

    /* Verify results */
    UtAssert_True(Result == SC_ERROR, "Result == SC_ERROR");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_PKT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyAtsEntry_Test_InvalidCmdLengthTooLow */

void SC_VerifyAtsEntry_Test_InvalidCmdLengthTooHigh(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    int16                Result;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Verify ATS Table error: invalid length: buf index = %%d, cmd num = %%d, pkt len = %%d");

    SC_InitTables();

    SC_OperData.AtsTblAddr[AtsIndex] = &AtsTable[0];
    Entry                            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber                 = 1;

    /* Set to generate error message SC_VERIFY_ATS_PKT_ERR_EID by satisfying condition "(CommandBytes <
     * SC_PACKET_MIN_SIZE)" */
    MsgSize = SC_PACKET_MAX_SIZE * 2;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_VerifyAtsEntry((SC_OperData.AtsTblAddr[AtsIndex]), AtsIndex, SC_ATS_BUFF_SIZE);

    /* Verify results */
    UtAssert_True(Result == SC_ERROR, "Result == SC_ERROR");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_PKT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyAtsEntry_Test_InvalidCmdLengthTooHigh */

void SC_VerifyAtsEntry_Test_BufferOverflow(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    int16                Result;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Verify ATS Table error: buffer overflow: buf index = %%d, cmd num = %%d, pkt len = %%d");

    SC_InitTables();

    SC_OperData.AtsTblAddr[AtsIndex] = &AtsTable[0];
    Entry                            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber                 = 1;

    /* Set to generate error message SC_VERIFY_ATS_BUF_ERR_EID */
    MsgSize = SC_PACKET_MAX_SIZE;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_VerifyAtsEntry((SC_OperData.AtsTblAddr[AtsIndex]), AtsIndex, 20);

    /* Verify results */
    UtAssert_True(Result == SC_ERROR, "Result == SC_ERROR");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_BUF_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyAtsEntry_Test_BufferOverflow */

void SC_VerifyAtsEntry_Test_DuplicateCmdNumber(void)
{
    SC_AtsEntryHeader_t *Entry;
    uint8                AtsIndex = 0;
    int16                Result;
    uint32               AtsTable[SC_ATS_BUFF_SIZE32];
    size_t               MsgSize;
    int32                strCmpResult;
    char                 ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Verify ATS Table error: dup cmd number: buf index = %%d, cmd num = %%d, dup index = %%d");

    SC_InitTables();

    SC_OperData.AtsTblAddr[AtsIndex] = &AtsTable[0];
    Entry                            = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][0];
    Entry->CmdNumber                 = 1;

    /* Set to generate error message SC_VERIFY_ATS_DUP_ERR_EID */
    MsgSize = SC_PACKET_MAX_SIZE;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    SC_OperData.AtsDupTestArray[0] = 99;

    /* Execute the function being tested */
    Result = SC_VerifyAtsEntry((SC_OperData.AtsTblAddr[AtsIndex]), AtsIndex, SC_ATS_BUFF_SIZE);

    /* Verify results */
    UtAssert_True(Result == SC_ERROR, "Result == SC_ERROR");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_DUP_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyAtsEntry_Test_DuplicateCmdNumber */

void UtTest_Setup(void)
{
    UtTest_Add(SC_LoadAts_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_Nominal");
    UtTest_Add(SC_LoadAts_Test_CmdRunOffEndOfBuffer, SC_Test_Setup, SC_Test_TearDown,
               "SC_LoadAts_Test_CmdRunOffEndOfBuffer");
    UtTest_Add(SC_LoadAts_Test_CmdLengthInvalid, SC_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_CmdLengthInvalid");
    UtTest_Add(SC_LoadAts_Test_CmdLengthZero, SC_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_CmdLengthZero");
    UtTest_Add(SC_LoadAts_Test_CmdNumberInvalid, SC_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_CmdNumberInvalid");
    UtTest_Add(SC_LoadAts_Test_EndOfLoadReached, SC_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_EndOfLoadReached");
    UtTest_Add(SC_LoadAts_Test_AtsBufferTooSmall, SC_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_AtsBufferTooSmall");
    UtTest_Add(SC_LoadAts_Test_AtsEntryOverflow, SC_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_AtsEntryOverflow");
    UtTest_Add(SC_LoadAts_Test_LoadExactlyBufferLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_LoadAts_Test_LoadExactlyBufferLength");
    UtTest_Add(SC_LoadAts_Test_CmdNotEmpty, SC_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_CmdNotEmpty");
    UtTest_Add(SC_LoadAts_Test_InvalidIndex, SC_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_InvalidIndex");

    UtTest_Add(SC_BuildTimeIndexTable_Test_InvalidIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_BuildTimeIndexTable_Test_InvalidIndex");
    UtTest_Add(SC_Insert_Test, SC_Test_Setup, SC_Test_TearDown, "SC_Insert_Test");
    UtTest_Add(SC_Insert_Test_MiddleOfList, SC_Test_Setup, SC_Test_TearDown, "SC_Insert_Test_MiddleOfList");
    UtTest_Add(SC_Insert_Test_MiddleOfListCompareAbsTimeTrue, SC_Test_Setup, SC_Test_TearDown,
               "SC_Insert_Test_MiddleOfListCompareAbsTimeTrue");
    UtTest_Add(SC_Insert_Test_InvalidIndex, SC_Test_Setup, SC_Test_TearDown, "SC_Insert_Test_InvalidIndex");
    UtTest_Add(SC_InitAtsTables_Test_InvalidIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_InitAtsTables_Test_InvalidIndex");
    UtTest_Add(SC_ValidateAts_Test, SC_Test_Setup, SC_Test_TearDown, "SC_ValidateAts_Test");
    UtTest_Add(SC_ValidateAppend_Test, SC_Test_Setup, SC_Test_TearDown, "SC_ValidateAppend_Test");
    UtTest_Add(SC_ValidateRts_Test, SC_Test_Setup, SC_Test_TearDown, "SC_ValidateRts_Test");
    UtTest_Add(SC_ValidateRts_Test_ParseRts, SC_Test_Setup, SC_Test_TearDown, "SC_ValidateRts_Test_ParseRts");
    UtTest_Add(SC_LoadRts_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_LoadRts_Test_Nominal");
    UtTest_Add(SC_LoadRts_Test_InvalidIndex, SC_Test_Setup, SC_Test_TearDown, "SC_LoadRts_Test_InvalidIndex");
    UtTest_Add(SC_ParseRts_Test_EndOfFile, SC_Test_Setup, SC_Test_TearDown, "SC_ParseRts_Test_EndOfFile");
    UtTest_Add(SC_ParseRts_Test_InvalidMsgId, SC_Test_Setup, SC_Test_TearDown, "SC_ParseRts_Test_InvalidMsgId");
    UtTest_Add(SC_ParseRts_Test_LengthErrorTooShort, SC_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_LengthErrorTooShort");
    UtTest_Add(SC_ParseRts_Test_LengthErrorTooLong, SC_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_LengthErrorTooLong");
    UtTest_Add(SC_ParseRts_Test_CmdRunsOffEndOfBuffer, SC_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_CmdRunsOffEndOfBuffer");
    UtTest_Add(SC_ParseRts_Test_CmdLengthEqualsBufferLength, SC_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_CmdLengthEqualsBufferLength");
    UtTest_Add(SC_ParseRts_Test_CmdDoesNotFitBufferEmpty, SC_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_CmdDoesNotFitBufferEmpty");
    UtTest_Add(SC_ParseRts_Test_CmdDoesNotFitBufferNotEmpty, SC_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_CmdDoesNotFitBufferNotEmpty");
    UtTest_Add(SC_UpdateAppend_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_UpdateAppend_Test_Nominal");
    UtTest_Add(SC_UpdateAppend_Test_CmdDoesNotFitBuffer, SC_Test_Setup, SC_Test_TearDown,
               "SC_UpdateAppend_Test_CmdDoesNotFitBuffer");
    UtTest_Add(SC_UpdateAppend_Test_InvalidCmdLengthTooLow, SC_Test_Setup, SC_Test_TearDown,
               "SC_UpdateAppend_Test_InvalidCmdLengthTooLow");
    UtTest_Add(SC_UpdateAppend_Test_InvalidCmdLengthTooHigh, SC_Test_Setup, SC_Test_TearDown,
               "SC_UpdateAppend_Test_InvalidCmdLengthTooHigh");
    UtTest_Add(SC_UpdateAppend_Test_EndOfBuffer, SC_Test_Setup, SC_Test_TearDown, "SC_UpdateAppend_Test_EndOfBuffer");
    UtTest_Add(SC_UpdateAppend_Test_CmdNumberZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_UpdateAppend_Test_CmdNumberZero");
    UtTest_Add(SC_UpdateAppend_Test_CmdNumberTooHigh, SC_Test_Setup, SC_Test_TearDown,
               "SC_UpdateAppend_Test_CmdNumberTooHigh");
    UtTest_Add(SC_ProcessAppend_Test, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessAppend_Test");
    UtTest_Add(SC_ProcessAppend_Test_CmdLoaded, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessAppend_Test_CmdLoaded");
    UtTest_Add(SC_ProcessAppend_Test_NotExecuting, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAppend_Test_NotExecuting");
    UtTest_Add(SC_ProcessAppend_Test_AtsNumber, SC_Test_Setup, SC_Test_TearDown, "SC_ProcessAppend_Test_AtsNumber");
    UtTest_Add(SC_ProcessAppend_Test_InvalidIndex, SC_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAppend_Test_InvalidIndex");
    UtTest_Add(SC_VerifyAtsTable_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_VerifyAtsTable_Test_Nominal");
    UtTest_Add(SC_VerifyAtsTable_Test_InvalidEntry, SC_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsTable_Test_InvalidEntry");
    UtTest_Add(SC_VerifyAtsTable_Test_EmptyTable, SC_Test_Setup, SC_Test_TearDown, "SC_VerifyAtsTable_Test_EmptyTable");
    UtTest_Add(SC_VerifyAtsEntry_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_VerifyAtsEntry_Test_Nominal");
    UtTest_Add(SC_VerifyAtsEntry_Test_EndOfBuffer, SC_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_EndOfBuffer");
    UtTest_Add(SC_VerifyAtsEntry_Test_InvalidCmdNumber, SC_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_InvalidCmdNumber");
    UtTest_Add(SC_VerifyAtsEntry_Test_BufferFull, SC_Test_Setup, SC_Test_TearDown, "SC_VerifyAtsEntry_Test_BufferFull");
    UtTest_Add(SC_VerifyAtsEntry_Test_InvalidCmdLengthTooLow, SC_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_InvalidCmdLengthTooLow");
    UtTest_Add(SC_VerifyAtsEntry_Test_InvalidCmdLengthTooHigh, SC_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_InvalidCmdLengthTooHigh");
    UtTest_Add(SC_VerifyAtsEntry_Test_BufferOverflow, SC_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_BufferOverflow");
    UtTest_Add(SC_VerifyAtsEntry_Test_DuplicateCmdNumber, SC_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_DuplicateCmdNumber");
} /* end UtTest_Setup */

/************************/
/*  End of File Comment */
/************************/
