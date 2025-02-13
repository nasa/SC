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

void UT_Handler_CFE_TBL_GetAddress(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    /* Set table addresses since they are cleared at initialization */
    SC_Test_SetTableAddrs();
}

/* Nominal setup for table initialization */
void UT_Setup_InitTables(void)
{
    uint8 i;

    /* There's 4 info/control tables, and SC_NUMBER_OF_ATS command status tables */
    for (i = 0; i < 4 + SC_NUMBER_OF_ATS; i++)
    {
        UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_SUCCESS);
    }
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);
}

void SC_AppMain_Test_Nominal(void)
{
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_CMD_MID);
    size_t         MsgSize   = sizeof(SC_NoopCmd_t);

    /* Called in a subfunction.  Set here to prevent segmentation fault. */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Load return buffer to make loop execute twice */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Return timeout first time through, will default to success on second */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SB_TIME_OUT);

    /* Set table addresses */
    UT_SetHandlerFunction(UT_KEY(CFE_TBL_GetAddress), UT_Handler_CFE_TBL_GetAddress, NULL);
    UT_Setup_InitTables();

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AppMain());

    /* Verify results */
    /* Generates 2 event messages we don't care about in this test */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_AppMain_Test_AppInitError(void)
{

    /* Set CFE_EVS_Register to return -1 in order to make SC_AppInit return -1, in order to reach "RunStatus =
     * CFE_ES_APP_ERROR" */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AppMain());

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_APP_EXIT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 2);
}

void SC_AppMain_Test_RcvMsgError(void)
{
    /* Set table address */
    UT_SetHandlerFunction(UT_KEY(CFE_TBL_GetAddress), UT_Handler_CFE_TBL_GetAddress, NULL);
    UT_Setup_InitTables();

    /* Set to make loop execute exactly once */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set CFE_SB_RcvMsg to return -1 in order to reach "RunStatus = CFE_ES_APP_ERROR" */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_AppMain());

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, SC_APP_EXIT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 3);
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 1);
}

void SC_AppInit_Test_NominalPowerOnReset(void)
{
    SC_OperData_t  Expected_SC_OperData;
    SC_AppData_t   Expected_SC_AppData;
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_HK_TLM_MID);
    size_t         MsgSize   = sizeof(SC_HkTlm_t);

    /* Set CFE_ES_GetResetType to return CFE_ES_POWERON_RESET in order to reach "SC_AppData.AutoStartRTS =
     * RTS_ID_AUTO_POWER_ON" */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetResetType), 1, CFE_PSP_RST_TYPE_POWERON);

    /* Set global data structures to non-zero values, to verify that SC_AppInit sets values to 0 (with some exceptions)
     */
    memset(&SC_OperData, 1, sizeof(SC_OperData));
    memset(&SC_AppData, 1, sizeof(SC_AppData));

    memset(&Expected_SC_OperData, 0, sizeof(Expected_SC_OperData));
    memset(&Expected_SC_AppData, 0, sizeof(Expected_SC_AppData));

    Expected_SC_AppData.NextCmdTime[SC_Process_ATP] = SC_MAX_TIME;
    Expected_SC_AppData.NextCmdTime[SC_Process_RTP] = SC_MAX_WAKEUP_CNT;
    Expected_SC_AppData.AutoStartRTS                = SC_RTS_NUM_C(RTS_ID_AUTO_POWER_ON);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set table address */
    UT_SetHandlerFunction(UT_KEY(CFE_TBL_GetAddress), UT_Handler_CFE_TBL_GetAddress, NULL);
    UT_Setup_InitTables();

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_AppInit(), CFE_SUCCESS);

    /* Verify results */
    Expected_SC_OperData.AtsInfoHandle         = 0;
    Expected_SC_OperData.RtsCtrlBlckHandle     = 0;
    Expected_SC_OperData.AtsCtrlBlckHandle     = 0;
    Expected_SC_OperData.AtsCmdStatusHandle[0] = 0;
    Expected_SC_OperData.AtsCmdStatusHandle[1] = 0;

    Expected_SC_OperData.HkPacket.Payload.ContinueAtsOnFailureFlag = SC_AtsCont_TRUE;

    UtAssert_MemCmp(&SC_OperData.CmdPipe, &Expected_SC_OperData.CmdPipe, sizeof(Expected_SC_OperData.CmdPipe), "2");
    UtAssert_MemCmp(&SC_OperData.AtsInfoHandle, &Expected_SC_OperData.AtsInfoHandle,
                    sizeof(Expected_SC_OperData.AtsInfoHandle), "AtsInfoHandle");
    UtAssert_MemCmp(&SC_OperData.RtsInfoHandle, &Expected_SC_OperData.RtsInfoHandle,
                    sizeof(Expected_SC_OperData.RtsInfoHandle), "13");
    UtAssert_MemCmp(&SC_OperData.RtsCtrlBlckHandle, &Expected_SC_OperData.RtsCtrlBlckHandle,
                    sizeof(Expected_SC_OperData.RtsCtrlBlckHandle), "RtsCtrlBlckHandle");
    UtAssert_MemCmp(&SC_OperData.AtsCtrlBlckHandle, &Expected_SC_OperData.AtsCtrlBlckHandle,
                    sizeof(Expected_SC_OperData.AtsCtrlBlckHandle), "AtsCtrlBlckHandle");
    UtAssert_MemCmp(&SC_OperData.AtsCmdStatusHandle, &Expected_SC_OperData.AtsCmdStatusHandle,
                    sizeof(Expected_SC_OperData.AtsCmdStatusHandle), "AtsCmdStatusHandle");
    UtAssert_MemCmp(&SC_OperData.AtsDupTestArray, &Expected_SC_OperData.AtsDupTestArray,
                    sizeof(Expected_SC_OperData.AtsDupTestArray), "21");
    UtAssert_MemCmp(&SC_OperData.NumCmdsWakeup, &Expected_SC_OperData.NumCmdsWakeup, sizeof(Expected_SC_OperData.NumCmdsWakeup),
                    "22");
    UtAssert_MemCmp(&SC_OperData.HkPacket, &Expected_SC_OperData.HkPacket, sizeof(Expected_SC_OperData.HkPacket), "23");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_INIT_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_AppInit_Test_Nominal(void)
{
    SC_OperData_t  Expected_SC_OperData;
    SC_AppData_t   Expected_SC_AppData;
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(SC_HK_TLM_MID);
    size_t         MsgSize   = sizeof(SC_HkTlm_t);

    /* Set CFE_ES_GetResetType to return something other than CFE_ES_POWERON_RESET in order to reach
     * "SC_AppData.AutoStartRTS = RTS_ID_AUTO_PROCESSOR" */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetResetType), 1, -1);

    /* Set global data structures to non-zero values, to verify that SC_AppInit sets values to 0 (with some exceptions)
     */
    memset(&SC_OperData, 1, sizeof(SC_OperData));
    memset(&SC_AppData, 1, sizeof(SC_AppData));

    memset(&Expected_SC_OperData, 0, sizeof(Expected_SC_OperData));
    memset(&Expected_SC_AppData, 0, sizeof(Expected_SC_AppData));

    Expected_SC_AppData.NextCmdTime[SC_Process_ATP] = SC_MAX_TIME;
    Expected_SC_AppData.NextCmdTime[SC_Process_RTP] = SC_MAX_WAKEUP_CNT;
    Expected_SC_AppData.AutoStartRTS                = SC_RTS_NUM_C(RTS_ID_AUTO_PROCESSOR);

    Expected_SC_OperData.HkPacket.Payload.ContinueAtsOnFailureFlag = SC_AtsCont_TRUE;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set table address */
    UT_SetHandlerFunction(UT_KEY(CFE_TBL_GetAddress), UT_Handler_CFE_TBL_GetAddress, NULL);
    UT_Setup_InitTables();

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_AppInit(), CFE_SUCCESS);

    /* Verify results */
    Expected_SC_OperData.AtsInfoHandle         = 0;
    Expected_SC_OperData.RtsCtrlBlckHandle     = 0;
    Expected_SC_OperData.AtsCtrlBlckHandle     = 0;
    Expected_SC_OperData.AtsCmdStatusHandle[0] = 0;
    Expected_SC_OperData.AtsCmdStatusHandle[1] = 0;

    UtAssert_MemCmp(&SC_OperData.CmdPipe, &Expected_SC_OperData.CmdPipe, sizeof(Expected_SC_OperData.CmdPipe), "2");
    UtAssert_MemCmp(&SC_OperData.AtsInfoHandle, &Expected_SC_OperData.AtsInfoHandle,
                    sizeof(Expected_SC_OperData.AtsInfoHandle), "AtsInfoHandle");
    UtAssert_MemCmp(&SC_OperData.RtsInfoHandle, &Expected_SC_OperData.RtsInfoHandle,
                    sizeof(Expected_SC_OperData.RtsInfoHandle), "13");
    UtAssert_MemCmp(&SC_OperData.RtsCtrlBlckHandle, &Expected_SC_OperData.RtsCtrlBlckHandle,
                    sizeof(Expected_SC_OperData.RtsCtrlBlckHandle), "RtsCtrlBlckHandle");
    UtAssert_MemCmp(&SC_OperData.AtsCtrlBlckHandle, &Expected_SC_OperData.AtsCtrlBlckHandle,
                    sizeof(Expected_SC_OperData.AtsCtrlBlckHandle), "AtsCtrlBlckHandle");
    UtAssert_MemCmp(&SC_OperData.AtsCmdStatusHandle, &Expected_SC_OperData.AtsCmdStatusHandle,
                    sizeof(Expected_SC_OperData.AtsCmdStatusHandle), "AtsCmdStatusHandle");
    UtAssert_MemCmp(&SC_OperData.AtsDupTestArray, &Expected_SC_OperData.AtsDupTestArray,
                    sizeof(Expected_SC_OperData.AtsDupTestArray), "21");
    UtAssert_MemCmp(&SC_OperData.NumCmdsWakeup, &Expected_SC_OperData.NumCmdsWakeup, sizeof(Expected_SC_OperData.NumCmdsWakeup),
                    "22");
    UtAssert_MemCmp(&SC_OperData.HkPacket, &Expected_SC_OperData.HkPacket, sizeof(Expected_SC_OperData.HkPacket), "23");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_INIT_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_AppInit_Test_EVSRegisterError(void)
{
    /* Set CFE_EVS_Register to return -1 in order to reach "CFE_ES_WriteToSysLog("Event Services Register returned:
     * 0x%08X\n", Result)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_AppInit(), -1);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 1);
}

void SC_AppInit_Test_SBCreatePipeError(void)
{
    /* Set CFE_SB_CreatePipe to return -1 in order to generate error message SC_CR_PIPE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_AppInit(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_CR_PIPE_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AppInit_Test_SBSubscribeHKError(void)
{
    /* Set CFE_SB_Subscribe to return -1 on the first call in order to generate error message
     * SC_INIT_SB_SUBSCRIBE_HK_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_AppInit(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_INIT_SB_SUBSCRIBE_HK_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AppInit_Test_SubscribeToWakeupError(void)
{
    /* Set CFE_SB_Subscribe to return -1 on the 2nd call in order to generate error message
     * SC_INIT_SB_SUBSCRIBE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 2, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_AppInit(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_INIT_SB_SUBSCRIBE_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AppInit_Test_SBSubscribeToCmdError(void)
{
    /* Set CFE_SB_Subscribe to return -1 on the 3rd call in order to generate error message
     * SC_INIT_SB_SUBSCRIBE_CMD_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 3, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_AppInit(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_INIT_SB_SUBSCRIBE_CMD_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_AppInit_Test_InitTablesError(void)
{
    /* Set CFE_TBL_Register to return -1 in order to reach return statement immediately after
     * comment "Must be able to create and initialize tables" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_AppInit(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_RTS_INFO_TABLE_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_InitTables_Test_Nominal(void)
{
    /* Set table address */
    UT_SetHandlerFunction(UT_KEY(CFE_TBL_GetAddress), UT_Handler_CFE_TBL_GetAddress, NULL);
    UT_Setup_InitTables();

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_AppInit(), CFE_SUCCESS);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LOAD_FAIL_COUNT_INFO_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_INIT_INF_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_InitTables_Test_ErrorRegisterAllTables(void)
{
    /* Set CFE_TBL_Register to return -1 in order to reach return statement immediately after
     * comment "Must be able to create and initialize tables" */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_InitTables(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_RTS_INFO_TABLE_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_InitTables_Test_ErrorGetDumpTablePointers(void)
{
    /* Set CFE_TBL_GetAddress to return -1 in order to cause SC_GetDumpTablePointers to return -1, in order to reach
     * "return(Result)" after SC_GetDumpTablePointers */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_InitTables(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_RTS_INFO_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_InitTables_Test_ErrorGetLoadTablePointers(void)
{
    /* Causes SC_GetLoadTablePointers to return -1 in order to cause SC_GetLoadTablePointers to return -1, in order to
     * reach "Return(Result)" after SC_GetLoadTablePointers */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 7, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_InitTables(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_GET_ADDRESS_ATS_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void SC_RegisterAllTables_Test_Nominal(void)
{
    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_RegisterAllTables(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_RegisterAllTables_Test_ErrorRegisterRTSInformation(void)
{
    /* Set CFE_TBL_Register to return -1 on the 1st call in order to generate error message
     * SC_REGISTER_RTS_INFO_TABLE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_RegisterAllTables(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_RTS_INFO_TABLE_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_RegisterAllTables_Test_ErrorRegisterRTPControl(void)
{
    /* Set CFE_TBL_Register to return -1 on the 2nd call in order to generate error message
     * SC_REGISTER_RTS_CTRL_BLK_TABLE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 2, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_RegisterAllTables(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_RTS_CTRL_BLK_TABLE_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_RegisterAllTables_Test_ErrorRegisterATSInformation(void)
{
    /* Set CFE_TBL_Register to return -1 on the 3rd call in order to generate error message
     * SC_REGISTER_ATS_INFO_TABLE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 3, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_RegisterAllTables(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_ATS_INFO_TABLE_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_RegisterAllTables_Test_ErrorRegisterATPControl(void)
{
    /* Set CFE_TBL_Register to return -1 on the 4th call in order to generate error message
     * SC_REGISTER_ATS_CTRL_BLK_TABLE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 4, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_RegisterAllTables(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_ATS_CTRL_BLK_TABLE_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_RegisterAllTables_Test_ErrorRegisterATSCommandStatus(void)
{
    /* Set CFE_TBL_Register to return -1 on the 5th call in order to generate error message
     * SC_REGISTER_ATS_CMD_STATUS_TABLE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 5, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_RegisterAllTables(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_ATS_CMD_STATUS_TABLE_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_RegisterAllTables_Test_ErrorRegisterLoadableRTS(void)
{
    /* Set CFE_TBL_Register to return -1 on the last call when registering loadable RTS tables in order to generate
     * error message SC_REGISTER_RTS_TBL_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 5 + SC_NUMBER_OF_ATS, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_RegisterAllTables(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_RTS_TBL_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_RegisterAllTables_Test_ErrorRegisterLoadableATS(void)
{
    /* Set CFE_TBL_Register to return -1 on the last call when registering loadable ATS tables in order to generate
     * error message SC_REGISTER_ATS_TBL_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 6 + SC_NUMBER_OF_ATS + SC_NUMBER_OF_RTS, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_RegisterAllTables(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_ATS_TBL_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_RegisterAllTables_Test_ErrorRegisterLoadableAppendATS(void)
{
    /* Set CFE_TBL_Register to return -1 on the last call when registering loadable Append ATS tables in order to
     * generate error message SC_REGISTER_APPEND_TBL_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 5 + 2 * SC_NUMBER_OF_ATS + SC_NUMBER_OF_RTS, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_RegisterAllTables(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_REGISTER_APPEND_TBL_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_RegisterDumpOnlyTables_Test_Nominal(void)
{
    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_RegisterDumpOnlyTables(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_TBL_Register, 4 + SC_NUMBER_OF_ATS);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_RegisterLoadableTables_Test_Nominal(void)
{
    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_RegisterLoadableTables(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_TBL_Register, 1 + SC_NUMBER_OF_RTS + SC_NUMBER_OF_ATS);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetDumpTablePointers_Test_Nominal(void)
{
    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_GetDumpTablePointers(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetDumpTablePointers_Test_ErrorGetAddressRTSInformation(void)
{

    /* Need to set CFE_TBL_GetAddress to return -1 on 1st call (to generate error message
     * SC_GET_ADDRESS_RTS_INFO_ERR_EID) and CFE_SUCCESS on all other calls.
     */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_GetDumpTablePointers(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_RTS_INFO_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetDumpTablePointers_Test_ErrorGetAddressRTPControl(void)
{
    /* Need to set CFE_TBL_GetAddress to return -1 on 2nd call (to generate error message
     * SC_GET_ADDRESS_RTS_CTRL_BLCK_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 2, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_GetDumpTablePointers(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_RTS_CTRL_BLCK_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetDumpTablePointers_Test_ErrorGetAddressATSInformation(void)
{
    /* Need to set CFE_TBL_GetAddress to return -1 on 3rd call (to generate error message
     * SC_GET_ADDRESS_ATS_INFO_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 3, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_GetDumpTablePointers(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_ATS_INFO_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetDumpTablePointers_Test_ErrorGetAddressATPControl(void)
{
    /* Need to set CFE_TBL_GetAddress to return -1 on 5th call (to generate error message
     * SC_GET_ADDRESS_ATS_CTRL_BLCK_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 4, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_GetDumpTablePointers(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_ATS_CTRL_BLCK_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetDumpTablePointers_Test_ErrorGetAddressATSCommandStatus(void)
{
    /* Need to set CFE_TBL_GetAddress to return -1 on 6th call (to generate error message
     * SC_GET_ADDRESS_ATS_CMD_STAT_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 5, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_GetDumpTablePointers(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_ATS_CMD_STAT_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetLoadTablePointers_Test_Nominal(void)
{
    /* Prevents all error messages */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_ERR_NEVER_LOADED);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_GetLoadTablePointers(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableATS(void)
{
    /* Need to set CFE_TBL_GetAddress to return -1 on 1st call (to generate error message SC_GET_ADDRESS_ATS_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_GetLoadTablePointers(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_ATS_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableATSAppend(void)
{
    /* Need to set CFE_TBL_GetAddress to return -1 on call (SC_NUMBER_OF_ATS + 1) (to generate error message
     * SC_GET_ADDRESS_APPEND_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_GetLoadTablePointers(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_APPEND_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableRTS(void)
{
    /* Need to set CFE_TBL_GetAddress to return -1 on call (SC_NUMBER_OF_ATS + 2) (to generate error message
     * SC_GET_ADDRESS_RTS_ERR_EID) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, CFE_TBL_INFO_UPDATED);
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_GetAddress), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(SC_GetLoadTablePointers(), -1);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_GET_ADDRESS_RTS_ERR_EID);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void SC_LoadDefaultTables_Test(void)
{
    /* Cover branch for failure of CFE_TBL_Load() */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Load), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_LoadDefaultTables());

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, SC_RTS_LOAD_FAIL_DBG_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, SC_RTS_LOAD_FAIL_COUNT_INFO_EID);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void UtTest_Setup(void)
{
    UtTest_Add(SC_AppMain_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_AppMain_Test_Nominal");
    UtTest_Add(SC_AppMain_Test_AppInitError, SC_Test_Setup, SC_Test_TearDown, "SC_AppMain_Test_AppInitError");
    UtTest_Add(SC_AppMain_Test_RcvMsgError, SC_Test_Setup, SC_Test_TearDown, "SC_AppMain_Test_RcvMsgError");
    UtTest_Add(SC_AppInit_Test_NominalPowerOnReset, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppInit_Test_NominalPowerOnReset");
    UtTest_Add(SC_AppInit_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_AppInit_Test_Nominal");
    UtTest_Add(SC_AppInit_Test_EVSRegisterError, SC_Test_Setup, SC_Test_TearDown, "SC_AppInit_Test_EVSRegisterError");
    UtTest_Add(SC_AppInit_Test_SBCreatePipeError, SC_Test_Setup, SC_Test_TearDown, "SC_AppInit_Test_SBCreatePipeError");
    UtTest_Add(SC_AppInit_Test_SBSubscribeHKError, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppInit_Test_SBSubscribeHKError");
    UtTest_Add(SC_AppInit_Test_SBSubscribeToCmdError, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppInit_Test_SBSubscribeToCmdError");
    UtTest_Add(SC_AppInit_Test_SubscribeToWakeupError, SC_Test_Setup, SC_Test_TearDown,
               "SC_AppInit_Test_SubscribeToWakeupError");
    UtTest_Add(SC_AppInit_Test_InitTablesError, SC_Test_Setup, SC_Test_TearDown, "SC_AppInit_Test_InitTablesError");
    UtTest_Add(SC_InitTables_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_InitTables_Test_Nominal");
    UtTest_Add(SC_InitTables_Test_ErrorRegisterAllTables, SC_Test_Setup, SC_Test_TearDown,
               "SC_InitTables_Test_ErrorRegisterAllTables");
    UtTest_Add(SC_InitTables_Test_ErrorGetDumpTablePointers, SC_Test_Setup, SC_Test_TearDown,
               "SC_InitTables_Test_ErrorGetDumpTablePointers");
    UtTest_Add(SC_InitTables_Test_ErrorGetLoadTablePointers, SC_Test_Setup, SC_Test_TearDown,
               "SC_InitTables_Test_ErrorGetLoadTablePointers");
    UtTest_Add(SC_RegisterAllTables_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_RegisterAllTables_Test_Nominal");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterRTSInformation, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterRTSInformation");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterRTPControl, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterRTPControl");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterATSInformation, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterATSInformation");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterATPControl, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterATPControl");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterATSCommandStatus, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterATSCommandStatus");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterLoadableRTS, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterLoadableRTS");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterLoadableATS, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterLoadableATS");
    UtTest_Add(SC_RegisterAllTables_Test_ErrorRegisterLoadableAppendATS, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterAllTables_Test_ErrorRegisterLoadableAppendATS");
    UtTest_Add(SC_RegisterDumpOnlyTables_Test_Nominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterDumpOnlyTables_Test_Nominal");
    UtTest_Add(SC_RegisterLoadableTables_Test_Nominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_RegisterLoadableTables_Test_Nominal");
    UtTest_Add(SC_GetDumpTablePointers_Test_Nominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetDumpTablePointers_Test_Nominal");
    UtTest_Add(SC_GetDumpTablePointers_Test_ErrorGetAddressRTSInformation, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetDumpTablePointers_Test_ErrorGetAddressRTSInformation");
    UtTest_Add(SC_GetDumpTablePointers_Test_ErrorGetAddressRTPControl, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetDumpTablePointers_Test_ErrorGetAddressRTPControl");
    UtTest_Add(SC_GetDumpTablePointers_Test_ErrorGetAddressATSInformation, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetDumpTablePointers_Test_ErrorGetAddressATSInformation");
    UtTest_Add(SC_GetDumpTablePointers_Test_ErrorGetAddressATPControl, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetDumpTablePointers_Test_ErrorGetAddressATPControl");
    UtTest_Add(SC_GetDumpTablePointers_Test_ErrorGetAddressATSCommandStatus, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetDumpTablePointers_Test_ErrorGetAddressATSCommandStatus");
    UtTest_Add(SC_GetLoadTablePointers_Test_Nominal, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetLoadTablePointers_Test_Nominal");
    UtTest_Add(SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableATS, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableATS");
    UtTest_Add(SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableATSAppend, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableATSAppend");
    UtTest_Add(SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableRTS, SC_Test_Setup, SC_Test_TearDown,
               "SC_GetLoadTablePointers_Test_ErrorGetAddressLoadableRTS");
    UtTest_Add(SC_LoadDefaultTables_Test, SC_Test_Setup, SC_Test_TearDown, "SC_LoadDefaultTables_Test");
}
