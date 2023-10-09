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
    uint8                RtsIndex = 0;
    size_t               MsgSize;

    Entry          = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag = 0;

    UT_CmdBuf.StartRtsCmd.Payload.RtsId = 1;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus    = SC_LOADED;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    MsgSize = sizeof(UT_CmdBuf.StartRtsCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

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
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveCtr == 1, "SC_OperData.HkPacket.Payload.RtsActiveCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_START_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsCmd_Test_StartRtsNoEvents(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex;
    size_t               MsgSize;

    UT_CmdBuf.StartRtsCmd.Payload.RtsId = SC_NUMBER_OF_RTS;

    RtsIndex = UT_CmdBuf.StartRtsCmd.Payload.RtsId - 1;

    Entry          = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag = 0;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus    = SC_LOADED;
    SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr       = 0;

    /* Set message size in order to satisfy if-statement after comment "Make sure the command is big enough, but not too
     * big" */
    MsgSize = sizeof(UT_CmdBuf.StartRtsCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

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
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveCtr == 1, "SC_OperData.HkPacket.Payload.RtsActiveCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    /* Handle if SC_LAST_RTS_WITH_EVENTS is the same as SC_NUM_OF_RTS */
    if (UT_CmdBuf.DisableRtsCmd.Payload.RtsId > SC_LAST_RTS_WITH_EVENTS)
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
    uint8                RtsIndex = 0;
    size_t               MsgSize;

    Entry          = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag = 0;

    UT_CmdBuf.StartRtsCmd.Payload.RtsId = 1;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus    = SC_LOADED;

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
    uint8                RtsIndex = 0;
    size_t               MsgSize;

    Entry          = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag = 0;

    UT_CmdBuf.StartRtsCmd.Payload.RtsId = 1;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus    = SC_LOADED;

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
    uint8                RtsIndex = 0;

    Entry          = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag = 0;

    UT_CmdBuf.StartRtsCmd.Payload.RtsId = 1;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus    = SC_IDLE;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_NOT_LDED_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsCmd_Test_RtsDisabled(void)
{
    SC_RtsEntryHeader_t *Entry;
    uint8                RtsIndex = 0;

    Entry          = (SC_RtsEntryHeader_t *)&SC_OperData.RtsTblAddr[RtsIndex][0];
    Entry->TimeTag = 0;

    UT_CmdBuf.StartRtsCmd.Payload.RtsId = 1;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = true;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus    = SC_LOADED;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_DISABLED_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsCmd_Test_InvalidRtsId(void)
{
    UT_CmdBuf.StartRtsCmd.Payload.RtsId = SC_NUMBER_OF_RTS * 2;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_INVALID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsCmd_Test_InvalidRtsIdZero(void)
{
    UT_CmdBuf.StartRtsCmd.Payload.RtsId = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsCmd(&UT_CmdBuf.StartRtsCmd));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTS_CMD_INVALID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_Nominal(void)
{
    uint8 RtsIndex = 0;

    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus = SC_LOADED;
    SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr    = 0;

    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

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
    UtAssert_True(SC_OperData.HkPacket.Payload.RtsActiveCtr == 1, "SC_OperData.HkPacket.Payload.RtsActiveCtr == 1");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_StartRtsGroupError(void)
{
    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsId = SC_NUMBER_OF_RTS * 2;
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsId  = SC_NUMBER_OF_RTS * 2;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_FirstRtsIndex(void)
{
    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsId = SC_NUMBER_OF_RTS + 1;
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_FirstRtsIndexZero(void)
{
    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsId = 0;
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_LastRtsIndex(void)
{
    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsId  = SC_NUMBER_OF_RTS + 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_LastRtsIndexZero(void)
{
    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsId  = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_FirstLastRtsIndex(void)
{
    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsId = 2;
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STARTRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StartRtsGrpCmd_Test_DisabledFlag(void)
{
    uint8 RtsIndex = 0;

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag   = true;
    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus      = SC_EXECUTING;
    SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr         = 0;
    SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr         = 0;
    SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr = 0;

    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr == 0, "SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr == 0,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr == 0,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr == 0");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr == 0, "SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr == 0");

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
    uint8 RtsIndex = 0;

    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus      = SC_EXECUTING;
    SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr         = 0;
    SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr         = 0;
    SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr = 0;

    UT_CmdBuf.StartRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.StartRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StartRtsGrpCmd(&UT_CmdBuf.StartRtsGrpCmd));

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
    UT_CmdBuf.StopRtsCmd.Payload.RtsId = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsCmd(&UT_CmdBuf.StopRtsCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTS_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsCmd_Test_InvalidRts(void)
{
    UT_CmdBuf.StopRtsCmd.Payload.RtsId = SC_NUMBER_OF_RTS * 2;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsCmd(&UT_CmdBuf.StopRtsCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTS_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_Nominal(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_Error(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsId = SC_NUMBER_OF_RTS * 2;
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsId  = SC_NUMBER_OF_RTS * 2;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_NotExecuting(void)
{
    uint8 RtsIndex = 0;

    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus = SC_EXECUTING;

    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_FirstRtsIndex(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsId = SC_NUMBER_OF_RTS + 1;
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_FirstRtsIndexZero(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsId = 0;
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_LastRtsIndex(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsId  = SC_NUMBER_OF_RTS + 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_LastRtsIndexZero(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsId  = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_StopRtsGrpCmd_Test_FirstLastRtsIndex(void)
{
    UT_CmdBuf.StopRtsGrpCmd.Payload.FirstRtsId = 2;
    UT_CmdBuf.StopRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_StopRtsGrpCmd(&UT_CmdBuf.StopRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_STOPRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsCmd_Test_Nominal(void)
{
    uint8 RtsIndex = 0;

    UT_CmdBuf.DisableRtsCmd.Payload.RtsId = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsCmd(&UT_CmdBuf.DisableRtsCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISABLE_RTS_DEB_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsCmd_Test_InvalidRtsID(void)
{
    UT_CmdBuf.DisableRtsCmd.Payload.RtsId = SC_NUMBER_OF_RTS * 2;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsCmd(&UT_CmdBuf.DisableRtsCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTS_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_Nominal(void)
{
    uint8 RtsIndex = 0; /* RtsId - 1 */

    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_Error(void)
{
    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsId = SC_NUMBER_OF_RTS * 2;
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsId  = SC_NUMBER_OF_RTS * 2;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_FirstRtsIndex(void)
{
    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsId = SC_NUMBER_OF_RTS + 1;
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_FirstRtsIndexZero(void)
{
    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsId = 0;
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_LastRtsIndex(void)
{
    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsId  = SC_NUMBER_OF_RTS + 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_LastRtsIndexZero(void)
{
    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsId  = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_FirstLastRtsIndex(void)
{
    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsId = 2;
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_DisableRtsGrpCmd_Test_DisabledFlag(void)
{
    uint8 RtsIndex = 0; /* RtsId - 1 */

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = true;

    UT_CmdBuf.DisableRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.DisableRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_DisableRtsGrpCmd(&UT_CmdBuf.DisableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_DISRTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsCmd_Test_Nominal(void)
{
    uint8 RtsIndex = 0;

    UT_CmdBuf.EnableRtsCmd.Payload.RtsId = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsCmd(&UT_CmdBuf.EnableRtsCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENABLE_RTS_DEB_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsCmd_Test_InvalidRtsID(void)
{
    UT_CmdBuf.EnableRtsCmd.Payload.RtsId = SC_NUMBER_OF_RTS * 2;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsCmd(&UT_CmdBuf.EnableRtsCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTS_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsCmd_Test_InvalidRtsIDZero(void)
{
    UT_CmdBuf.EnableRtsCmd.Payload.RtsId = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsCmd(&UT_CmdBuf.EnableRtsCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTS_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_Nominal(void)
{
    uint8 RtsIndex = 0; /* RtsId - 1 */

    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_Error(void)
{
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsId = SC_NUMBER_OF_RTS * 2;
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsId  = SC_NUMBER_OF_RTS * 2;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_FirstRtsIndex(void)
{
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsId = SC_NUMBER_OF_RTS + 1;
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_FirstRtsIndexZero(void)
{
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsId = 0;
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_LastRtsIndex(void)
{
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsId  = SC_NUMBER_OF_RTS + 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_LastRtsIndexZero(void)
{
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsId = 1;
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsId  = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_FirstLastRtsIndex(void)
{
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsId = 2;
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsId  = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdErrCtr == 1, "SC_OperData.HkPacket.Payload.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_EnableRtsGrpCmd_Test_DisabledFlag(void)
{
    uint8 RtsIndex = 0; /* RtsId - 1 */

    SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
    SC_OperData.RtsInfoTblAddr[1].DisabledFlag        = true;
    UT_CmdBuf.EnableRtsGrpCmd.Payload.FirstRtsId      = 1;
    UT_CmdBuf.EnableRtsGrpCmd.Payload.LastRtsId       = 2;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_EnableRtsGrpCmd(&UT_CmdBuf.EnableRtsGrpCmd));

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false");
    UtAssert_True(SC_OperData.HkPacket.Payload.CmdCtr == 1, "SC_OperData.HkPacket.Payload.CmdCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_ENARTSGRP_CMD_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_KillRts_Test(void)
{
    uint8 RtsIndex = 0;

    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus = SC_EXECUTING;
    SC_OperData.RtsCtrlBlckAddr->NumRtsActive      = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_KillRts(RtsIndex));

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_LOADED,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_LOADED");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime == SC_MAX_TIME,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime == SC_MAX_TIME");
    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_KillRts_Test_NoActiveRts(void)
{
    uint8 RtsIndex = 0;

    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus = SC_EXECUTING;
    SC_OperData.RtsCtrlBlckAddr->NumRtsActive      = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_KillRts(RtsIndex));

    /* Verify results */
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_LOADED,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_LOADED");
    UtAssert_True(SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime == SC_MAX_TIME,
                  "SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime == SC_MAX_TIME");
    UtAssert_True(SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0, "SC_OperData.RtsCtrlBlckAddr->NumRtsActive == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_KillRts_Test_InvalidIndex(void)
{
    uint8 RtsIndex = SC_NUMBER_OF_RTS;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_KillRts(RtsIndex));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_KILLRTS_INV_INDEX_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AutoStartRts_Test_Nominal(void)
{
    uint8 RtsId = 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AutoStartRts(RtsId));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_AutoStartRts_Test_InvalidId(void)
{
    uint8 RtsId = SC_NUMBER_OF_RTS + 1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AutoStartRts(RtsId));

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_AUTOSTART_RTS_INV_ID_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AutoStartRts_Test_InvalidIdZero(void)
{
    uint8 RtsId = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AutoStartRts(RtsId));

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
    UtTest_Add(SC_StartRtsCmd_Test_InvalidRtsId, SC_Test_Setup, SC_Test_TearDown, "SC_StartRtsCmd_Test_InvalidRtsId");

    UtTest_Add(SC_StartRtsCmd_Test_InvalidRtsIdZero, SC_Test_Setup, SC_Test_TearDown,
               "SC_StartRtsCmd_Test_InvalidRtsIdZero");
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
