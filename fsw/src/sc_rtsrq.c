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
 *     This file contains functions to handle all of the RTS
 *     executive requests and internal reuqests to control
 *     the RTP and RTSs.
 */

/**************************************************************************
 **
 ** Include section
 **
 **************************************************************************/

#include "cfe.h"
#include "sc_app.h"
#include "sc_rtsrq.h"
#include "sc_utils.h"
#include "sc_events.h"
#include "sc_msgids.h"

/**************************************************************************
 **
 ** Functions
 **
 **************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Starts and RTS                                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_StartRtsCmd(const SC_StartRtsCmd_t *Cmd)
{
    uint16               RtsId;         /* rts ID */
    uint16               RtsIndex;      /* rts array index */
    SC_RtsEntryHeader_t *RtsEntryPtr;   /* pointer to an rts entry */
    CFE_MSG_Message_t *  RtsEntryCmd;   /* pointer to an rts command */
    CFE_MSG_Size_t       CmdLength = 0; /* the length of the 1st cmd */

    /*
     ** Check start RTS parameters
     */
    RtsId = Cmd->Payload.RtsId;

    if ((RtsId > 0) && (RtsId <= SC_NUMBER_OF_RTS))
    {
        /* convert RTS ID to RTS array index */
        RtsIndex = SC_RTS_ID_TO_INDEX(RtsId);

        /* make sure that RTS is not disabled */
        if (SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false)
        {
            /* the requested RTS is not being used and is not empty */
            if (SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_Status_LOADED)
            {
                /*
                 ** Check the command length
                 */
                RtsEntryPtr = (SC_RtsEntryHeader_t *)SC_OperData.RtsTblAddr[RtsIndex];
                RtsEntryCmd = (CFE_MSG_Message_t *)((uint8_t *)RtsEntryPtr + SC_RTS_HEADER_SIZE);

                CFE_MSG_GetSize(RtsEntryCmd, &CmdLength);

                /* Make sure the command is big enough, but not too big  */
                if (CmdLength >= SC_PACKET_MIN_SIZE && CmdLength <= SC_PACKET_MAX_SIZE)
                {
                    /*
                     **  Initialize the RTS info table entry
                     */
                    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus      = SC_Status_EXECUTING;
                    SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr         = 0;
                    SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr      = 0;
                    SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr = 0;
                    SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr++;

                    /*
                     ** Get the absolute time for the RTSs next_cmd_time
                     ** using the current time and the relative time tag.
                     */
                    SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime =
                        SC_ComputeAbsTime(((SC_RtsEntryHeader_t *)SC_OperData.RtsTblAddr[RtsIndex])->TimeTag);

                    /*
                     ** Last, Increment some global counters associated with the
                     ** starting of the RTS
                     */
                    SC_OperData.RtsCtrlBlckAddr->NumRtsActive++;
                    SC_OperData.HkPacket.Payload.RtsActiveCtr++;
                    SC_OperData.HkPacket.Payload.CmdCtr++;

                    if (Cmd->Payload.RtsId <= SC_LAST_RTS_WITH_EVENTS)
                    {
                        CFE_EVS_SendEvent(SC_RTS_START_INF_EID, CFE_EVS_EventType_INFORMATION,
                                          "RTS Number %03d Started", RtsId);
                    }
                    else
                    {
                        CFE_EVS_SendEvent(SC_STARTRTS_CMD_DBG_EID, CFE_EVS_EventType_DEBUG, "Start RTS #%d command",
                                          RtsId);
                    }
                }
                else
                { /* the length field of the 1st cmd was bad */
                    CFE_EVS_SendEvent(
                        SC_STARTRTS_CMD_INVLD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                        "Start RTS %03d Rejected: Invld Len Field for 1st Cmd in Sequence. Invld Cmd Length = %d",
                        RtsId, (int)CmdLength);

                    SC_OperData.HkPacket.Payload.CmdErrCtr++;
                    SC_OperData.HkPacket.Payload.RtsActiveErrCtr++;

                } /* end if - check command number */
            }
            else
            { /* Cannot use the RTS now */

                CFE_EVS_SendEvent(SC_STARTRTS_CMD_NOT_LDED_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Start RTS %03d Rejected: RTS Not Loaded or In Use, Status: %d", Cmd->Payload.RtsId,
                                  SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus);

                SC_OperData.HkPacket.Payload.CmdErrCtr++;
                SC_OperData.HkPacket.Payload.RtsActiveErrCtr++;

            } /* end if */
        }
        else
        { /* the RTS is disabled */
            CFE_EVS_SendEvent(SC_STARTRTS_CMD_DISABLED_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Start RTS %03d Rejected: RTS Disabled", RtsId);

            SC_OperData.HkPacket.Payload.CmdErrCtr++;
            SC_OperData.HkPacket.Payload.RtsActiveErrCtr++;
        } /* end if */
    }
    else
    { /* the rts id is invalid */
        CFE_EVS_SendEvent(SC_STARTRTS_CMD_INVALID_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Start RTS %03d Rejected: Invalid RTS ID", RtsId);

        SC_OperData.HkPacket.Payload.CmdErrCtr++;
        SC_OperData.HkPacket.Payload.RtsActiveErrCtr++;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Start a group of RTS                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_StartRtsGrpCmd(const SC_StartRtsGrpCmd_t *Cmd)
{
    uint16 FirstId;
    uint16 LastId;
    uint16 FirstIndex; /* RTS array index */
    uint16 LastIndex;
    uint16 RtsIndex;
    int32  StartCount = 0;

    FirstId = Cmd->Payload.FirstRtsId;
    LastId  = Cmd->Payload.LastRtsId;

    /* make sure the specified group is valid */
    if ((FirstId > 0) && (LastId > 0) && (FirstId <= SC_NUMBER_OF_RTS) && (LastId <= SC_NUMBER_OF_RTS) &&
        (FirstId <= LastId))
    {
        /* convert RTS ID to RTS array index */
        FirstIndex = SC_RTS_ID_TO_INDEX(FirstId);
        LastIndex  = SC_RTS_ID_TO_INDEX(LastId);

        for (RtsIndex = FirstIndex; RtsIndex <= LastIndex; RtsIndex++)
        {
            /* make sure that RTS is not disabled, empty or executing */
            if (SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false)
            {
                if (SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_Status_LOADED)
                {
                    /* initialize the RTS info table entry */
                    SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus      = SC_Status_EXECUTING;
                    SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr         = 0;
                    SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr      = 0;
                    SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr = 0;
                    SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr++;

                    /* get absolute time for 1st cmd in the RTS */
                    SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime =
                        SC_ComputeAbsTime(((SC_RtsEntryHeader_t *)SC_OperData.RtsTblAddr[RtsIndex])->TimeTag);

                    /* maintain counters associated with starting RTS */
                    SC_OperData.RtsCtrlBlckAddr->NumRtsActive++;
                    SC_OperData.HkPacket.Payload.RtsActiveCtr++;

                    /* count the RTS that were actually started */
                    StartCount++;
                }
                else
                { /* Cannot use the RTS now */
                    CFE_EVS_SendEvent(
                        SC_STARTRTSGRP_CMD_NOT_LDED_ERR_EID, CFE_EVS_EventType_ERROR,
                        "Start RTS group error: rejected RTS ID %03d, RTS Not Loaded or In Use, Status: %d",
                        SC_RTS_INDEX_TO_ID(RtsIndex), SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus);

                    SC_OperData.HkPacket.Payload.RtsActiveErrCtr++;

                } /* end if */
            }
            else
            { /* the RTS is disabled */
                CFE_EVS_SendEvent(SC_STARTRTSGRP_CMD_DISABLED_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Start RTS group error: rejected RTS ID %03d, RTS Disabled",
                                  SC_RTS_INDEX_TO_ID(RtsIndex));

                SC_OperData.HkPacket.Payload.RtsActiveErrCtr++;

            } /* end if */
        }

        /* success */
        CFE_EVS_SendEvent(SC_STARTRTSGRP_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Start RTS group: FirstID=%d, LastID=%d, Modified=%d", FirstId, LastId, (int)StartCount);
        SC_OperData.HkPacket.Payload.CmdCtr++;
    }
    else
    { /* error */
        CFE_EVS_SendEvent(SC_STARTRTSGRP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Start RTS group error: FirstID=%d, LastID=%d", FirstId, LastId);
        SC_OperData.HkPacket.Payload.CmdErrCtr++;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Stop an RTS                                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_StopRtsCmd(const SC_StopRtsCmd_t *Cmd)
{
    uint16 RtsId;    /* RTS ID */
    uint16 RtsIndex; /* RTS array index */

    RtsId = Cmd->Payload.RtsId;

    /* check the command parameter */
    if (RtsId <= SC_NUMBER_OF_RTS)
    {
        /* convert RTS ID to RTS array index */
        RtsIndex = SC_RTS_ID_TO_INDEX(RtsId);

        /* stop the rts by calling a generic routine */
        SC_KillRts(RtsIndex);

        SC_OperData.HkPacket.Payload.CmdCtr++;

        CFE_EVS_SendEvent(SC_STOPRTS_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "RTS %03d Aborted", RtsId);
    }
    else
    { /* the specified RTS is invalid */

        /* the rts id is invalid */
        CFE_EVS_SendEvent(SC_STOPRTS_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Stop RTS %03d rejected: Invalid RTS ID",
                          RtsId);

        SC_OperData.HkPacket.Payload.CmdErrCtr++;

    } /* end if */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Stop a group of RTS                                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_StopRtsGrpCmd(const SC_StopRtsGrpCmd_t *Cmd)
{
    uint16 FirstId;
    uint16 LastId;
    uint16 FirstIndex; /* RTS array index */
    uint16 LastIndex;
    uint16 RtsIndex;
    int32  StopCount = 0;

    FirstId = Cmd->Payload.FirstRtsId;
    LastId  = Cmd->Payload.LastRtsId;

    /* make sure the specified group is valid */
    if ((FirstId > 0) && (LastId > 0) && (FirstId <= SC_NUMBER_OF_RTS) && (LastId <= SC_NUMBER_OF_RTS) &&
        (FirstId <= LastId))
    {
        /* convert RTS ID to RTS array index */
        FirstIndex = SC_RTS_ID_TO_INDEX(FirstId);
        LastIndex  = SC_RTS_ID_TO_INDEX(LastId);

        for (RtsIndex = FirstIndex; RtsIndex <= LastIndex; RtsIndex++)
        {
            /* count the entries that were actually stopped */
            if (SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_Status_EXECUTING)
            {
                SC_KillRts(RtsIndex);
                StopCount++;
            }
        }

        /* success */
        CFE_EVS_SendEvent(SC_STOPRTSGRP_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Stop RTS group: FirstID=%d, LastID=%d, Modified=%d", FirstId, LastId, (int)StopCount);
        SC_OperData.HkPacket.Payload.CmdCtr++;
    }
    else
    { /* error */
        CFE_EVS_SendEvent(SC_STOPRTSGRP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Stop RTS group error: FirstID=%d, LastID=%d", FirstId, LastId);
        SC_OperData.HkPacket.Payload.CmdErrCtr++;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disables an RTS                                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_DisableRtsCmd(const SC_DisableRtsCmd_t *Cmd)
{
    uint16 RtsId;    /* RTS ID */
    uint16 RtsIndex; /* RTS array index */

    RtsId = Cmd->Payload.RtsId;

    /* make sure tha specified rts is valid */
    if (RtsId <= SC_NUMBER_OF_RTS)
    {
        /* convert RTS ID to RTS array index */
        RtsIndex = SC_RTS_ID_TO_INDEX(RtsId);

        /* disable the RTS */
        SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = true;

        /* update the command status */
        SC_OperData.HkPacket.Payload.CmdCtr++;

        CFE_EVS_SendEvent(SC_DISABLE_RTS_DEB_EID, CFE_EVS_EventType_DEBUG, "Disabled RTS %03d", RtsId);
    }
    else
    { /* it is not a valid RTS id */
        CFE_EVS_SendEvent(SC_DISRTS_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Disable RTS %03d Rejected: Invalid RTS ID",
                          RtsId);

        /* update the command error status */
        SC_OperData.HkPacket.Payload.CmdErrCtr++;
    } /* end if */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable a group of RTS                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_DisableRtsGrpCmd(const SC_DisableRtsGrpCmd_t *Cmd)
{
    uint16 FirstId;
    uint16 LastId;
    uint16 FirstIndex; /* RTS array index */
    uint16 LastIndex;
    uint16 RtsIndex;
    int32  DisableCount = 0;

    FirstId = Cmd->Payload.FirstRtsId;
    LastId  = Cmd->Payload.LastRtsId;

    /* make sure the specified group is valid */
    if ((FirstId > 0) && (LastId > 0) && (FirstId <= SC_NUMBER_OF_RTS) && (LastId <= SC_NUMBER_OF_RTS) &&
        (FirstId <= LastId))
    {
        /* convert RTS ID to RTS array index */
        FirstIndex = SC_RTS_ID_TO_INDEX(FirstId);
        LastIndex  = SC_RTS_ID_TO_INDEX(LastId);

        for (RtsIndex = FirstIndex; RtsIndex <= LastIndex; RtsIndex++)
        {
            /* count the entries that were actually disabled */
            if (SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == false)
            {
                DisableCount++;
                SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = true;
            }
        }

        /* success */
        CFE_EVS_SendEvent(SC_DISRTSGRP_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Disable RTS group: FirstID=%d, LastID=%d, Modified=%d", FirstId, LastId, (int)DisableCount);
        SC_OperData.HkPacket.Payload.CmdCtr++;
    }
    else
    { /* error */
        CFE_EVS_SendEvent(SC_DISRTSGRP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Disable RTS group error: FirstID=%d, LastID=%d", FirstId, LastId);
        SC_OperData.HkPacket.Payload.CmdErrCtr++;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enables an RTS                                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_EnableRtsCmd(const SC_EnableRtsCmd_t *Cmd)
{
    uint16 RtsId;    /* RTS ID */
    uint16 RtsIndex; /* RTS array index */

    RtsId = Cmd->Payload.RtsId;

    /* make sure the specified rts is valid */
    if ((RtsId > 0) && (RtsId <= SC_NUMBER_OF_RTS))
    {
        /* convert RTS ID to RTS array index */
        RtsIndex = SC_RTS_ID_TO_INDEX(RtsId);

        /* re-enable the RTS */
        SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;

        /* update the command status */
        SC_OperData.HkPacket.Payload.CmdCtr++;

        CFE_EVS_SendEvent(SC_ENABLE_RTS_DEB_EID, CFE_EVS_EventType_DEBUG, "Enabled RTS %03d", RtsId);
    }
    else
    { /* it is not a valid RTS id */
        CFE_EVS_SendEvent(SC_ENARTS_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Enable RTS %03d Rejected: Invalid RTS ID",
                          RtsId);

        /* update the command error status */
        SC_OperData.HkPacket.Payload.CmdErrCtr++;

    } /* end if */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable a group of RTS                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_EnableRtsGrpCmd(const SC_EnableRtsGrpCmd_t *Cmd)
{
    uint16 FirstId;
    uint16 LastId;
    uint16 FirstIndex; /* RTS array index */
    uint16 LastIndex;
    uint16 RtsIndex;
    int32  EnableCount = 0;

    FirstId = Cmd->Payload.FirstRtsId;
    LastId  = Cmd->Payload.LastRtsId;

    /* make sure the specified group is valid */
    if ((FirstId > 0) && (LastId > 0) && (FirstId <= SC_NUMBER_OF_RTS) && (LastId <= SC_NUMBER_OF_RTS) &&
        (FirstId <= LastId))
    {
        /* convert RTS ID to RTS array index */
        FirstIndex = SC_RTS_ID_TO_INDEX(FirstId);
        LastIndex  = SC_RTS_ID_TO_INDEX(LastId);

        for (RtsIndex = FirstIndex; RtsIndex <= LastIndex; RtsIndex++)
        {
            /* count the entries that were actually enabled */
            if (SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag == true)
            {
                EnableCount++;
                SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = false;
            }
        }

        /* success */
        CFE_EVS_SendEvent(SC_ENARTSGRP_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Enable RTS group: FirstID=%d, LastID=%d, Modified=%d", FirstId, LastId, (int)EnableCount);
        SC_OperData.HkPacket.Payload.CmdCtr++;
    }
    else
    { /* error */
        CFE_EVS_SendEvent(SC_ENARTSGRP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Enable RTS group error: FirstID=%d, LastID=%d", FirstId, LastId);
        SC_OperData.HkPacket.Payload.CmdErrCtr++;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Kill an RTS and clear out its data                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_KillRts(uint16 RtsIndex)
{
    /* validate RTS array index */
    if (RtsIndex >= SC_NUMBER_OF_RTS)
    {
        CFE_EVS_SendEvent(SC_KILLRTS_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR, "RTS kill error: invalid RTS index %d",
                          RtsIndex);
    }
    else if (SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_Status_EXECUTING)
    {
        /*
         ** Stop the RTS from executing
         */
        SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus       = SC_Status_LOADED;
        SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime = SC_MAX_TIME;

        /*
         ** Note: the rest of the fields are left alone
         ** to provide information on where the
         ** rts stopped. They are cleared out when it is restarted.
         */

        if (SC_OperData.RtsCtrlBlckAddr->NumRtsActive > 0)
        {
            SC_OperData.RtsCtrlBlckAddr->NumRtsActive--;
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Start an RTS on initilization                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_AutoStartRts(uint16 RtsNumber)
{
    SC_StartRtsCmd_t CmdPkt; /* the command packet to start an RTS */

    memset(&CmdPkt, 0, sizeof(CmdPkt));

    /* validate RTS ID */
    if ((RtsNumber > 0) && (RtsNumber <= SC_NUMBER_OF_RTS))
    {
        /*
         ** Format the command packet to start the first RTS
         */
        CFE_MSG_Init(CFE_MSG_PTR(CmdPkt.CommandHeader), CFE_SB_ValueToMsgId(SC_CMD_MID), sizeof(CmdPkt));

        CFE_MSG_SetFcnCode(CFE_MSG_PTR(CmdPkt.CommandHeader), SC_START_RTS_CC);

        /*
         ** Get the RTS ID to start.
         */
        CmdPkt.Payload.RtsId = RtsNumber;

        /*
         ** Now send the command back to SC
         */
        CFE_SB_TransmitMsg(CFE_MSG_PTR(CmdPkt.CommandHeader), true);
    }
    else
    {
        CFE_EVS_SendEvent(SC_AUTOSTART_RTS_INV_ID_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RTS autostart error: invalid RTS ID %d", RtsNumber);
    }
}
