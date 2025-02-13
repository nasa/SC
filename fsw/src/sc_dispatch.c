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
 *   This file contains the functions to handle processing of ground
 *   command requests, housekeeping requests, and table updates
 */

/**************************************************************************
 **
 ** Include section
 **
 **************************************************************************/

#include "cfe.h"
#include "sc_app.h"
#include "sc_atsrq.h"
#include "sc_cmds.h"
#include "sc_dispatch.h"
#include "sc_events.h"
#include "sc_msgids.h"
#include "sc_rtsrq.h"
#include "sc_utils.h"

/**************************************************************************
 **
 ** Functions
 **
 **************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SC Verify the length of the command                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool SC_VerifyCmdLength(const CFE_MSG_Message_t *Msg, size_t ExpectedLength)
{
    CFE_SB_MsgId_t    MessageID    = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode  = 0;
    bool              Result       = true;
    size_t            ActualLength = 0;

    CFE_MSG_GetSize(Msg, &ActualLength);

    /* Verify the command packet length */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(Msg, &MessageID);
        CFE_MSG_GetFcnCode(Msg, &CommandCode);

        CFE_EVS_SendEvent(SC_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid msg length: ID = 0x%08lX, CC = %d, Len = %d, Expected = %d",
                          (unsigned long)CFE_SB_MsgIdToValue(MessageID), CommandCode, (int)ActualLength,
                          (int)ExpectedLength);
        Result = false;
        if (CFE_SB_MsgIdToValue(MessageID) == SC_CMD_MID)
        {
            SC_OperData.HkPacket.Payload.CmdErrCtr++;
        }
    }
    return (Result);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Process Requests                                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_ProcessRequest(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_SB_MsgId_t MessageID = CFE_SB_INVALID_MSG_ID;

    /* cast the packet header pointer on the packet buffer */
    CFE_MSG_GetMsgId(&BufPtr->Msg, &MessageID);

    /*
     ** Get the current system time in the global SC_AppData.CurrentTime
     */
    SC_GetCurrentTime();

    switch (CFE_SB_MsgIdToValue(MessageID))
    {
        case SC_CMD_MID:
            /* request from the ground */
            SC_ProcessCommand(BufPtr);
            break;

        case SC_SEND_HK_MID:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_SendHkCmd_t)))
            {
                SC_SendHkCmd((const SC_SendHkCmd_t *)BufPtr);
            }
            break;

        case SC_WAKEUP_MID:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_WakeupCmd_t)))
            {
                SC_WakeupCmd((const SC_WakeupCmd_t *)BufPtr);
            }
            break;

        default:
            CFE_EVS_SendEvent(SC_MID_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid command pipe message ID: 0x%08lX",
                              (unsigned long)CFE_SB_MsgIdToValue(MessageID));

            SC_OperData.HkPacket.Payload.CmdErrCtr++;
            break;
    } /* end switch */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Process a command                                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_ProcessCommand(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;
    CFE_SB_MsgId_t    MessageID   = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&BufPtr->Msg, &MessageID);
    CFE_MSG_GetFcnCode(&BufPtr->Msg, &CommandCode);

    switch (CommandCode)
    {
        case SC_NOOP_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_NoopCmd_t)))
            {
                SC_NoopCmd((const SC_NoopCmd_t *)BufPtr);
            }
            break;

        case SC_RESET_COUNTERS_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_ResetCountersCmd_t)))
            {
                SC_ResetCountersCmd((const SC_ResetCountersCmd_t *)BufPtr);
            }
            break;

        case SC_START_ATS_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_StartAtsCmd_t)))
            {
                SC_StartAtsCmd((const SC_StartAtsCmd_t *)BufPtr);
            }
            break;

        case SC_STOP_ATS_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_StopAtsCmd_t)))
            {
                SC_StopAtsCmd((const SC_StopAtsCmd_t *)BufPtr);
            }
            break;

        case SC_START_RTS_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_StartRtsCmd_t)))
            {
                SC_StartRtsCmd((const SC_StartRtsCmd_t *)BufPtr);
            }
            else
            {
                SC_OperData.HkPacket.Payload.RtsActiveErrCtr++;
            }
            break;

        case SC_STOP_RTS_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_StopRtsCmd_t)))
            {
                SC_StopRtsCmd((const SC_StopRtsCmd_t *)BufPtr);
            }
            break;

        case SC_DISABLE_RTS_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_DisableRtsCmd_t)))
            {
                SC_DisableRtsCmd((const SC_DisableRtsCmd_t *)BufPtr);
            }
            break;

        case SC_ENABLE_RTS_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_EnableRtsCmd_t)))
            {
                SC_EnableRtsCmd((const SC_EnableRtsCmd_t *)BufPtr);
            }
            break;

        case SC_SWITCH_ATS_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_SwitchAtsCmd_t)))
            {
                SC_SwitchAtsCmd((const SC_SwitchAtsCmd_t *)BufPtr);
            }
            break;

        case SC_JUMP_ATS_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_JumpAtsCmd_t)))
            {
                SC_JumpAtsCmd((const SC_JumpAtsCmd_t *)BufPtr);
            }
            break;

        case SC_CONTINUE_ATS_ON_FAILURE_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_ContinueAtsOnFailureCmd_t)))
            {
                SC_ContinueAtsOnFailureCmd((const SC_ContinueAtsOnFailureCmd_t *)BufPtr);
            }
            break;

        case SC_APPEND_ATS_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_AppendAtsCmd_t)))
            {
                SC_AppendAtsCmd((const SC_AppendAtsCmd_t *)BufPtr);
            }
            break;

        case SC_MANAGE_TABLE_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_ManageTableCmd_t)))
            {
                SC_ManageTableCmd((const SC_ManageTableCmd_t *)BufPtr);
            }
            break;

        case SC_START_RTS_GRP_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_StartRtsGrpCmd_t)))
            {
                SC_StartRtsGrpCmd((const SC_StartRtsGrpCmd_t *)BufPtr);
            }
            break;

        case SC_STOP_RTS_GRP_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_StopRtsGrpCmd_t)))
            {
                SC_StopRtsGrpCmd((const SC_StopRtsGrpCmd_t *)BufPtr);
            }
            break;

        case SC_DISABLE_RTS_GRP_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_DisableRtsGrpCmd_t)))
            {
                SC_DisableRtsGrpCmd((const SC_DisableRtsGrpCmd_t *)BufPtr);
            }
            break;

        case SC_ENABLE_RTS_GRP_CC:
            if (SC_VerifyCmdLength(&BufPtr->Msg, sizeof(SC_EnableRtsGrpCmd_t)))
            {
                SC_EnableRtsGrpCmd((const SC_EnableRtsGrpCmd_t *)BufPtr);
            }
            break;

        default:
            CFE_EVS_SendEvent(SC_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid Command Code: MID =  0x%08lX CC =  %d",
                              (unsigned long)CFE_SB_MsgIdToValue(MessageID), CommandCode);
            SC_OperData.HkPacket.Payload.CmdErrCtr++;
            break;
    } /* end switch */
}
