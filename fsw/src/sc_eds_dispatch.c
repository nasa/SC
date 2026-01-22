/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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
 *   CFS Checksum (CS) Application provides the service of background
 *   checksumming user-defined objects in the CFS
 */
#include "cfe.h"

#include "sc_app.h"
#include "sc_cmds.h"
#include "sc_atsrq.h"
#include "sc_rtsrq.h"
#include "sc_events.h"
#include "sc_msgids.h"
#include "sc_dispatch.h"
#include "sc_utils.h"

#include "sc_eds_dictionary.h"
#include "sc_eds_dispatcher.h"

/* clang-format off */
static const EdsDispatchTable_EdsComponent_SC_Application_CFE_SB_Telecommand_t SC_TC_DISPATCH_TABLE =
{
    .CMD =
    {
        .NoopCmd_indication = SC_NoopCmd,
        .ResetCountersCmd_indication = SC_ResetCountersCmd,
        .StartAtsCmd_indication = SC_StartAtsCmd,
        .StopAtsCmd_indication = SC_StopAtsCmd,
        .StartRtsCmd_indication = SC_StartRtsCmd,
        .StopRtsCmd_indication = SC_StopRtsCmd,
        .DisableRtsCmd_indication = SC_DisableRtsCmd,
        .EnableRtsCmd_indication = SC_EnableRtsCmd,
        .SwitchAtsCmd_indication = SC_SwitchAtsCmd,
        .JumpAtsCmd_indication = SC_JumpAtsCmd,
        .ContinueAtsOnFailureCmd_indication = SC_ContinueAtsOnFailureCmd,
        .AppendAtsCmd_indication = SC_AppendAtsCmd,
        .ManageTableCmd_indication = SC_ManageTableCmd,
        .StartRtsGrpCmd_indication = SC_StartRtsGrpCmd,
        .StopRtsGrpCmd_indication = SC_StopRtsGrpCmd,
        .DisableRtsGrpCmd_indication = SC_DisableRtsGrpCmd,
        .EnableRtsGrpCmd_indication = SC_EnableRtsGrpCmd,

    },
    .SEND_HK          =
    {
        .indication = SC_SendHkCmd
    },
    .WAKEUP =
    {
        .indication = SC_WakeupCmd
    }
};
/* clang-format on */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SC_ProcessRequest() -- Process command pipe message           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_ProcessRequest(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_Status_t      status;
    CFE_SB_MsgId_t    MsgId;
    CFE_MSG_Size_t    MsgSize;
    CFE_MSG_FcnCode_t MsgFc;

    status = EdsDispatch_EdsComponent_SC_Application_Telecommand(BufPtr, &SC_TC_DISPATCH_TABLE);

    if (status != CFE_SUCCESS)
    {
        SC_OperData.HkPacket.Payload.CmdErrCtr++;

        CFE_MSG_GetMsgId(&BufPtr->Msg, &MsgId);
        CFE_MSG_GetFcnCode(&BufPtr->Msg, &MsgFc);

        if (status == CFE_STATUS_VALIDATION_FAILURE || status == CFE_STATUS_UNKNOWN_MSG_ID)
        {
            CFE_EVS_SendEvent(SC_MID_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid command pipe message ID: 0x%08lX",
                              (unsigned long)CFE_SB_MsgIdToValue(MsgId));
        }
        else if (status == CFE_STATUS_WRONG_MSG_LENGTH)
        {
            CFE_MSG_GetSize(&BufPtr->Msg, &MsgSize);
            CFE_EVS_SendEvent(SC_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid msg length: ID = 0x%08lX, CC = %d, Len = %lu",
                              (unsigned long)CFE_SB_MsgIdToValue(MsgId), MsgFc, (unsigned long)MsgSize);
        }
        else
        {
            CFE_EVS_SendEvent(SC_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid message recvd: ID=0x%08lX, CC=%d, status=%d",
                              (unsigned long)CFE_SB_MsgIdToValue(MsgId), MsgFc, (int)status);
        }
    }
}
