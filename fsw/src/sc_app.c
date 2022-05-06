/*************************************************************************
 ** File: sc_app.c 
 **
 **  Copyright © 2007-2014 United States Government as represented by the
 **  Administrator of the National Aeronautics and Space Administration.
 **  All Other Rights Reserved.
 **
 **  This software was created at NASA's Goddard Space Flight Center.
 **  This software is governed by the NASA Open Source Agreement and may be
 **  used, distributed and modified only pursuant to the terms of that
 **  agreement.
 **
 ** Purpose:
 **     This file contains the Stored Command main event loop function. It also
 **     contains the initialization function. The SC app handles the scheduling
 **     of stored commands for the fsw. The main event loop handles the Software
 **     Bus interface.
 **
 ** References:
 **   Flight Software Branch C Coding Standard Version 1.2
 **   CFS Development Standards Document
 **
 *************************************************************************/

/**************************************************************************
 **
 ** Include section
 **
 **************************************************************************/

#include "cfe.h"
#include "sc_app.h"
#include "sc_rts.h"
#include "sc_cmds.h"
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
    int32            Result;
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

} /* end SC_AppMain() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Application startup initialization                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SC_AppInit(void)
{
    int32 Result;

    /* Clear global data structures */
    memset(&SC_OperData, 0, sizeof(SC_OperData_t));
    memset(&SC_AppData, 0, sizeof(SC_AppData_t));

    /* Number of ATS and RTS commands already executed this second */
    SC_OperData.NumCmdsSec = 0;

    /* Continue ATS execution if ATS command checksum fails */
    SC_OperData.HkPacket.ContinueAtsOnFailureFlag = SC_CONT_ON_FAILURE_START;

    /* Make sure nothing is running */
    SC_AppData.NextProcNumber      = SC_NONE;
    SC_AppData.NextCmdTime[SC_ATP] = SC_MAX_TIME;
    SC_AppData.NextCmdTime[SC_RTP] = SC_MAX_TIME;

    /* Initialize the SC housekeeping packet */
    CFE_MSG_Init(&SC_OperData.HkPacket.TlmHeader.Msg, SC_HK_TLM_MID, sizeof(SC_HkTlm_t));

    /* Select auto-exec RTS to start during first HK request */
    if (CFE_ES_GetResetType(NULL) == CFE_PSP_RST_TYPE_POWERON)
    {
        SC_AppData.AutoStartRTS = RTS_ID_AUTO_POWER_ON;
    }
    else
    {
        SC_AppData.AutoStartRTS = RTS_ID_AUTO_PROCESSOR;
    }

    /* Must be able to register for events */
    Result = CFE_EVS_Register(NULL, 0, CFE_EVS_NO_FILTER);
    if (Result != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Event Services Register returned: 0x%08X\n", (unsigned int)Result);
        return (Result);
    }

    /* Must be able to create Software Bus message pipe */
    Result = CFE_SB_CreatePipe(&SC_OperData.CmdPipe, SC_PIPE_DEPTH, SC_CMD_PIPE_NAME);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_INIT_SB_CREATE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Software Bus Create Pipe returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Must be able to subscribe to HK request command */
    Result = CFE_SB_Subscribe(SC_SEND_HK_MID, SC_OperData.CmdPipe);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_INIT_SB_SUBSCRIBE_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Software Bus subscribe to housekeeping returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Must be able to subscribe to 1Hz wakeup command */
    Result = CFE_SB_Subscribe(SC_1HZ_WAKEUP_MID, SC_OperData.CmdPipe);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_INIT_SB_SUBSCRIBE_1HZ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Software Bus subscribe to 1 Hz cycle returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Must be able to subscribe to SC commands */
    Result = CFE_SB_Subscribe(SC_CMD_MID, SC_OperData.CmdPipe);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_INIT_SB_SUBSCRIBE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Software Bus subscribe to command returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Must be able to create and initialize tables */
    Result = SC_InitTables();
    if (Result != CFE_SUCCESS)
    {
        return (Result);
    }

    /* Send application startup event */
    CFE_EVS_SendEvent(SC_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "SC Initialized. Version %d.%d.%d.%d",
                      SC_MAJOR_VERSION, SC_MINOR_VERSION, SC_REVISION, SC_MISSION_REV);

    return (CFE_SUCCESS);

} /* end SC_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Create and initialize loadable and dump-only tables             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SC_InitTables(void)
{
    int32 Result;
    int32 i;
    int32 j;

    /* Must be able to register all tables with cFE Table Services */
    Result = SC_RegisterAllTables();
    if (Result != CFE_SUCCESS)
    {
        return (Result);
    }

    /* Must be able to get dump only table pointers */
    Result = SC_GetDumpTablePointers();
    if (Result != CFE_SUCCESS)
    {
        return (Result);
    }

    /* ATP control block status table */
    SC_OperData.AtsCtrlBlckAddr->AtpState  = SC_IDLE;
    SC_OperData.AtsCtrlBlckAddr->AtsNumber = SC_NO_ATS;
    SC_OperData.AtsCtrlBlckAddr->CmdNumber = SC_INVALID_CMD_NUMBER;

    /* RTP control block status table */
    SC_OperData.RtsCtrlBlckAddr->NumRtsActive = 0;
    SC_OperData.RtsCtrlBlckAddr->RtsNumber    = SC_INVALID_RTS_NUMBER;

    /* ATS command status table(s) */
    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        for (j = 0; j < SC_MAX_ATS_CMDS; j++)
        {
            SC_OperData.AtsCmdStatusTblAddr[i][j] = SC_EMPTY;
        }
    }

    /* RTS information table */
    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        SC_OperData.RtsInfoTblAddr[i].NextCommandTime = SC_MAX_TIME;
        SC_OperData.RtsInfoTblAddr[i].NextCommandPtr  = 0;
        SC_OperData.RtsInfoTblAddr[i].RtsStatus       = SC_EMPTY;
        SC_OperData.RtsInfoTblAddr[i].DisabledFlag    = true;
    }

    /* Load default RTS tables */
    SC_LoadDefaultTables();

    /* Must be able to get loadable table pointers */
    Result = SC_GetLoadTablePointers();
    if (Result != CFE_SUCCESS)
    {
        return (Result);
    }

    /* Register for table update notification commands */
    SC_RegisterManageCmds();

    return (CFE_SUCCESS);

} /* end SC_InitTables() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Register all SC tables with cFE Table Services                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SC_RegisterAllTables(void)
{
    int    i;
    int32  Result;
    int32  TableSize;
    uint16 TableOptions;
    char   TableName[CFE_MISSION_TBL_MAX_NAME_LENGTH];

    /* Set table options for dump only tables */
    TableOptions = (CFE_TBL_OPT_DEFAULT | CFE_TBL_OPT_SNGL_BUFFER | CFE_TBL_OPT_DUMP_ONLY);

    /* Register dump only RTS information table */
    TableSize = sizeof(SC_RtsInfoEntry_t) * SC_NUMBER_OF_RTS;
    Result    = CFE_TBL_Register(&SC_OperData.RtsInfoHandle, SC_RTSINFO_TABLE_NAME, TableSize, TableOptions, NULL);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_RTS_INFO_TABLE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RTS info table register failed, returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Register dump only RTP control block table */
    TableSize = sizeof(SC_RtpControlBlock_t);
    Result    = CFE_TBL_Register(&SC_OperData.RtsCtrlBlckHandle, SC_RTP_CTRL_TABLE_NAME, TableSize, TableOptions, NULL);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_RTS_CTRL_BLK_TABLE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RTS control block table register failed, returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Register dump only ATS information table */
    TableSize = sizeof(SC_AtsInfoTable_t) * SC_NUMBER_OF_ATS;
    Result    = CFE_TBL_Register(&SC_OperData.AtsInfoHandle, SC_ATSINFO_TABLE_NAME, TableSize, TableOptions, NULL);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_ATS_INFO_TABLE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ATS Info table register failed, returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Register dump only Append ATS information table */
    TableSize = sizeof(SC_AtsInfoTable_t);
    Result = CFE_TBL_Register(&SC_OperData.AppendInfoHandle, SC_APPENDINFO_TABLE_NAME, TableSize, TableOptions, NULL);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_APPEND_INFO_TABLE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Append ATS Info table register failed, returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Register dump only ATP control block table */
    TableSize = sizeof(SC_AtpControlBlock_t);
    Result    = CFE_TBL_Register(&SC_OperData.AtsCtrlBlckHandle, SC_ATS_CTRL_TABLE_NAME, TableSize, TableOptions, NULL);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_ATS_CTRL_BLK_TABLE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ATS control block table register failed, returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Register dump only ATS command status tables */
    TableSize = SC_MAX_ATS_CMDS * sizeof(uint32);
    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        snprintf(TableName, CFE_MISSION_TBL_MAX_NAME_LENGTH, "%s%d", SC_ATS_CMD_STAT_TABLE_NAME, i + 1);
        Result = CFE_TBL_Register(&SC_OperData.AtsCmdStatusHandle[i], TableName, TableSize, TableOptions, NULL);
        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SC_REGISTER_ATS_CMD_STATUS_TABLE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ATS command status table register failed for ATS %d, returned: 0x%08X", i + 1,
                              (unsigned int)Result);
            return (Result);
        }
    }

    /* Register loadable RTS tables */
    TableOptions = CFE_TBL_OPT_DEFAULT | CFE_TBL_OPT_SNGL_BUFFER;
    TableSize    = SC_RTS_BUFF_SIZE32 * SC_BYTES_IN_WORD;
    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        snprintf(TableName, CFE_MISSION_TBL_MAX_NAME_LENGTH, "%s%03d", SC_RTS_TABLE_NAME, i + 1);
        Result = CFE_TBL_Register(&SC_OperData.RtsTblHandle[i], TableName, TableSize, TableOptions, SC_ValidateRts);
        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SC_REGISTER_RTS_TBL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RTS Table Registration Failed for RTS %d, returned: 0x%08X", i + 1,
                              (unsigned int)Result);
            return (Result);
        }
    }

    /* Register loadable ATS tables */
    TableOptions = CFE_TBL_OPT_DBL_BUFFER;
    TableSize    = SC_ATS_BUFF_SIZE32 * SC_BYTES_IN_WORD;
    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        snprintf(TableName, CFE_MISSION_TBL_MAX_NAME_LENGTH, "%s%d", SC_ATS_TABLE_NAME, i + 1);
        Result = CFE_TBL_Register(&SC_OperData.AtsTblHandle[i], TableName, TableSize, TableOptions, SC_ValidateAts);
        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SC_REGISTER_ATS_TBL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ATS Table Registration Failed for ATS %d, returned: 0x%08X", i + 1,
                              (unsigned int)Result);
            return (Result);
        }
    }

    /* Register loadable Append ATS table */
    TableOptions = CFE_TBL_OPT_DBL_BUFFER;
    TableSize    = SC_APPEND_BUFF_SIZE32 * SC_BYTES_IN_WORD;
    Result       = CFE_TBL_Register(&SC_OperData.AppendTblHandle, SC_APPEND_TABLE_NAME, TableSize, TableOptions,
                              SC_ValidateAppend);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_APPEND_TBL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Append ATS Table Registration Failed, returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    return (CFE_SUCCESS);

} /* end SC_RegisterAllTables() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Get buffer pointers for dump only tables                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SC_GetDumpTablePointers(void)
{
    int   i;
    int32 Result;

    /* Get buffer address for dump only RTS information table */
    Result = CFE_TBL_GetAddress((void **)&SC_OperData.RtsInfoTblAddr, SC_OperData.RtsInfoHandle);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_GET_ADDRESS_RTS_INFO_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RTS Info table failed Getting Address, returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Get buffer address for dump only RTP control block table */
    Result = CFE_TBL_GetAddress((void **)&SC_OperData.RtsCtrlBlckAddr, SC_OperData.RtsCtrlBlckHandle);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_GET_ADDRESS_RTS_CTRL_BLCK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RTS Ctrl Blck table failed Getting Address, returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Get buffer address for dump only ATS information table */
    Result = CFE_TBL_GetAddress((void **)&SC_OperData.AtsInfoTblAddr, SC_OperData.AtsInfoHandle);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_GET_ADDRESS_ATS_INFO_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ATS Info table failed Getting Address, returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Get buffer address for dump only Append ATS information table */
    Result = CFE_TBL_GetAddress((void **)&SC_OperData.AppendInfoTblAddr, SC_OperData.AppendInfoHandle);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_GET_ADDRESS_APPEND_INFO_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Append ATS Info table failed Getting Address, returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Get buffer address for dump only ATP control block table */
    Result = CFE_TBL_GetAddress((void **)&SC_OperData.AtsCtrlBlckAddr, SC_OperData.AtsCtrlBlckHandle);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_GET_ADDRESS_ATS_CTRL_BLCK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ATS Ctrl Blck table failed Getting Address, returned: 0x%08X", (unsigned int)Result);
        return (Result);
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
            return (Result);
        }
    }

    return (CFE_SUCCESS);

} /* end SC_GetDumpTablePointers() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Get buffer pointers for loadable tables                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SC_GetLoadTablePointers(void)
{
    int   i;
    int32 Result;

    /* Get buffer address for loadable ATS tables */
    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        Result = CFE_TBL_GetAddress((void **)&SC_OperData.AtsTblAddr[i], SC_OperData.AtsTblHandle[i]);

        if ((Result != CFE_TBL_ERR_NEVER_LOADED) && (Result != CFE_TBL_INFO_UPDATED))
        {
            CFE_EVS_SendEvent(SC_GET_ADDRESS_ATS_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ATS table %d failed Getting Address, returned: 0x%08X", i + 1, (unsigned int)Result);
            return (Result);
        }
    }

    /* Get buffer address for loadable ATS Append table */
    Result = CFE_TBL_GetAddress((void **)&SC_OperData.AppendTblAddr, SC_OperData.AppendTblHandle);

    if ((Result != CFE_TBL_ERR_NEVER_LOADED) && (Result != CFE_TBL_INFO_UPDATED))
    {
        CFE_EVS_SendEvent(SC_GET_ADDRESS_APPEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Append ATS table failed Getting Address, returned: 0x%08X", (unsigned int)Result);
        return (Result);
    }

    /* Get buffer address for loadable RTS tables */
    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        Result = CFE_TBL_GetAddress((void **)&SC_OperData.RtsTblAddr[i], SC_OperData.RtsTblHandle[i]);

        if ((Result != CFE_TBL_ERR_NEVER_LOADED) && (Result != CFE_TBL_INFO_UPDATED))
        {
            CFE_EVS_SendEvent(SC_GET_ADDRESS_RTS_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RTS table %d failed Getting Address, returned: 0x%08X", i + 1, (unsigned int)Result);
            return (Result);
        }

        /* Process new RTS table data */
        if (Result == CFE_TBL_INFO_UPDATED)
        {
            SC_LoadRts(i);
        }
    }

    return (CFE_SUCCESS);

} /* end SC_GetLoadTablePointers() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Load default startup tables from non-volatile memory            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_LoadDefaultTables(void)
{
    char      TableName[OS_MAX_PATH_LEN];
    osal_id_t FileDesc = OS_OBJECT_ID_UNDEFINED;
    int32     RtsIndex;
    int32     RtsCount = 0;
    int32     Status   = OS_SUCCESS;

    /*
    ** Currently, only RTS tables are loaded during initialization.
    **
    ** ATS and ATS Append tables must be loaded by command.
    */
    for (RtsIndex = 0; RtsIndex < SC_NUMBER_OF_RTS; RtsIndex++)
    {
        /* Example filename: /cf/apps/sc_rts001.tbl */
        snprintf(TableName, sizeof(TableName), "%s%03d.tbl", SC_RTS_FILE_NAME, (int)(RtsIndex + 1));
        Status = OS_OpenCreate(&FileDesc, TableName, OS_FILE_FLAG_NONE, OS_READ_ONLY);

        if (Status == OS_SUCCESS)
        {
            OS_close(FileDesc);

            /* Only try to load table files that can be opened */
            Status = CFE_TBL_Load(SC_OperData.RtsTblHandle[RtsIndex], CFE_TBL_SRC_FILE, TableName);
            if (Status != CFE_SUCCESS)
            {
                RtsCount++;

                /* send an event for each failed load */
                CFE_EVS_SendEvent(SC_RTS_LOAD_FAIL_DBG_EID, CFE_EVS_EventType_DEBUG,
                                  "RTS table %d failed to load, returned: 0x%08X", (int)RtsIndex, Status);
            }
        }
        else
        {
            RtsCount++;

            /* send an event for each failed open */
            CFE_EVS_SendEvent(SC_RTS_OPEN_FAIL_DBG_EID, CFE_EVS_EventType_DEBUG,
                              "RTS table %d file open failed, returned: 0x%08X", (int)RtsIndex, Status);
        }
    }

    /* Display startup RTS load failure count */
    CFE_EVS_SendEvent(SC_RTS_LOAD_FAIL_COUNT_INFO_EID, CFE_EVS_EventType_INFORMATION,
                      "RTS table file load failure count = %d", (int)RtsCount);

    return;

} /* end SC_LoadDefaultTables() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Register to receive cFE table manage request commands           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_RegisterManageCmds(void)
{
    int32 i;

    /* Register for RTS info table manage request commands */
    CFE_TBL_NotifyByMessage(SC_OperData.RtsInfoHandle, SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_RTS_INFO);

    /* Register for RTS control block table manage request commands */
    CFE_TBL_NotifyByMessage(SC_OperData.RtsCtrlBlckHandle, SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_RTP_CTRL);

    /* Register for ATS info table manage request commands */
    CFE_TBL_NotifyByMessage(SC_OperData.AtsInfoHandle, SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_ATS_INFO);

    /* Register for ATS Append info table manage request commands */
    CFE_TBL_NotifyByMessage(SC_OperData.AppendInfoHandle, SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_APP_INFO);

    /* Register for ATS control block table manage request commands */
    CFE_TBL_NotifyByMessage(SC_OperData.AtsCtrlBlckHandle, SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_ATP_CTRL);

    /* Register for ATS Append table manage request commands */
    CFE_TBL_NotifyByMessage(SC_OperData.AppendTblHandle, SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_APPEND);

    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        /* Register for ATS cmd status table manage request commands */
        CFE_TBL_NotifyByMessage(SC_OperData.AtsCmdStatusHandle[i], SC_CMD_MID, SC_MANAGE_TABLE_CC,
                                SC_TBL_ID_ATS_CMD_0 + i);

        /* Register for ATS table manage request commands */
        CFE_TBL_NotifyByMessage(SC_OperData.AtsTblHandle[i], SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_ATS_0 + i);
    }

    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        /* Register for RTS table manage request commands */
        CFE_TBL_NotifyByMessage(SC_OperData.RtsTblHandle[i], SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_RTS_0 + i);
    }

    return;

} /* End SC_RegisterManageCmds() */

/************************/
/*  End of File Comment */
/************************/
