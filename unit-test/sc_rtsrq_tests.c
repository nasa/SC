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

void SC_StartRtsCmd_Test_Nominal(void)
{
    SC_RtsEntryHeader_t *Entry;
    SC_RtsIndex_t        RtsIndex = SC_RTS_IDX_C(0);
    size_t               MsgSize;
    SC_RtsInfoEntry_t *  RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    Entry          = (SC_RtsEntryHeader_t *)SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->WakeupCount = 0;

    UT_CmdBuf.StartRtsCmd.Payload.RtsNum = SC_RtsIndexToNum(RtsIndex);

    RtsInfoPtr->DisabledFlag = false;
    RtsInfoPtr->RtsStatus    = SC_Status_LOADED;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    MsgSize = sizeof(UT_CmdBuf.StartRtsCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* Verify results */
    UtAssert_True(RtsInfoPtr->RtsStatus == SC_Status_EXECUTING, "RtsInfoPtr->RtsStatus == SC_Status_EXECUTING");
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdCtr, 0);
    UtAssert_True(RtsInfoPtr->CmdErrCtr == 0, "RtsInfoPtr->CmdErrCtr == 0");
    SC_Assert_IDX_VALUE(RtsInfoPtr->NextCommandPtr, 0);
    UtAssert_UINT32_EQ(RtsInfoPtr->UseCtr, 1);

    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 1, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveCtr == 1, "SC_OperData.HkPacket.Payload.RtsActiveCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_START_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsCmd_Test_StartRtsNoEvents(void)
{
    SC_RtsEntryHeader_t *Entry;
    SC_RtsIndex_t        RtsIndex;
    size_t               MsgSize;
    SC_RtsInfoEntry_t *  RtsInfoPtr;

    UT_CmdBuf.StartRtsCmd.Payload.RtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS);

    RtsIndex   = SC_RtsNumToIndex(UT_CmdBuf.StartRtsCmd.Payload.RtsNum);
    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    Entry          = (SC_RtsEntryHeader_t *)SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->WakeupCount = 0;

    RtsInfoPtr->DisabledFlag = false;
    RtsInfoPtr->RtsStatus    = SC_Status_LOADED;
    RtsInfoPtr->UseCtr       = 0;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    MsgSize = sizeof(UT_CmdBuf.StartRtsCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* Verify results */
    UtAssert_True(RtsInfoPtr->RtsStatus == SC_Status_EXECUTING, "RtsInfoPtr->RtsStatus == SC_Status_EXECUTING");
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdCtr, 0);
    UtAssert_True(RtsInfoPtr->CmdErrCtr == 0, "RtsInfoPtr->CmdErrCtr == 0");
    SC_Assert_IDX_VALUE(RtsInfoPtr->NextCommandPtr, 0);
    UtAssert_UINT32_EQ(RtsInfoPtr->UseCtr, 1);

    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 1, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveCtr == 1, "SC_OperData.HkPacket.Payload.RtsActiveCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    /* Handle if SC_LAST_RTS_WITH_EVENTS is the same as SC_NUM_OF_RTS */
    if (!SC_RtsNumHasEvent(UT_CmdBuf.DisableRtsCmd.Payload.RtsNum))
    {
        UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_DBG_EID);
    }
    else
    {
        UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_START_INF_EID);
    }

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsCmd_Test_InvalidCommandLength1(void)
{
    SC_RtsEntryHeader_t *Entry;
    SC_RtsIndex_t        RtsIndex = SC_RTS_IDX_C(0);
    size_t               MsgSize;
    SC_RtsInfoEntry_t *  RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    Entry          = (SC_RtsEntryHeader_t *)SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->WakeupCount = 0;

    UT_CmdBuf.StartRtsCmd.Payload.RtsNum = SC_RtsIndexToNum(RtsIndex);

    RtsInfoPtr->DisabledFlag = false;
    RtsInfoPtr->RtsStatus    = SC_Status_LOADED;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    MsgSize = 0;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_INVLD_LEN_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsCmd_Test_InvalidCommandLength2(void)
{
    SC_RtsEntryHeader_t *Entry;
    SC_RtsIndex_t        RtsIndex = SC_RTS_IDX_C(0);
    size_t               MsgSize;
    SC_RtsInfoEntry_t *  RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    Entry          = (SC_RtsEntryHeader_t *)SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->WakeupCount = 0;

    UT_CmdBuf.StartRtsCmd.Payload.RtsNum = SC_RtsIndexToNum(RtsIndex);

    RtsInfoPtr->DisabledFlag = false;
    RtsInfoPtr->RtsStatus    = SC_Status_LOADED;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    MsgSize = SC_PACKET_MAX_SIZE + 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_INVLD_LEN_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsCmd_Test_RtsNotLoadedOrInUse(void)
{
    SC_RtsEntryHeader_t *Entry;
    SC_RtsIndex_t        RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *  RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    Entry          = (SC_RtsEntryHeader_t *)SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->WakeupCount = 0;

    UT_CmdBuf.StartRtsCmd.Payload.RtsNum = SC_RtsIndexToNum(RtsIndex);

    RtsInfoPtr->DisabledFlag = false;
    RtsInfoPtr->RtsStatus    = SC_Status_IDLE;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_NOT_LDED_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsCmd_Test_RtsDisabled(void)
{
    SC_RtsEntryHeader_t *Entry;
    SC_RtsIndex_t        RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *  RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    Entry          = (SC_RtsEntryHeader_t *)SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST);
    Entry->WakeupCount = 0;

    UT_CmdBuf.StartRtsCmd.Payload.RtsNum = SC_RtsIndexToNum(RtsIndex);

    RtsInfoPtr->DisabledFlag = true;
    RtsInfoPtr->RtsStatus    = SC_Status_LOADED;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_DISABLED_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsCmd_Test_InvalidRtsNum(void)
{
    UT_CmdBuf.StartRtsCmd.Payload.RtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS * 2);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_INVALID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsCmd_Test_InvalidRtsNumZero(void)
{
    UT_CmdBuf.StartRtsCmd.Payload.RtsNum = SC_RTS_NUM_C(0);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_INVALID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_Nominal(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    RtsInfoPtr->RtsStatus = SC_Status_LOADED;
    RtsInfoPtr->UseCtr    = 0;

    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsNum = SC_RtsIndexToNum(RtsIndex);
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsNum  = SC_RtsIndexToNum(RtsIndex);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(RtsInfoPtr->RtsStatus == SC_Status_EXECUTING, "RtsInfoPtr->RtsStatus == SC_Status_EXECUTING");
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdCtr, 0);
    UtAssert_True(RtsInfoPtr->CmdErrCtr == 0, "RtsInfoPtr->CmdErrCtr == 0");
    SC_Assert_IDX_VALUE(RtsInfoPtr->NextCommandPtr, 0);
    UtAssert_UINT32_EQ(RtsInfoPtr->UseCtr, 1);

    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 1, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveCtr == 1, "SC_OperData.HkPacket.Payload.RtsActiveCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_StartRtsGroupError(void)
{
    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS * 2);
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(SC_NUMBER_OF_RTS * 2);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_FirstRtsIndex(void)
{
    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS + 1);
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_FirstRtsIndexZero(void)
{
    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(0);
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_LastRtsIndex(void)
{
    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(1);
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(SC_NUMBER_OF_RTS + 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_LastRtsIndexZero(void)
{
    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(1);
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(0);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_FirstLastRtsIndex(void)
{
    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(2);
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_DisabledFlag(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    RtsInfoPtr->DisabledFlag   = true;
    RtsInfoPtr->RtsStatus      = SC_Status_EXECUTING;
    RtsInfoPtr->UseCtr         = 0;
    RtsInfoPtr->CmdCtr         = 0;
    RtsInfoPtr->NextCommandPtr = SC_ENTRY_OFFSET_FIRST;

    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(1);
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdCtr, 0);
    UtAssert_True(RtsInfoPtr->CmdErrCtr == 0, "RtsInfoPtr->CmdErrCtr == 0");
    SC_Assert_IDX_VALUE(RtsInfoPtr->NextCommandPtr, 0);
    UtAssert_UINT32_EQ(RtsInfoPtr->UseCtr, 0);

    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveCtr == 0, "SC_OperData.HkPacket.Payload.RtsActiveCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveErrCtr == 1,
                  "SC_OperData.HkPacket.Payload.RtsActiveErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_DISABLED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_STARTRTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_StartRtsGrpCmd_Test_RtsStatus(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    RtsInfoPtr->RtsStatus      = SC_Status_EXECUTING;
    RtsInfoPtr->UseCtr         = 0;
    RtsInfoPtr->CmdCtr         = 0;
    RtsInfoPtr->NextCommandPtr = SC_ENTRY_OFFSET_FIRST;

    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(1);
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(RtsInfoPtr->RtsStatus == SC_Status_EXECUTING, "RtsInfoPtr->RtsStatus == SC_Status_EXECUTING");
    UtAssert_UINT32_EQ(RtsInfoPtr->CmdCtr, 0);
    UtAssert_True(RtsInfoPtr->CmdErrCtr == 0, "RtsInfoPtr->CmdErrCtr == 0");
    SC_Assert_IDX_VALUE(RtsInfoPtr->NextCommandPtr, 0);
    UtAssert_UINT32_EQ(RtsInfoPtr->UseCtr, 0);

    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveCtr == 0, "SC_OperData.HkPacket.Payload.RtsActiveCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveErrCtr == 1,
                  "SC_OperData.HkPacket.Payload.RtsActiveErrCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_NOT_LDED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_STARTRTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_StopRtsCmd_Test_Nominal(void)
{
    UT_CmdBuf.StopRtsCmd.Payload.RtsNum = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsCmd(&UT_CmdBuf.StopRtsCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTS_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsCmd_Test_InvalidRts(void)
{
    UT_CmdBuf.StopRtsCmd.Payload.RtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS * 2);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsCmd(&UT_CmdBuf.StopRtsCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTS_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_Nominal(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(1);
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_Error(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS * 2);
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(SC_NUMBER_OF_RTS * 2);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_NotExecuting(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    RtsInfoPtr->RtsStatus = SC_Status_EXECUTING;

    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(1);
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_FirstRtsIndex(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS + 1);
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_FirstRtsIndexZero(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(0);
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_LastRtsIndex(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(1);
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(SC_NUMBER_OF_RTS + 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_LastRtsIndexZero(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(1);
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(0);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_FirstLastRtsIndex(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(2);
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsCmd_Test_Nominal(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    UT_CmdBuf.DisableRtsCmd.Payload.RtsNum = SC_RtsIndexToNum(RtsIndex);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsCmd(&UT_CmdBuf.DisableRtsCmd));

    /* Verify results */
    UtAssert_True(RtsInfoPtr->DisabledFlag == true, "RtsInfoPtr->DisabledFlag == true");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID,
                      SC_DISABLE_RTS_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsCmd_Test_InvalidRtsID(void)
{
    UT_CmdBuf.DisableRtsCmd.Payload.RtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS * 2);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsCmd(&UT_CmdBuf.DisableRtsCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTS_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_Nominal(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsNum = SC_RtsIndexToNum(RtsIndex);
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsNum  = SC_RtsIndexToNum(RtsIndex);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(RtsInfoPtr->DisabledFlag == true, "RtsInfoPtr->DisabledFlag == true");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_Error(void)
{
    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS * 2);
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(SC_NUMBER_OF_RTS * 2);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_FirstRtsIndex(void)
{
    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS + 1);
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_FirstRtsIndexZero(void)
{
    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(0);
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_LastRtsIndex(void)
{
    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(1);
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(SC_NUMBER_OF_RTS + 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_LastRtsIndexZero(void)
{
    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(1);
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(0);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_FirstLastRtsIndex(void)
{
    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(2);
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_DisabledFlag(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0); /* RtsNum - 1 */
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    RtsInfoPtr->DisabledFlag = true;

    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsNum = SC_RtsIndexToNum(RtsIndex);
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsNum  = SC_RtsIndexToNum(RtsIndex);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(RtsInfoPtr->DisabledFlag == true, "RtsInfoPtr->DisabledFlag == true");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsCmd_Test_Nominal(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    UT_CmdBuf.EnableRtsCmd.Payload.RtsNum = SC_RtsIndexToNum(RtsIndex);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsCmd(&UT_CmdBuf.EnableRtsCmd));

    /* Verify results */
    UtAssert_True(RtsInfoPtr->DisabledFlag == false, "RtsInfoPtr->DisabledFlag == false");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID,
                      SC_ENABLE_RTS_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsCmd_Test_InvalidRtsID(void)
{
    UT_CmdBuf.EnableRtsCmd.Payload.RtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS * 2);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsCmd(&UT_CmdBuf.EnableRtsCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTS_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsCmd_Test_InvalidRtsIDZero(void)
{
    UT_CmdBuf.EnableRtsCmd.Payload.RtsNum = SC_RTS_NUM_C(0);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsCmd(&UT_CmdBuf.EnableRtsCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTS_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_Nominal(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0); /* RtsNum - 1 */
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsNum = SC_RtsIndexToNum(RtsIndex);
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsNum  = SC_RtsIndexToNum(RtsIndex);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(RtsInfoPtr->DisabledFlag == false, "RtsInfoPtr->DisabledFlag == false");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_Error(void)
{
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS * 2);
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(SC_NUMBER_OF_RTS * 2);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_FirstRtsIndex(void)
{
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS + 1);
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_FirstRtsIndexZero(void)
{
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(0);
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_LastRtsIndex(void)
{
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(1);
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(SC_NUMBER_OF_RTS + 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_LastRtsIndexZero(void)
{
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(1);
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(0);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_FirstLastRtsIndex(void)
{
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsNum = SC_RTS_NUM_C(2);
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsNum  = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_DisabledFlag(void)
{
    SC_RtsIndex_t      RtsIndex0 = SC_RTS_IDX_C(0);
    SC_RtsIndex_t      RtsIndex1 = SC_RTS_IDX_C(1);
    SC_RtsInfoEntry_t *RtsInfoPtr0;
    SC_RtsInfoEntry_t *RtsInfoPtr1;

    RtsInfoPtr0 = SC_GetRtsInfoObject(RtsIndex0);
    RtsInfoPtr1 = SC_GetRtsInfoObject(RtsIndex1);

    RtsInfoPtr0->DisabledFlag                     = false;
    RtsInfoPtr1->DisabledFlag                     = true;
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsNum = SC_RtsIndexToNum(RtsIndex0);
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsNum  = SC_RtsIndexToNum(RtsIndex1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(RtsInfoPtr0->DisabledFlag == false, "RtsInfoPtr->DisabledFlag == false");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_KillRts_Test(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    RtsInfoPtr->RtsStatus                     = SC_Status_EXECUTING;
    SC_OperData.RtsCtrlBlckAddr->NumRtsActive = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_KillRts(RtsIndex));

    /* Verify results */
    UtAssert_True(RtsInfoPtr->RtsStatus == SC_Status_LOADED, "RtsInfoPtr->RtsStatus == SC_Status_LOADED");
    UtAssert_True(RtsInfoPtr->NextCommandTgtWakeup == SC_MAX_WAKEUP_CNT, "RtsInfoPtr->NextCommandTgtWakeup == SC_MAX_WAKEUP_CNT");
    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_KillRts_Test_NoActiveRts(void)
{
    SC_RtsIndex_t      RtsIndex = SC_RTS_IDX_C(0);
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    RtsInfoPtr->RtsStatus                     = SC_Status_EXECUTING;
    SC_OperData.RtsCtrlBlckAddr->NumRtsActive = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_KillRts(RtsIndex));

    /* Verify results */
    UtAssert_True(RtsInfoPtr->RtsStatus == SC_Status_LOADED, "RtsInfoPtr->RtsStatus == SC_Status_LOADED");
    UtAssert_True(RtsInfoPtr->NextCommandTgtWakeup == SC_MAX_WAKEUP_CNT, "RtsInfoPtr->NextCommandTgtWakeup == SC_MAX_WAKEUP_CNT");
    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_KillRts_Test_InvalidIndex(void)
{
    SC_RtsIndex_t RtsIndex = SC_RTS_IDX_C(SC_NUMBER_OF_RTS);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_KillRts(RtsIndex));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_KILLRTS_INV_INDEX_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AutoStartRts_Test_Nominal(void)
{
    SC_RtsNum_t RtsNum = SC_RTS_NUM_C(1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AutoStartRts(RtsNum));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_AutoStartRts_Test_InvalidId(void)
{
    SC_RtsNum_t RtsNum = SC_RTS_NUM_C(SC_NUMBER_OF_RTS + 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AutoStartRts(RtsNum));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_AUTOSTART_RTS_INV_ID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AutoStartRts_Test_InvalidIdZero(void)
{
    SC_RtsNum_t RtsNum = SC_RTS_NUM_C(0);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AutoStartRts(RtsNum));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_AUTOSTART_RTS_INV_ID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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
    UtTest_Add(SC_StartRtsCmd_Test_InvalidRtsNum, SC_Test_Setup, SC_Test_TearDown, "SC_StartRtsCmd_Test_InvalidRtsNum");

    UtTest_Add(SC_StartRtsCmd_Test_InvalidRtsNumZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsCmd_Test_InvalidRtsNumZero");
    UtTest_Add(SC_StartRtsGrpCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_StartRtsGrpCmd_Test_Nominal");
    UtTest_Add(SC_StartRtsGrpCmd_Test_StartRtsGroupError, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsGrpCmd_Test_StartRtsGroupError");
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
    UtTest_Add(SC_StopRtsGrpCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_StopRtsGrpCmd_Test_Nominal");
    UtTest_Add(SC_StopRtsGrpCmd_Test_Error, SC_Test_Setup, SC_Test_TearDown, "SC_StopRtsGrpCmd_Test_Error");
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
    UtTest_Add(SC_DisableRtsGrpCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_DisableRtsGrpCmd_Test_Nominal");
    UtTest_Add(SC_DisableRtsGrpCmd_Test_Error, SC_Test_Setup, SC_Test_TearDown, "SC_DisableRtsGrpCmd_Test_Error");
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
    UtTest_Add(SC_EnableRtsGrpCmd_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_EnableRtsGrpCmd_Test_Nominal");
    UtTest_Add(SC_EnableRtsGrpCmd_Test_Error, SC_Test_Setup, SC_Test_TearDown, "SC_EnableRtsGrpCmd_Test_Error");
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
