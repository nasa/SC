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

#define UT_SC_NOMINAL_CMD_SIZE (SC_PACKET_MAX_SIZE / 2)

static const SC_SeqIndex_t UT_SEQ_INDEX_0 = SC_IDX_FROM_UINT(0);
static const SC_SeqIndex_t UT_SEQ_INDEX_1 = SC_IDX_FROM_UINT(1);

/*
 * Function Definitions
 */

void UT_SC_CmdTableSizeHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    const CFE_MSG_Message_t *MsgPtr     = UT_Hook_GetArgValueByName(Context, "MsgPtr", const CFE_MSG_Message_t *);
    CFE_MSG_Size_t *         Size       = UT_Hook_GetArgValueByName(Context, "Size", CFE_MSG_Size_t *);
    uint32 *                 StoredSize = (uint32 *)(void *)MsgPtr;

    *Size = *StoredSize;
}

void UT_SC_CmdTableMidHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    CFE_SB_MsgId_t *MsgId         = UT_Hook_GetArgValueByName(Context, "MsgId", CFE_SB_MsgId_t *);
    CFE_SB_MsgId_t *TestCaseMsgId = UserObj;

    *MsgId = *TestCaseMsgId;
}

void UT_SC_SetMsgId(CFE_SB_MsgId_t MsgId)
{
    static CFE_SB_MsgId_t TestCaseMsgId;

    TestCaseMsgId = MsgId;

    UT_SetHandlerFunction(UT_KEY(CFE_MSG_GetMsgId), UT_SC_CmdTableMidHandler, &TestCaseMsgId);
}

void UT_SC_Loads_Test_Setup(void)
{
    SC_Test_Setup();

    UT_SetHandlerFunction(UT_KEY(CFE_MSG_GetSize), UT_SC_CmdTableSizeHandler, NULL);
}

void *UT_SC_GetAtsTable(SC_AtsIndex_t AtsIndex)
{
    return SC_OperData.AtsTblAddr[SC_IDX_AS_UINT(AtsIndex)];
}

void *UT_SC_GetRtsTable(SC_RtsIndex_t RtsIndex)
{
    return SC_OperData.RtsTblAddr[SC_IDX_AS_UINT(RtsIndex)];
}

void *UT_SC_GetAppendTable(void)
{
    return SC_OperData.AppendTblAddr;
}

void UT_SC_AdvanceTailPtr(void **TailPtr, size_t HdrSize, size_t MsgSize)
{
    uint32 *U32Ptr = (uint32 *)(*TailPtr);

    /* Advance past the header, in words */
    U32Ptr += HdrSize / sizeof(uint32);

    /* Stash the size here */
    if (MsgSize < sizeof(uint32))
    {
        UtAssert_Failed("Cannot store a message with size = %lu", (unsigned long)MsgSize);
    }
    else
    {
        *U32Ptr = MsgSize;
    }

    /* Advance past the command content, in words */
    U32Ptr += (MsgSize + SC_ROUND_UP_BYTES) / sizeof(uint32);

    *TailPtr = U32Ptr;
}

SC_AtsEntryHeader_t *UT_SC_AppendSingleAtsEntry(void **TailPtr, uint16 CmdNumber, size_t MsgSize)
{
    SC_AtsEntryHeader_t *Entry = (SC_AtsEntryHeader_t *)(*TailPtr);

    UT_SC_AdvanceTailPtr(TailPtr, SC_ATS_HEADER_SIZE, MsgSize);

    Entry->CmdNumber = SC_COMMAND_NUM_C(CmdNumber);

    return Entry;
}

SC_RtsEntryHeader_t *UT_SC_AppendSingleRtsEntry(void **TailPtr, SC_RelWakeupCount_t WakeupCount, size_t MsgSize)
{
    SC_RtsEntryHeader_t *Entry = (SC_RtsEntryHeader_t *)(*TailPtr);

    UT_SC_AdvanceTailPtr(TailPtr, SC_RTS_HEADER_SIZE, MsgSize);

    Entry->WakeupCount = WakeupCount;

    return Entry;
}

SC_AtsEntryHeader_t *UT_SC_SetupSingleAtsEntry(SC_AtsIndex_t AtsIndex, uint16 CmdNumber, size_t MsgSize)
{
    void *TailPtr;

    TailPtr = UT_SC_GetAtsTable(AtsIndex);

    return UT_SC_AppendSingleAtsEntry(&TailPtr, CmdNumber, MsgSize);
}

SC_RtsEntryHeader_t *UT_SC_SetupSingleRtsEntry(SC_RtsIndex_t RtsIndex, CFE_SB_MsgId_t MsgId, SC_RelWakeupCount_t WakeupCount,
                                               size_t MsgSize)
{
    void *TailPtr;

    UT_SC_SetMsgId(MsgId);
    TailPtr = UT_SC_GetRtsTable(RtsIndex);

    return UT_SC_AppendSingleRtsEntry(&TailPtr, WakeupCount, MsgSize);
}

uint32 UT_SC_GetEntryWordCount(size_t HdrSize, size_t MsgSize)
{
    return (MsgSize + HdrSize + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD;
}

void *UT_SC_SetupFullTable(void **TailPtr, size_t HdrSize, size_t MsgSize, size_t MaxLimitWords,
                           size_t TargetEndingWord, void (*EntryInit_Callback)(void *, size_t, size_t))
{
    uint8 *HeadPtr;
    void * CurrPtr;
    size_t BufEntryWords;
    size_t LastUsableWord;
    size_t Idx;
    size_t WordsUsed;

    HeadPtr = *TailPtr;
    CurrPtr = NULL;

    /* Causes CFE_MSG_GetSize to satisfy the conditions of if-statement below comment "if the length of the command is
     * valid", but NOT the if-statement immediately after */
    BufEntryWords  = UT_SC_GetEntryWordCount(HdrSize, MsgSize);
    LastUsableWord = MaxLimitWords;

    if (TargetEndingWord != 0 && TargetEndingWord < LastUsableWord)
    {
        LastUsableWord = TargetEndingWord;
    }

    if (LastUsableWord > BufEntryWords)
    {
        LastUsableWord -= BufEntryWords;
    }
    else
    {
        LastUsableWord = 0;
    }

    /* Fill the buffer up to the last "complete" entry */
    for (Idx = 0; Idx < SC_MAX_ATS_CMDS; ++Idx)
    {
        WordsUsed = ((uint8 *)(*TailPtr) - HeadPtr) / sizeof(uint32);
        if (WordsUsed > LastUsableWord)
        {
            break;
        }

        if ((WordsUsed + BufEntryWords) > LastUsableWord && TargetEndingWord != 0)
        {
            /* This will be the last entry.  Adjust the size to hit the target. */
            BufEntryWords = TargetEndingWord - WordsUsed;
            MsgSize       = (BufEntryWords * sizeof(uint32)) - HdrSize;
        }

        CurrPtr = *TailPtr;
        UT_SC_AdvanceTailPtr(TailPtr, HdrSize, MsgSize);

        EntryInit_Callback(CurrPtr, Idx, MsgSize);
        // EntryInit(CurrPtr, SC_CommandIndexToNum(Idx), MsgSize);
    }

    /* Capture the last entry that was complete -
     * test case may want to change it to exercise end-of-buffer handling */
    return CurrPtr;
}

void UT_SC_AtsEntryInit(void *EntryPtr, size_t Idx, size_t MsgSize)
{
    SC_AtsEntryHeader_t *Entry = EntryPtr;

    Entry->CmdNumber = SC_CommandIndexToNum(SC_COMMAND_IDX_C(Idx));
}

SC_AtsEntryHeader_t *UT_SC_SetupAtsTable(SC_AtsIndex_t AtsIndex, size_t MsgSize, size_t TargetEndingWord,
                                         void **TailPtrOut)
{
    void *               TailPtr;
    SC_AtsEntryHeader_t *FinalEntry;

    TailPtr    = UT_SC_GetAtsTable(AtsIndex);
    FinalEntry = UT_SC_SetupFullTable(&TailPtr, SC_ATS_HEADER_SIZE, MsgSize, SC_ATS_BUFF_SIZE32, TargetEndingWord,
                                      UT_SC_AtsEntryInit);

    /* Capture a pointer to the _end_ of the filled data -
     * the test case might need to write additional data here */
    if (TailPtrOut != NULL)
    {
        *TailPtrOut = TailPtr;
    }

    /* Capture the last ATS entry that was complete -
     * test case may want to change it to exercise end-of-buffer handling */
    return FinalEntry;
}

void UT_SC_RtsEntryInit(void *EntryPtr, size_t Idx, size_t MsgSize)
{
    SC_RtsEntryHeader_t *Entry = EntryPtr;

    Entry->WakeupCount = 1;
}

SC_RtsEntryHeader_t *UT_SC_SetupRtsTable(SC_RtsIndex_t RtsIndex, CFE_SB_MsgId_t MsgId, size_t MsgSize,
                                         size_t TargetEndingWord, void **TailPtrOut)
{
    void *               TailPtr;
    SC_RtsEntryHeader_t *FinalEntry;

    UT_SC_SetMsgId(MsgId);
    TailPtr    = UT_SC_GetRtsTable(RtsIndex);
    FinalEntry = UT_SC_SetupFullTable(&TailPtr, SC_RTS_HEADER_SIZE, MsgSize, SC_RTS_BUFF_SIZE32, TargetEndingWord,
                                      UT_SC_RtsEntryInit);

    /* Capture a pointer to the _end_ of the filled data -
     * the test case might need to write additional data here */
    if (TailPtrOut != NULL)
    {
        *TailPtrOut = TailPtr;
    }

    /* Capture the last Rts entry that was complete -
     * test case may want to change it to exercise end-of-buffer handling */
    return FinalEntry;
}

void SC_LoadAts_Test_Nominal(void)
{
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, sizeof(SC_NoopCmd_t));

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_FIRST);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_LOADED);
    UtAssert_UINT32_EQ(AtsInfoPtr->NumberOfCommands, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_LoadAts_Test_CmdRunOffEndOfBuffer(void)
{
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    /* Set up -- Modify the final entry so it would go off the end of the table */
    UT_SC_SetupAtsTable(AtsIndex, UT_SC_NOMINAL_CMD_SIZE, SC_ATS_BUFF_SIZE32 + 1, NULL);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_INVALID);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_EMPTY);
    UtAssert_ZERO(AtsInfoPtr->NumberOfCommands);
    UtAssert_ZERO(AtsInfoPtr->AtsSize);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_LoadAts_Test_CmdLengthInvalid(void)
{
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, SC_PACKET_MAX_SIZE + 1);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_INVALID);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_EMPTY);
    UtAssert_ZERO(AtsInfoPtr->NumberOfCommands);
    UtAssert_ZERO(AtsInfoPtr->AtsSize);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_LoadAts_Test_CmdLengthZero(void)
{
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, SC_PACKET_MIN_SIZE - 1);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_INVALID);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_EMPTY);
    UtAssert_ZERO(AtsInfoPtr->NumberOfCommands);
    UtAssert_ZERO(AtsInfoPtr->AtsSize);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_LoadAts_Test_CmdNumberInvalid(void)
{
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, SC_MAX_ATS_CMDS * 2, UT_SC_NOMINAL_CMD_SIZE);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_INVALID);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_EMPTY);
    UtAssert_ZERO(AtsInfoPtr->NumberOfCommands);
    UtAssert_ZERO(AtsInfoPtr->AtsSize);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_LoadAts_Test_AtsBufferTooSmall(void)
{
    SC_AtsEntryHeader_t *         LastValidEntry;
    void *                        TailPtr;
    SC_AtsEntryHeader_t *         InvalidEntry;
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    /* Set up -- Modify the final entry so it would go off the end of the table */
    LastValidEntry =
        UT_SC_SetupAtsTable(AtsIndex, UT_SC_NOMINAL_CMD_SIZE, SC_ATS_BUFF_SIZE32 - SC_ATS_HDR_NOPKT_WORDS, &TailPtr);

    InvalidEntry = TailPtr;
    /*
     * Set up final (invalid) entry that will create error condition -
     * This is an ATS header at the very end of the ATS buffer, where there is no room
     * for a cmd message to follow it, but it has a nonzero command number
     */
    InvalidEntry->CmdNumber = SC_COMMAND_NUM_C(SC_IDNUM_AS_UINT(LastValidEntry->CmdNumber) + 1);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_INVALID);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_EMPTY);
    UtAssert_ZERO(AtsInfoPtr->NumberOfCommands);
    UtAssert_ZERO(AtsInfoPtr->AtsSize);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_LoadAts_Test_AtsEmpty(void)
{
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_INVALID);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_EMPTY);
    UtAssert_ZERO(AtsInfoPtr->NumberOfCommands);
    UtAssert_ZERO(AtsInfoPtr->AtsSize);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_LoadAts_Test_LoadExactlyBufferLength(void)
{
    SC_AtsEntryHeader_t *         Entry;
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    /* Set up -- Modify the final entry so it exactly takes up the remainder of the buffer */
    Entry = UT_SC_SetupAtsTable(AtsIndex, UT_SC_NOMINAL_CMD_SIZE, SC_ATS_BUFF_SIZE32, NULL);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    SC_Assert_IDX_VALUE(CmdOffsetRec->Offset, 0);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_LOADED);
    SC_Assert_ID_VALUE(Entry->CmdNumber, AtsInfoPtr->NumberOfCommands);
    UtAssert_UINT32_EQ(AtsInfoPtr->AtsSize, SC_ATS_BUFF_SIZE32);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_LoadAts_Test_CmdNotEmpty(void)
{
    void *                        TailPtr;
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    /* Set up a buffer that has a duplicate command number entry */
    TailPtr = UT_SC_GetAtsTable(AtsIndex);
    UT_SC_AppendSingleAtsEntry(&TailPtr, 1, UT_SC_NOMINAL_CMD_SIZE);
    UT_SC_AppendSingleAtsEntry(&TailPtr, 1, UT_SC_NOMINAL_CMD_SIZE);

    /* Execute the function being tested */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_INVALID);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_EMPTY);
    UtAssert_UINT32_EQ(AtsInfoPtr->NumberOfCommands, 0);
    UtAssert_UINT32_EQ(AtsInfoPtr->AtsSize, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_LoadAts_Test_InvalidIndex(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(SC_NUMBER_OF_ATS);

    /* Pass in invalid index */
    SC_LoadAts(AtsIndex);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_LOADATS_INV_INDEX_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_BuildTimeIndexTable_Test_InvalidIndex(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(SC_NUMBER_OF_ATS);

    /* Execute the function being tested */
    SC_BuildTimeIndexTable(AtsIndex);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_BUILD_TIME_IDXTBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_Insert_Test(void)
{
    SC_AtsIndex_t         AtsIndex    = SC_ATS_IDX_C(0);
    uint8                 ListLength  = 1;
    SC_CommandIndex_t     NewCmdIndex = SC_COMMAND_IDX_C(0);
    SC_AtsCmdNumRecord_t *AtsCmdNumRec0;
    SC_AtsCmdNumRecord_t *AtsCmdNumRec1;

    AtsCmdNumRec0 = SC_GetAtsCommandNumAtSeq(AtsIndex, UT_SEQ_INDEX_0);
    AtsCmdNumRec1 = SC_GetAtsCommandNumAtSeq(AtsIndex, UT_SEQ_INDEX_1);

    AtsCmdNumRec0->CmdNum = SC_CommandIndexToNum(NewCmdIndex);

    /* Execute the function being tested */
    SC_Insert(AtsIndex, NewCmdIndex, ListLength);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    SC_Assert_ID_EQ(AtsCmdNumRec1->CmdNum, AtsCmdNumRec0->CmdNum);
    SC_Assert_ID_EQ(AtsCmdNumRec1->CmdNum, SC_CommandIndexToNum(NewCmdIndex));
}

void SC_Insert_Test_MiddleOfList(void)
{
    SC_AtsIndex_t         AtsIndex    = SC_ATS_IDX_C(0);
    uint8                 ListLength  = 1;
    SC_CommandIndex_t     NewCmdIndex = SC_COMMAND_IDX_C(0);
    SC_AtsCmdNumRecord_t *AtsCmdNumRec0;
    SC_AtsCmdNumRecord_t *AtsCmdNumRec1;

    AtsCmdNumRec0 = SC_GetAtsCommandNumAtSeq(AtsIndex, UT_SEQ_INDEX_0);
    AtsCmdNumRec1 = SC_GetAtsCommandNumAtSeq(AtsIndex, UT_SEQ_INDEX_1);

    /* Set to cause SC_CompareAbsTime to return false, in order to reach block starting with
      "new cmd will execute at same time or after this list entry" */
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, false);

    AtsCmdNumRec0->CmdNum = SC_CommandIndexToNum(NewCmdIndex);

    /* Execute the function being tested */
    SC_Insert(AtsIndex, NewCmdIndex, ListLength);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    SC_Assert_ID_EQ(AtsCmdNumRec1->CmdNum, AtsCmdNumRec0->CmdNum);
    SC_Assert_ID_EQ(AtsCmdNumRec1->CmdNum, SC_CommandIndexToNum(NewCmdIndex));
}

void SC_Insert_Test_MiddleOfListCompareAbsTimeTrue(void)
{
    SC_AtsIndex_t         AtsIndex    = SC_ATS_IDX_C(0);
    uint8                 ListLength  = 1;
    SC_CommandIndex_t     NewCmdIndex = SC_COMMAND_IDX_C(0);
    SC_AtsCmdNumRecord_t *AtsCmdNumRec0;
    SC_AtsCmdNumRecord_t *AtsCmdNumRec1;

    AtsCmdNumRec0 = SC_GetAtsCommandNumAtSeq(AtsIndex, UT_SEQ_INDEX_0);
    AtsCmdNumRec1 = SC_GetAtsCommandNumAtSeq(AtsIndex, UT_SEQ_INDEX_1);

    /* Set to cause SC_CompareAbsTime to return false, in order to reach block starting with
      "new cmd will execute at same time or after this list entry" */
    UT_SetDeferredRetcode(UT_KEY(SC_CompareAbsTime), 1, true);

    AtsCmdNumRec0->CmdNum = SC_CommandIndexToNum(NewCmdIndex);

    /* Execute the function being tested */
    SC_Insert(AtsIndex, NewCmdIndex, ListLength);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    SC_Assert_ID_EQ(AtsCmdNumRec1->CmdNum, AtsCmdNumRec0->CmdNum);
    SC_Assert_ID_EQ(AtsCmdNumRec1->CmdNum, SC_CommandIndexToNum(NewCmdIndex));
}

void SC_Insert_Test_InvalidIndex(void)
{
    SC_AtsIndex_t     AtsIndex    = SC_ATS_IDX_C(SC_NUMBER_OF_ATS);
    uint8             ListLength  = 1;
    SC_CommandIndex_t NewCmdIndex = SC_COMMAND_IDX_C(0);

    /* Execute the function being tested */
    SC_Insert(AtsIndex, NewCmdIndex, ListLength);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_INSERTATS_INV_INDEX_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_InitAtsTables_Test_InvalidIndex(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(SC_NUMBER_OF_ATS);

    /* Execute the function being tested */
    SC_InitAtsTables(AtsIndex);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_INIT_ATSTBL_INV_INDEX_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_ValidateAts_Test(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_ValidateAts(TablePtr), SC_ERROR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_MPT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_ValidateAppend_Test(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_ValidateAppend(TablePtr), SC_ERROR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_MPT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_ValidateRts_Test(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetRtsTable(RtsIndex);

    UT_SC_SetupSingleRtsEntry(RtsIndex, CFE_SB_INVALID_MSG_ID, 1, SC_PACKET_MIN_SIZE);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_ValidateRts(TablePtr), SC_ERROR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_INVLD_MID_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_ValidateRts_Test_ParseRts(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetRtsTable(RtsIndex);

    UT_SC_SetupSingleRtsEntry(RtsIndex, CFE_SB_INVALID_MSG_ID, 0, SC_PACKET_MIN_SIZE);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_ValidateRts(TablePtr), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_LoadRts_Test_Nominal(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(0);

    UT_SC_SetMsgId(CFE_SB_INVALID_MSG_ID);
    UT_SC_GetRtsTable(RtsIndex);

    /* Execute the function being tested */
    SC_LoadRts(RtsIndex);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_LoadRts_Test_InvalidIndex(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(SC_NUMBER_OF_RTS);

    /* Execute the function being tested */
    SC_LoadRts(RtsIndex);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_LOADRTS_INV_INDEX_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_ParseRts_Test_EndOfFile(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetRtsTable(RtsIndex);

    UT_SC_SetupSingleRtsEntry(RtsIndex, CFE_SB_INVALID_MSG_ID, 0, SC_PACKET_MIN_SIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(SC_ParseRts(TablePtr));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ParseRts_Test_InvalidMsgId(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetRtsTable(RtsIndex);

    UT_SC_SetupSingleRtsEntry(RtsIndex, CFE_SB_INVALID_MSG_ID, 1, SC_PACKET_MIN_SIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_ParseRts(TablePtr));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_INVLD_MID_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_ParseRts_Test_LengthErrorTooShort(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetRtsTable(RtsIndex);

    UT_SC_SetupSingleRtsEntry(RtsIndex, SC_UT_MID_1, 1, SC_PACKET_MIN_SIZE - 1);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_ParseRts(TablePtr));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_ParseRts_Test_LengthErrorTooLong(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetRtsTable(RtsIndex);

    UT_SC_SetupSingleRtsEntry(RtsIndex, SC_UT_MID_1, 1, SC_PACKET_MAX_SIZE + 1);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_ParseRts(TablePtr));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_ParseRts_Test_CmdRunsOffEndOfBuffer(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetRtsTable(RtsIndex);

    /* Set up -- Modify the final entry so it would go off the end of the table */
    UT_SC_SetupRtsTable(RtsIndex, SC_UT_MID_1, UT_SC_NOMINAL_CMD_SIZE, SC_RTS_BUFF_SIZE32 + 1, NULL);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_ParseRts(TablePtr));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LEN_BUFFER_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_ParseRts_Test_CmdLengthEqualsBufferLength(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetRtsTable(RtsIndex);

    UT_SC_SetupRtsTable(RtsIndex, SC_UT_MID_1, UT_SC_NOMINAL_CMD_SIZE, SC_RTS_BUFF_SIZE32, NULL);

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(SC_ParseRts(TablePtr));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ParseRts_Test_CmdDoesNotFitBufferEmpty(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetRtsTable(RtsIndex);

    UT_SC_SetupRtsTable(RtsIndex, SC_UT_MID_1, UT_SC_NOMINAL_CMD_SIZE, SC_RTS_BUFF_SIZE32 - 1, NULL);

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(SC_ParseRts(TablePtr));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ParseRts_Test_CmdDoesNotFitBufferNotEmpty(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(0);
    void *        TailPtr;
    void *        TablePtr;

    TablePtr = UT_SC_GetRtsTable(RtsIndex);

    UT_SC_SetupRtsTable(RtsIndex, SC_UT_MID_1, UT_SC_NOMINAL_CMD_SIZE, SC_RTS_BUFF_SIZE32 - 1, &TailPtr);

    /* Add some nonzero data at the tail end */
    *((uint32 *)TailPtr) = 0xFFFFFFFF;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_ParseRts(TablePtr));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LEN_TOO_LONG_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_UpdateAppend_Test_Nominal(void)
{
    void *TailPtr;

    TailPtr = UT_SC_GetAppendTable();
    UT_SC_AppendSingleAtsEntry(&TailPtr, 1, UT_SC_NOMINAL_CMD_SIZE);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_UpdateAppend());

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.AppendLoadCount, 1);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.AppendEntryCount, 1);
    UtAssert_UINT32_EQ(SC_AppData.AppendWordCount, (SC_ATS_HEADER_SIZE + UT_SC_NOMINAL_CMD_SIZE) / SC_BYTES_IN_WORD);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
}

void SC_UpdateAppend_Test_CmdDoesNotFitBuffer(void)
{
    uint32 *FirstPtr;
    uint32 *LastPtr;
    void *  TailPtr;
    uint32  ExpectedCount;

    TailPtr       = UT_SC_GetAppendTable();
    FirstPtr      = TailPtr;
    LastPtr       = UT_SC_SetupFullTable(&TailPtr, SC_ATS_HEADER_SIZE, UT_SC_NOMINAL_CMD_SIZE, SC_APPEND_BUFF_SIZE32,
                                   SC_APPEND_BUFF_SIZE32 + 1, UT_SC_AtsEntryInit);
    ExpectedCount = (SC_APPEND_BUFF_SIZE32 / UT_SC_GetEntryWordCount(SC_ATS_HEADER_SIZE, UT_SC_NOMINAL_CMD_SIZE)) - 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_UpdateAppend());

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.AppendLoadCount, 1);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.AppendEntryCount, ExpectedCount);
    UtAssert_UINT32_EQ(SC_AppData.AppendWordCount, LastPtr - FirstPtr);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
}

void SC_UpdateAppend_Test_InvalidCmdLengthTooLow(void)
{
    void *TailPtr;

    TailPtr = UT_SC_GetAppendTable();
    UT_SC_AppendSingleAtsEntry(&TailPtr, 1, SC_PACKET_MIN_SIZE - 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_UpdateAppend());

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.AppendLoadCount, 1);
    UtAssert_ZERO(SC_OperData.HkPacket.Payload.AppendEntryCount);
    UtAssert_ZERO(SC_AppData.AppendWordCount);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
}

void SC_UpdateAppend_Test_InvalidCmdLengthTooHigh(void)
{
    void *TailPtr;

    TailPtr = UT_SC_GetAppendTable();
    UT_SC_AppendSingleAtsEntry(&TailPtr, 1, SC_PACKET_MAX_SIZE + 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_UpdateAppend());

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.AppendLoadCount, 1);
    UtAssert_ZERO(SC_OperData.HkPacket.Payload.AppendEntryCount);
    UtAssert_ZERO(SC_AppData.AppendWordCount);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
}

void SC_UpdateAppend_Test_EndOfBuffer(void)
{
    void * TailPtr;
    uint32 ExpectedCount;

    TailPtr = UT_SC_GetAppendTable();
    UT_SC_SetupFullTable(&TailPtr, SC_ATS_HEADER_SIZE, UT_SC_NOMINAL_CMD_SIZE, SC_APPEND_BUFF_SIZE32,
                         SC_APPEND_BUFF_SIZE32, UT_SC_AtsEntryInit);
    ExpectedCount = SC_APPEND_BUFF_SIZE32 / UT_SC_GetEntryWordCount(SC_ATS_HEADER_SIZE, UT_SC_NOMINAL_CMD_SIZE);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_UpdateAppend());

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.AppendLoadCount, 1);
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.AppendEntryCount, ExpectedCount);
    UtAssert_UINT32_EQ(SC_AppData.AppendWordCount, SC_APPEND_BUFF_SIZE32);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
}

void SC_UpdateAppend_Test_CmdNumberZero(void)
{
    void *TailPtr;

    TailPtr = UT_SC_GetAppendTable();
    UT_SC_AppendSingleAtsEntry(&TailPtr, 0, UT_SC_NOMINAL_CMD_SIZE);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_UpdateAppend());

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.AppendLoadCount, 1);
    UtAssert_ZERO(SC_OperData.HkPacket.Payload.AppendEntryCount);
    UtAssert_ZERO(SC_AppData.AppendWordCount);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
}

void SC_UpdateAppend_Test_CmdNumberTooHigh(void)
{
    void *TailPtr;

    TailPtr = UT_SC_GetAppendTable();
    UT_SC_AppendSingleAtsEntry(&TailPtr, SC_MAX_ATS_CMDS + 1, UT_SC_NOMINAL_CMD_SIZE);

    /* Execute the function being tested */
    SC_UpdateAppend();

    /* Verify results */
    UtAssert_UINT32_EQ(SC_OperData.HkPacket.Payload.AppendLoadCount, 1);
    UtAssert_ZERO(SC_OperData.HkPacket.Payload.AppendEntryCount);
    UtAssert_ZERO(SC_AppData.AppendWordCount);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_UPDATE_APPEND_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
}

void SC_ProcessAppend_Test(void)
{
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    void *                        TailPtr;
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, UT_SC_NOMINAL_CMD_SIZE);

    TailPtr = UT_SC_GetAppendTable();
    UT_SC_AppendSingleAtsEntry(&TailPtr, 1, UT_SC_NOMINAL_CMD_SIZE);

    SC_AppData.AppendWordCount                    = 1;
    SC_OperData.HkPacket.Payload.AppendEntryCount = 1;

    SC_OperData.AtsCtrlBlckAddr->AtpState   = SC_Status_EXECUTING;
    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_AtsIndexToNum(AtsIndex);

    /* restart ATS */
    UT_SetDeferredRetcode(UT_KEY(SC_BeginAts), 1, true);

    /* Execute the function being tested */
    SC_ProcessAppend(AtsIndex);

    /* Verify results */
    UtAssert_UINT32_EQ(AtsInfoPtr->AtsSize, 1);
    UtAssert_UINT32_EQ(AtsInfoPtr->NumberOfCommands, 1);
    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_FIRST);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_LOADED);
    SC_Assert_CmdStatus(SC_OperData.AtsCtrlBlckAddr->AtpState, SC_Status_EXECUTING);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAppend_Test_CmdLoaded(void)
{
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    void *                        TailPtr;
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));
    AtsInfoPtr     = SC_GetAtsInfoObject(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, UT_SC_NOMINAL_CMD_SIZE);

    TailPtr = UT_SC_GetAppendTable();
    UT_SC_AppendSingleAtsEntry(&TailPtr, 1, UT_SC_NOMINAL_CMD_SIZE);

    SC_AppData.AppendWordCount                    = 1;
    SC_OperData.HkPacket.Payload.AppendEntryCount = 1;

    StatusEntryPtr->Status                  = SC_Status_LOADED;
    SC_OperData.AtsCtrlBlckAddr->AtpState   = SC_Status_EXECUTING;
    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_AtsIndexToNum(AtsIndex);

    /* Execute the function being tested */
    SC_ProcessAppend(AtsIndex);

    /* Verify results */
    UtAssert_UINT32_EQ(AtsInfoPtr->AtsSize, 1);
    UtAssert_UINT32_EQ(AtsInfoPtr->NumberOfCommands, 0);
    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_FIRST);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_LOADED);
    SC_Assert_CmdStatus(SC_OperData.AtsCtrlBlckAddr->AtpState, SC_Status_EXECUTING);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAppend_Test_NotExecuting(void)
{
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    void *                        TailPtr;
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, UT_SC_NOMINAL_CMD_SIZE);

    TailPtr = UT_SC_GetAppendTable();
    UT_SC_AppendSingleAtsEntry(&TailPtr, 1, UT_SC_NOMINAL_CMD_SIZE);

    SC_AppData.AppendWordCount                    = 1;
    SC_OperData.HkPacket.Payload.AppendEntryCount = 1;

    SC_OperData.AtsCtrlBlckAddr->AtpState   = SC_Status_IDLE;
    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_AtsIndexToNum(AtsIndex);

    /* Execute the function being tested */
    SC_ProcessAppend(AtsIndex);

    /* Verify results */
    UtAssert_UINT32_EQ(AtsInfoPtr->AtsSize, 1);
    UtAssert_UINT32_EQ(AtsInfoPtr->NumberOfCommands, 1);
    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_FIRST);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_LOADED);
    SC_Assert_CmdStatus(SC_OperData.AtsCtrlBlckAddr->AtpState, SC_Status_IDLE);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAppend_Test_IdMismatch(void)
{
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    void *                        TailPtr;
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, UT_SC_NOMINAL_CMD_SIZE);

    TailPtr = UT_SC_GetAppendTable();
    UT_SC_AppendSingleAtsEntry(&TailPtr, 1, UT_SC_NOMINAL_CMD_SIZE);

    SC_AppData.AppendWordCount                    = 1;
    SC_OperData.HkPacket.Payload.AppendEntryCount = 1;

    SC_OperData.AtsCtrlBlckAddr->AtpState   = SC_Status_EXECUTING;
    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_AtsIndexToNum(SC_ATS_IDX_C(1));

    /* Execute the function being tested */
    SC_ProcessAppend(AtsIndex);

    /* Verify results */
    UtAssert_UINT32_EQ(AtsInfoPtr->AtsSize, 1);
    UtAssert_UINT32_EQ(AtsInfoPtr->NumberOfCommands, 1);
    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_FIRST);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_LOADED);
    SC_Assert_CmdStatus(SC_OperData.AtsCtrlBlckAddr->AtpState, SC_Status_EXECUTING);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAppend_Test_AtsNum(void)
{
    SC_AtsIndex_t                 AtsIndex = SC_ATS_IDX_C(0);
    void *                        TailPtr;
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(0));
    StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(0));

    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, UT_SC_NOMINAL_CMD_SIZE);

    TailPtr = UT_SC_GetAppendTable();
    UT_SC_AppendSingleAtsEntry(&TailPtr, 1, UT_SC_NOMINAL_CMD_SIZE);

    SC_AppData.AppendWordCount                    = 1;
    SC_OperData.HkPacket.Payload.AppendEntryCount = 1;

    SC_OperData.AtsCtrlBlckAddr->AtpState   = SC_Status_EXECUTING;
    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_AtsIndexToNum(AtsIndex);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAppend(AtsIndex));

    /* Verify results */
    UtAssert_UINT32_EQ(AtsInfoPtr->AtsSize, 1);
    UtAssert_UINT32_EQ(AtsInfoPtr->NumberOfCommands, 1);
    SC_Assert_IDX_EQ(CmdOffsetRec->Offset, SC_ENTRY_OFFSET_FIRST);
    SC_Assert_CmdStatus(StatusEntryPtr->Status, SC_Status_LOADED);
    SC_Assert_CmdStatus(SC_OperData.AtsCtrlBlckAddr->AtpState, SC_Status_EXECUTING);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_ProcessAppend_Test_InvalidIndex(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(SC_NUMBER_OF_ATS);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_ProcessAppend(AtsIndex));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_PROCESS_APPEND_INV_INDEX_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_VerifyAtsTable_Test_Nominal(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, UT_SC_NOMINAL_CMD_SIZE);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_VerifyAtsTable(TablePtr, SC_ATS_BUFF_SIZE), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Test element 1.  Note: element 0 is modified in call to SC_VerifyAtsEntry and so it does not need to be verified
     * in this test */
    UtAssert_INT32_EQ(SC_OperData.AtsDupTestArray[1], SC_DUP_TEST_UNUSED);

    /* Test middle element.  Note: element 0 is modified in call to SC_VerifyAtsEntry */
    UtAssert_INT32_EQ(SC_OperData.AtsDupTestArray[SC_MAX_ATS_CMDS / 2], SC_DUP_TEST_UNUSED);

    /* Test last element */
    UtAssert_INT32_EQ(SC_OperData.AtsDupTestArray[SC_MAX_ATS_CMDS - 1], SC_DUP_TEST_UNUSED);
}

void SC_VerifyAtsTable_Test_InvalidEntry(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, SC_MAX_ATS_CMDS + 10, UT_SC_NOMINAL_CMD_SIZE);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_VerifyAtsTable(TablePtr, SC_ATS_BUFF_SIZE), SC_ERROR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_NUM_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_VerifyAtsTable_Test_EmptyTable(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_VerifyAtsTable(TablePtr, SC_ATS_BUFF_SIZE), SC_ERROR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_MPT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_VerifyAtsEntry_Test_Nominal(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, SC_PACKET_MAX_SIZE);

    SC_OperData.AtsDupTestArray[0] = SC_DUP_TEST_UNUSED;

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_VerifyAtsEntry(TablePtr, 0, SC_ATS_BUFF_SIZE),
                      SC_ATS_HDR_NOPKT_WORDS + ((SC_PACKET_MAX_SIZE + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_ZERO(SC_OperData.AtsDupTestArray[0]);
}

void SC_VerifyAtsEntry_Test_EndOfBuffer(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, UT_SC_NOMINAL_CMD_SIZE);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_VerifyAtsEntry(TablePtr, 10000, SC_ATS_BUFF_SIZE), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_VerifyAtsEntry_Test_InvalidCmdNumber(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, SC_MAX_ATS_CMDS + 20, UT_SC_NOMINAL_CMD_SIZE);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_VerifyAtsEntry(TablePtr, 0, SC_ATS_BUFF_SIZE), SC_ERROR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_NUM_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_VerifyAtsEntry_Test_BufferFull(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, UT_SC_NOMINAL_CMD_SIZE);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_VerifyAtsEntry(TablePtr, 0, 2), SC_ERROR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_END_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_VerifyAtsEntry_Test_InvalidCmdLengthTooLow(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, SC_PACKET_MIN_SIZE - 1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_VerifyAtsEntry(TablePtr, 0, SC_ATS_BUFF_SIZE), SC_ERROR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_PKT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_VerifyAtsEntry_Test_InvalidCmdLengthTooHigh(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, SC_PACKET_MAX_SIZE * 2);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_VerifyAtsEntry(TablePtr, 0, SC_ATS_BUFF_SIZE), SC_ERROR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_PKT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_VerifyAtsEntry_Test_BufferOverflow(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, SC_PACKET_MAX_SIZE);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_VerifyAtsEntry(TablePtr, 0, 20), SC_ERROR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_BUF_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void SC_VerifyAtsEntry_Test_DuplicateCmdNumber(void)
{
    SC_AtsIndex_t AtsIndex = SC_ATS_IDX_C(0);
    void *        TablePtr;

    TablePtr = UT_SC_GetAtsTable(AtsIndex);

    UT_SC_SetupSingleAtsEntry(AtsIndex, 1, SC_PACKET_MAX_SIZE);

    /* Set to generate error message SC_VERIFY_ATS_DUP_ERR_EID */
    SC_OperData.AtsDupTestArray[0] = 99;

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_VerifyAtsEntry(TablePtr, 0, SC_ATS_BUFF_SIZE), SC_ERROR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_VERIFY_ATS_DUP_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void UtTest_Setup(void)
{
    UtTest_Add(SC_LoadAts_Test_Nominal, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_Nominal");
    UtTest_Add(SC_LoadAts_Test_CmdRunOffEndOfBuffer, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_LoadAts_Test_CmdRunOffEndOfBuffer");
    UtTest_Add(SC_LoadAts_Test_CmdLengthInvalid, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_LoadAts_Test_CmdLengthInvalid");
    UtTest_Add(SC_LoadAts_Test_CmdLengthZero, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_LoadAts_Test_CmdLengthZero");
    UtTest_Add(SC_LoadAts_Test_CmdNumberInvalid, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_LoadAts_Test_CmdNumberInvalid");
    UtTest_Add(SC_LoadAts_Test_AtsBufferTooSmall, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_LoadAts_Test_AtsBufferTooSmall");
    UtTest_Add(SC_LoadAts_Test_AtsEmpty, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_AtsEmpty");
    UtTest_Add(SC_LoadAts_Test_LoadExactlyBufferLength, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_LoadAts_Test_LoadExactlyBufferLength");
    UtTest_Add(SC_LoadAts_Test_CmdNotEmpty, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_CmdNotEmpty");
    UtTest_Add(SC_LoadAts_Test_InvalidIndex, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_LoadAts_Test_InvalidIndex");

    UtTest_Add(SC_BuildTimeIndexTable_Test_InvalidIndex, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_BuildTimeIndexTable_Test_InvalidIndex");
    UtTest_Add(SC_Insert_Test, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_Insert_Test");
    UtTest_Add(SC_Insert_Test_MiddleOfList, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_Insert_Test_MiddleOfList");
    UtTest_Add(SC_Insert_Test_MiddleOfListCompareAbsTimeTrue, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_Insert_Test_MiddleOfListCompareAbsTimeTrue");
    UtTest_Add(SC_Insert_Test_InvalidIndex, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_Insert_Test_InvalidIndex");
    UtTest_Add(SC_InitAtsTables_Test_InvalidIndex, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_InitAtsTables_Test_InvalidIndex");
    UtTest_Add(SC_ValidateAts_Test, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_ValidateAts_Test");
    UtTest_Add(SC_ValidateAppend_Test, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_ValidateAppend_Test");
    UtTest_Add(SC_ValidateRts_Test, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_ValidateRts_Test");
    UtTest_Add(SC_ValidateRts_Test_ParseRts, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_ValidateRts_Test_ParseRts");
    UtTest_Add(SC_LoadRts_Test_Nominal, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_LoadRts_Test_Nominal");
    UtTest_Add(SC_LoadRts_Test_InvalidIndex, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_LoadRts_Test_InvalidIndex");
    UtTest_Add(SC_ParseRts_Test_EndOfFile, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_ParseRts_Test_EndOfFile");
    UtTest_Add(SC_ParseRts_Test_InvalidMsgId, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_InvalidMsgId");
    UtTest_Add(SC_ParseRts_Test_LengthErrorTooShort, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_LengthErrorTooShort");
    UtTest_Add(SC_ParseRts_Test_LengthErrorTooLong, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_LengthErrorTooLong");
    UtTest_Add(SC_ParseRts_Test_CmdRunsOffEndOfBuffer, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_CmdRunsOffEndOfBuffer");
    UtTest_Add(SC_ParseRts_Test_CmdLengthEqualsBufferLength, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_CmdLengthEqualsBufferLength");
    UtTest_Add(SC_ParseRts_Test_CmdDoesNotFitBufferEmpty, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_CmdDoesNotFitBufferEmpty");
    UtTest_Add(SC_ParseRts_Test_CmdDoesNotFitBufferNotEmpty, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_ParseRts_Test_CmdDoesNotFitBufferNotEmpty");
    UtTest_Add(SC_UpdateAppend_Test_Nominal, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_UpdateAppend_Test_Nominal");
    UtTest_Add(SC_UpdateAppend_Test_CmdDoesNotFitBuffer, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_UpdateAppend_Test_CmdDoesNotFitBuffer");
    UtTest_Add(SC_UpdateAppend_Test_InvalidCmdLengthTooLow, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_UpdateAppend_Test_InvalidCmdLengthTooLow");
    UtTest_Add(SC_UpdateAppend_Test_InvalidCmdLengthTooHigh, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_UpdateAppend_Test_InvalidCmdLengthTooHigh");
    UtTest_Add(SC_UpdateAppend_Test_EndOfBuffer, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_UpdateAppend_Test_EndOfBuffer");
    UtTest_Add(SC_UpdateAppend_Test_CmdNumberZero, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_UpdateAppend_Test_CmdNumberZero");
    UtTest_Add(SC_UpdateAppend_Test_CmdNumberTooHigh, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_UpdateAppend_Test_CmdNumberTooHigh");
    UtTest_Add(SC_ProcessAppend_Test, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_ProcessAppend_Test");
    UtTest_Add(SC_ProcessAppend_Test_CmdLoaded, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAppend_Test_CmdLoaded");
    UtTest_Add(SC_ProcessAppend_Test_NotExecuting, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAppend_Test_NotExecuting");
    UtTest_Add(SC_ProcessAppend_Test_IdMismatch, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAppend_Test_IdMismatch");
    UtTest_Add(SC_ProcessAppend_Test_AtsNum, UT_SC_Loads_Test_Setup, SC_Test_TearDown, "SC_ProcessAppend_Test_AtsNum");
    UtTest_Add(SC_ProcessAppend_Test_InvalidIndex, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_ProcessAppend_Test_InvalidIndex");
    UtTest_Add(SC_VerifyAtsTable_Test_Nominal, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsTable_Test_Nominal");
    UtTest_Add(SC_VerifyAtsTable_Test_InvalidEntry, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsTable_Test_InvalidEntry");
    UtTest_Add(SC_VerifyAtsTable_Test_EmptyTable, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsTable_Test_EmptyTable");
    UtTest_Add(SC_VerifyAtsEntry_Test_Nominal, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_Nominal");
    UtTest_Add(SC_VerifyAtsEntry_Test_EndOfBuffer, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_EndOfBuffer");
    UtTest_Add(SC_VerifyAtsEntry_Test_InvalidCmdNumber, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_InvalidCmdNumber");
    UtTest_Add(SC_VerifyAtsEntry_Test_BufferFull, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_BufferFull");
    UtTest_Add(SC_VerifyAtsEntry_Test_InvalidCmdLengthTooLow, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_InvalidCmdLengthTooLow");
    UtTest_Add(SC_VerifyAtsEntry_Test_InvalidCmdLengthTooHigh, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_InvalidCmdLengthTooHigh");
    UtTest_Add(SC_VerifyAtsEntry_Test_BufferOverflow, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_BufferOverflow");
    UtTest_Add(SC_VerifyAtsEntry_Test_DuplicateCmdNumber, UT_SC_Loads_Test_Setup, SC_Test_TearDown,
               "SC_VerifyAtsEntry_Test_DuplicateCmdNumber");
}
