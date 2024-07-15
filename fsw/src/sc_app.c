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

/**
 * @file
 *     This file contains the Stored Command main event loop function. It also
 *     contains the initialization function. The SC app handles the scheduling
 *     of stored commands for the fsw. The main event loop handles the Software
 *     Bus interface.
 */

/**************************************************************************
 **
 ** Include section
 **
 **************************************************************************/

#include "cfe.h"
#include "sc_app.h"
#include "sc_utils.h"
#include "sc_dispatch.h"
#include "sc_loads.h"
#include "sc_events.h"
#include "sc_msgids.h"
#include "sc_perfids.h"
#include "sc_version.h"
#include "sc_verify.h"
#include <string.h>

/**************************************************************************
 **
 ** Global variables
 **
 **************************************************************************/

SC_AppData_t  SC_AppData;  /* SC Application Data */
SC_OperData_t SC_OperData; /* SC Operational Data */

/**************************************************************************
 **
 ** Functions
 **
 **************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  SC main process loop (task entry point)                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_AppMain(void)
{
    uint32           RunStatus = CFE_ES_RunStatus_APP_RUN;
    CFE_Status_t     Result;
    CFE_SB_Buffer_t *BufPtr = NULL;

    /* Performance Log (start time counter) */
    CFE_ES_PerfLogEntry(SC_APPMAIN_PERF_ID);

    /* Startup initialization */
    Result = SC_AppInit();

    /* Check for start-up error */
    if (Result != CFE_SUCCESS)
    {
        /* Set request to terminate main loop */
        RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /* Main process loop */
    while (CFE_ES_RunLoop(&RunStatus))
    {
        /* Performance Log (stop time counter) */
        CFE_ES_PerfLogExit(SC_APPMAIN_PERF_ID);

        /* Pend on Software Bus for message */
        Result = CFE_SB_ReceiveBuffer(&BufPtr, SC_OperData.CmdPipe, SC_SB_TIMEOUT);

        /* Performance Log (start time counter) */
        CFE_ES_PerfLogEntry(SC_APPMAIN_PERF_ID);

        /* Check for Software Bus error */
        if (Result == CFE_SUCCESS)
        {
            /* Invoke command handlers */
            SC_ProcessRequest(BufPtr);
        }
        else if (Result == CFE_SB_TIME_OUT)
        {
            /* no action, but also no error */
        }
        else
        {
            /* Exit main process loop */
            RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /* Check for "fatal" process error */
    if (Result != CFE_SUCCESS)
    {
        /* Send event describing reason for termination */
        CFE_EVS_SendEvent(SC_APP_EXIT_ERR_EID, CFE_EVS_EventType_ERROR, "App terminating, Result = 0x%08X",
                          (unsigned int)Result);

        /* In case cFE Event Services is not working */
        CFE_ES_WriteToSysLog("SC App terminating, Result = 0x%08X\n", (unsigned int)Result);
    }

    /* Performance Log (stop time counter) */
    CFE_ES_PerfLogExit(SC_APPMAIN_PERF_ID);

    /* Let cFE kill the app */
    CFE_ES_ExitApp(RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Application startup initialization                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CFE_Status_t SC_AppInit(void)
{
    CFE_Status_t Result;

    /* Clear global data structures */
    memset(&SC_OperData, 0, sizeof(SC_OperData));
    memset(&SC_AppData, 0, sizeof(SC_AppData));

    /* Number of ATS and RTS commands already executed this second */
    SC_OperData.NumCmdsSec = 0;

    /* Continue ATS execution if ATS command checksum fails */
    SC_OperData.HkPacket.Payload.ContinueAtsOnFailureFlag = SC_CONT_ON_FAILURE_START;

    SC_AppData.EnableHeaderUpdate = SC_PLATFORM_ENABLE_HEADER_UPDATE;

    /* assign the time ref accessor from the compile-time option */
    SC_AppData.TimeRef = SC_LookupTimeAccessor(SC_TIME_TO_USE);

    /* Make sure nothing is running */
    SC_AppData.NextProcNumber = SC_Process_NONE;
    /* SAD: SC_Process_ATP is 0, within the valid index range of NextCmdTime array, which has 2 elements */
    SC_AppData.NextCmdTime[SC_Process_ATP] = SC_MAX_TIME;
    /* SAD: SC_Process_RTP is 1, within the valid index range of NextCmdTime array, which has 2 elements */
    SC_AppData.NextCmdTime[SC_Process_RTP] = SC_MAX_TIME;

    /* Initialize the SC housekeeping packet */
    CFE_MSG_Init(CFE_MSG_PTR(SC_OperData.HkPacket.TelemetryHeader), CFE_SB_ValueToMsgId(SC_HK_TLM_MID),
                 sizeof(SC_HkTlm_t));

    /* Select auto-exec RTS to start during first HK request */
    if (CFE_ES_GetResetType(NULL) == CFE_PSP_RST_TYPE_POWERON)
    {
        SC_AppData.AutoStartRTS = SC_RTS_NUM_C(RTS_ID_AUTO_POWER_ON);
    }
    else
    {
        SC_AppData.AutoStartRTS = SC_RTS_NUM_C(RTS_ID_AUTO_PROCESSOR);
    }

    /* Must be able to register for events */
    Result = CFE_EVS_Register(NULL, 0, CFE_EVS_NO_FILTER);
    if (Result != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Event Services Register returned: 0x%08X\n", (unsigned int)Result);
        return Result;
    }

    /* Must be able to create Software Bus message pipe */
    Result = CFE_SB_CreatePipe(&SC_OperData.CmdPipe, SC_PIPE_DEPTH, SC_CMD_PIPE_NAME);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Software Bus Create Pipe returned: 0x%08X", (unsigned int)Result);
        return Result;
    }

    /* Must be able to subscribe to HK request command */
    Result = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SC_SEND_HK_MID), SC_OperData.CmdPipe);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_INIT_SB_SUBSCRIBE_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Software Bus subscribe to housekeeping returned: 0x%08X", (unsigned int)Result);
        return Result;
    }

    /* Must be able to subscribe to 1Hz wakeup command */
    Result = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SC_ONEHZ_WAKEUP_MID), SC_OperData.CmdPipe);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_INIT_SB_SUBSCRIBE_ONEHZ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Software Bus subscribe to 1 Hz cycle returned: 0x%08X", (unsigned int)Result);
        return Result;
    }

    /* Must be able to subscribe to SC commands */
    Result = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SC_CMD_MID), SC_OperData.CmdPipe);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_INIT_SB_SUBSCRIBE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Software Bus subscribe to command returned: 0x%08X", (unsigned int)Result);
        return Result;
    }

    /* Must be able to create and initialize tables */
    Result = SC_InitTables();
    if (Result != CFE_SUCCESS)
    {
        return Result;
    }

    /* Send application startup event */
    CFE_EVS_SendEvent(SC_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "SC Initialized. Version %d.%d.%d.%d",
                      SC_MAJOR_VERSION, SC_MINOR_VERSION, SC_REVISION, SC_MISSION_REV);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Create and initialize loadable and dump-only tables             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CFE_Status_t SC_InitTables(void)
{
    CFE_Status_t            Result;
    int32                   i;
    int32                   j;
    SC_RtsInfoEntry_t *     RtsInfoPtr;
    SC_AtsCmdStatusEntry_t *StatusEntryPtr;

    /* Must be able to register all tables with cFE Table Services */
    Result = SC_RegisterAllTables();
    if (Result != CFE_SUCCESS)
    {
        return Result;
    }

    /* Must be able to get dump only table pointers */
    Result = SC_GetDumpTablePointers();
    if (Result != CFE_SUCCESS)
    {
        return Result;
    }

    /* ATP control block status table */
    SC_OperData.AtsCtrlBlckAddr->AtpState   = SC_Status_IDLE;
    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_NULL;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber  = SC_INVALID_CMD_NUMBER;

    /* RTP control block status table */
    SC_OperData.RtsCtrlBlckAddr->NumRtsActive = 0;
    SC_OperData.RtsCtrlBlckAddr->CurrRtsNum   = SC_RTS_NUM_NULL;

    /* ATS command status table(s) */
    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        for (j = 0; j < SC_MAX_ATS_CMDS; j++)
        {
            StatusEntryPtr = SC_GetAtsStatusEntryForCommand(SC_ATS_IDX_C(i), SC_COMMAND_IDX_C(j));

            StatusEntryPtr->Status = SC_Status_EMPTY;
        }
    }

    /* RTS information table */
    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        RtsInfoPtr = SC_GetRtsInfoObject(SC_RTS_IDX_C(i));

        RtsInfoPtr->NextCommandTime = SC_MAX_TIME;
        RtsInfoPtr->NextCommandPtr  = SC_ENTRY_OFFSET_FIRST;
        RtsInfoPtr->RtsStatus       = SC_Status_EMPTY;
        RtsInfoPtr->DisabledFlag    = true;
    }

    /* Load default RTS tables */
    SC_LoadDefaultTables();

    /* Must be able to get loadable table pointers */
    Result = SC_GetLoadTablePointers();
    if (Result != CFE_SUCCESS)
    {
        return Result;
    }

    /* Register for table update notification commands */
    SC_RegisterManageCmds();

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Register all SC tables with cFE Table Services                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CFE_Status_t SC_RegisterAllTables(void)
{
    CFE_Status_t Result;

    Result = SC_RegisterDumpOnlyTables();
    if (Result != CFE_SUCCESS)
    {
        return Result;
    }

    Result = SC_RegisterLoadableTables();
    if (Result != CFE_SUCCESS)
    {
        return Result;
    }

    return CFE_SUCCESS;
}

CFE_Status_t SC_RegisterDumpOnlyTables(void)
{
    int          i;
    CFE_Status_t Result;
    char         TableName[CFE_MISSION_TBL_MAX_NAME_LENGTH];

    CFE_TBL_Handle_t *TblHandlePtr[4] = {&SC_OperData.RtsInfoHandle, &SC_OperData.RtsCtrlBlckHandle,
                                         &SC_OperData.AtsInfoHandle, &SC_OperData.AtsCtrlBlckHandle};
    const char *      Name[4]         = {SC_RTSINFO_TABLE_NAME, SC_RTP_CTRL_TABLE_NAME, SC_ATSINFO_TABLE_NAME,
                           SC_ATS_CTRL_TABLE_NAME};
    int32             TableSize[4]    = {sizeof(SC_RtsInfoEntry_t) * SC_NUMBER_OF_RTS, sizeof(SC_RtpControlBlock_t),
                          sizeof(SC_AtsInfoTable_t) * SC_NUMBER_OF_ATS, sizeof(SC_AtpControlBlock_t)};
    uint16            EventID[4]      = {SC_REGISTER_RTS_INFO_TABLE_ERR_EID, SC_REGISTER_RTS_CTRL_BLK_TABLE_ERR_EID,
                         SC_REGISTER_ATS_INFO_TABLE_ERR_EID, SC_REGISTER_ATS_CTRL_BLK_TABLE_ERR_EID};
    const char *      Spec[4]         = {"RTS info", "RTS control block", "ATS Info", "ATS control block"};
    uint16            TableOptions    = (CFE_TBL_OPT_DEFAULT | CFE_TBL_OPT_SNGL_BUFFER | CFE_TBL_OPT_DUMP_ONLY);

    for (i = 0; i < 4; i++)
    {
        /* Register dump only table */
        Result = CFE_TBL_Register(TblHandlePtr[i], Name[i], TableSize[i], TableOptions, NULL);
        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(EventID[i], CFE_EVS_EventType_ERROR, "%s table register failed, returned: 0x%08X",
                              Spec[i], (unsigned int)Result);
            return Result;
        }
    }

    /* Register dump only ATS command status tables */
    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        /* SAD: No need to check snprintf return value; TableName's buffer is large enough to safely hold
         * SC_ATS_CMD_STAT_TABLE_NAME with the added index */
        snprintf(TableName, CFE_MISSION_TBL_MAX_NAME_LENGTH, "%s%d", SC_ATS_CMD_STAT_TABLE_NAME, i + 1);
        Result = CFE_TBL_Register(&SC_OperData.AtsCmdStatusHandle[i], TableName, SC_MAX_ATS_CMDS * sizeof(uint32),
                                  TableOptions, NULL);

        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SC_REGISTER_ATS_CMD_STATUS_TABLE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ATS command status table register failed for ATS %d, returned: 0x%08X", i + 1,
                              (unsigned int)Result);
            return Result;
        }
    }

    return CFE_SUCCESS;
}

CFE_Status_t SC_RegisterLoadableTables(void)
{
    int          i;
    int          j;
    CFE_Status_t Result;
    char         TableName[CFE_MISSION_TBL_MAX_NAME_LENGTH];

    uint16            NumTables[2]    = {SC_NUMBER_OF_RTS, SC_NUMBER_OF_ATS};
    const char *      StrFormat[2]    = {"%s%03d", "%s%d"};
    const char *      Name[2]         = {SC_RTS_TABLE_NAME, SC_ATS_TABLE_NAME};
    CFE_TBL_Handle_t *TblHandlePtr[2] = {SC_OperData.RtsTblHandle, SC_OperData.AtsTblHandle};
    int32             TableSize[2]    = {SC_RTS_BUFF_SIZE32 * SC_BYTES_IN_WORD, SC_ATS_BUFF_SIZE32 * SC_BYTES_IN_WORD};
    uint16            TableOptions[2] = {(CFE_TBL_OPT_DEFAULT | CFE_TBL_OPT_SNGL_BUFFER), CFE_TBL_OPT_DBL_BUFFER};
    uint16            EventID[2]      = {SC_REGISTER_RTS_TBL_ERR_EID, SC_REGISTER_ATS_TBL_ERR_EID};
    const char *      Spec[2]         = {"RTS", "ATS"};
    CFE_TBL_CallbackFuncPtr_t TblValidationFuncPtr[2] = {SC_ValidateRts, SC_ValidateAts};

    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < NumTables[i]; j++)
        {
            /* SAD: No need to check snprintf return value; TableName's buffer is large enough to safely hold
             * CFE_MISSION_TBL_MAX_NAME_LENGTH with the added table identifiers */
            snprintf(TableName, CFE_MISSION_TBL_MAX_NAME_LENGTH, StrFormat[i], Name[i], j + 1);
            Result = CFE_TBL_Register(&TblHandlePtr[i][j], TableName, TableSize[i], TableOptions[i],
                                      TblValidationFuncPtr[i]);

            if (Result != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(EventID[i], CFE_EVS_EventType_ERROR,
                                  "Table Registration Failed for %s %d, returned: 0x%08X", Spec[i], j + 1,
                                  (unsigned int)Result);
                return Result;
            }
        }
    }

    /* Register loadable Append ATS table */
    Result = CFE_TBL_Register(&SC_OperData.AppendTblHandle, SC_APPEND_TABLE_NAME,
                              SC_APPEND_BUFF_SIZE32 * SC_BYTES_IN_WORD, CFE_TBL_OPT_DBL_BUFFER, SC_ValidateAppend);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_APPEND_TBL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Append ATS Table Registration Failed, returned: 0x%08X", (unsigned int)Result);
        return Result;
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Get buffer pointers for dump only tables                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CFE_Status_t SC_GetDumpTablePointers(void)
{
    int          i;
    CFE_Status_t Result;

    void **          TblAddr[4]   = {(void **)&SC_OperData.RtsInfoTblAddr, (void **)&SC_OperData.RtsCtrlBlckAddr,
                         (void **)&SC_OperData.AtsInfoTblAddr, (void **)&SC_OperData.AtsCtrlBlckAddr};
    CFE_TBL_Handle_t TblHandle[4] = {SC_OperData.RtsInfoHandle, SC_OperData.RtsCtrlBlckHandle,
                                     SC_OperData.AtsInfoHandle, SC_OperData.AtsCtrlBlckHandle};
    uint16           EventID[4]   = {SC_GET_ADDRESS_RTS_INFO_ERR_EID, SC_GET_ADDRESS_RTS_CTRL_BLCK_ERR_EID,
                         SC_GET_ADDRESS_ATS_INFO_ERR_EID, SC_GET_ADDRESS_ATS_CTRL_BLCK_ERR_EID};

    for (i = 0; i < 4; i++)
    {
        Result = CFE_TBL_GetAddress(TblAddr[i], TblHandle[i]);
        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(EventID[i], CFE_EVS_EventType_ERROR, "Table failed Getting Address, returned: 0x%08X",
                              (unsigned int)Result);
            return Result;
        }
    }

    /* Get buffer address for dump only ATS command status tables */
    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        Result = CFE_TBL_GetAddress((void **)&SC_OperData.AtsCmdStatusTblAddr[i], SC_OperData.AtsCmdStatusHandle[i]);
        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SC_GET_ADDRESS_ATS_CMD_STAT_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ATS Cmd Status table for ATS %d failed Getting Address, returned: 0x%08X", i + 1,
                              (unsigned int)Result);
            return Result;
        }
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Get buffer pointers for loadable tables                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CFE_Status_t SC_GetLoadTablePointers(void)
{
    int          i;
    CFE_Status_t Result;

    /* Get buffer address for loadable ATS tables */
    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        Result = CFE_TBL_GetAddress((void **)&SC_OperData.AtsTblAddr[i], SC_OperData.AtsTblHandle[i]);

        if ((Result != CFE_TBL_ERR_NEVER_LOADED) && (Result != CFE_TBL_INFO_UPDATED))
        {
            CFE_EVS_SendEvent(SC_GET_ADDRESS_ATS_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ATS table %d failed Getting Address, returned: 0x%08X", i + 1, (unsigned int)Result);
            return Result;
        }
    }

    /* Get buffer address for loadable ATS Append table */
    Result = CFE_TBL_GetAddress((void **)&SC_OperData.AppendTblAddr, SC_OperData.AppendTblHandle);

    if ((Result != CFE_TBL_ERR_NEVER_LOADED) && (Result != CFE_TBL_INFO_UPDATED))
    {
        CFE_EVS_SendEvent(SC_GET_ADDRESS_APPEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Append ATS table failed Getting Address, returned: 0x%08X", (unsigned int)Result);
        return Result;
    }

    /* Get buffer address for loadable RTS tables */
    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        Result = CFE_TBL_GetAddress((void **)&SC_OperData.RtsTblAddr[i], SC_OperData.RtsTblHandle[i]);

        if ((Result != CFE_TBL_ERR_NEVER_LOADED) && (Result != CFE_TBL_INFO_UPDATED))
        {
            CFE_EVS_SendEvent(SC_GET_ADDRESS_RTS_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RTS table %d failed Getting Address, returned: 0x%08X", i + 1, (unsigned int)Result);
            return Result;
        }

        /* Process new RTS table data */
        if (Result == CFE_TBL_INFO_UPDATED)
        {
            SC_LoadRts(SC_RTS_IDX_C(i));
        }
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Load default startup tables from non-volatile memory            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_LoadDefaultTables(void)
{
    char  TableName[OS_MAX_PATH_LEN];
    int32 RtsIndex;
    int32 NotLoadedCount = 0;
    int32 Status;

    /*
    ** Currently, only RTS tables are loaded during initialization.
    **
    ** ATS and ATS Append tables must be loaded by command.
    */
    for (RtsIndex = 0; RtsIndex < SC_NUMBER_OF_RTS; RtsIndex++)
    {
        /* Example filename: /cf/apps/sc_rts001.tbl */
        snprintf(TableName, sizeof(TableName), "%s%03d.tbl", SC_RTS_FILE_NAME, (int)(RtsIndex + 1));

        Status = CFE_TBL_Load(SC_OperData.RtsTblHandle[RtsIndex], CFE_TBL_SRC_FILE, TableName);
        if (Status != CFE_SUCCESS)
        {
            NotLoadedCount++;

            /* send an event for each failed load */
            CFE_EVS_SendEvent(SC_RTS_LOAD_FAIL_DBG_EID, CFE_EVS_EventType_DEBUG,
                              "RTS table %d failed to load, returned: 0x%08lX", (int)RtsIndex, (unsigned long)Status);
        }
    }

    /* Display startup RTS not loaded count */
    CFE_EVS_SendEvent(SC_RTS_LOAD_FAIL_COUNT_INFO_EID, CFE_EVS_EventType_INFORMATION,
                      "RTS table files not loaded at initialization = %d of %d", (int)NotLoadedCount, SC_NUMBER_OF_RTS);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Register to receive cFE table manage request commands           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_RegisterManageCmds(void)
{
    int32 i;

    CFE_TBL_Handle_t TblHandles[5] = {SC_OperData.RtsInfoHandle, SC_OperData.RtsCtrlBlckHandle,
                                      SC_OperData.AtsInfoHandle, SC_OperData.AtsCtrlBlckHandle,
                                      SC_OperData.AppendTblHandle};
    uint32           params[5]     = {SC_TBL_ID_RTS_INFO, SC_TBL_ID_RTP_CTRL, SC_TBL_ID_ATS_INFO, SC_TBL_ID_ATP_CTRL,
                        SC_TBL_ID_APPEND};

    for (i = 0; i < 5; i++)
    {
        CFE_TBL_NotifyByMessage(TblHandles[i], CFE_SB_ValueToMsgId(SC_CMD_MID), SC_MANAGE_TABLE_CC, params[i]);
    }

    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        /* Register for ATS cmd status table manage request commands */
        CFE_TBL_NotifyByMessage(SC_OperData.AtsCmdStatusHandle[i], CFE_SB_ValueToMsgId(SC_CMD_MID), SC_MANAGE_TABLE_CC,
                                SC_TBL_ID_ATS_CMD_0 + i);

        /* Register for ATS table manage request commands */
        CFE_TBL_NotifyByMessage(SC_OperData.AtsTblHandle[i], CFE_SB_ValueToMsgId(SC_CMD_MID), SC_MANAGE_TABLE_CC,
                                SC_TBL_ID_ATS_0 + i);
    }

    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        /* Register for RTS table manage request commands */
        CFE_TBL_NotifyByMessage(SC_OperData.RtsTblHandle[i], CFE_SB_ValueToMsgId(SC_CMD_MID), SC_MANAGE_TABLE_CC,
                                SC_TBL_ID_RTS_0 + i);
    }
}
