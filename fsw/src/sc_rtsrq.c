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
    SC_RtsNum_t          RtsNum;        /* rts number */
    SC_RtsIndex_t        RtsIndex;      /* rts array index */
    SC_RtsEntryHeader_t *RtsEntryPtr;   /* pointer to an rts entry */
    CFE_MSG_Message_t *  RtsEntryCmd;   /* pointer to an rts command */
    CFE_MSG_Size_t       CmdLength = 0; /* the length of the 1st cmd */
    SC_RtsInfoEntry_t *  RtsInfoPtr;

    /*
     ** Check start RTS parameters
     */
    RtsNum = Cmd->Payload.RtsNum;

    if (SC_RtsNumIsValid(RtsNum))
    {
        /* convert RTS ID to RTS array index */
        RtsIndex   = SC_RtsNumToIndex(RtsNum);
        RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

        /* make sure that RTS is not disabled */
        if (RtsInfoPtr->DisabledFlag == false)
        {
            /* the requested RTS is not being used and is not empty */
            if (RtsInfoPtr->RtsStatus == SC_Status_LOADED)
            {
                /*
                 ** Check the command length
                 */
                RtsEntryPtr = &SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST)->Header;
                RtsEntryCmd = (CFE_MSG_Message_t *)((uint8_t *)RtsEntryPtr + SC_RTS_HEADER_SIZE);

                CFE_MSG_GetSize(RtsEntryCmd, &CmdLength);

                /* Make sure the command is big enough, but not too big  */
                if (CmdLength >= SC_PACKET_MIN_SIZE && CmdLength <= SC_PACKET_MAX_SIZE)
                {
                    /*
                     **  Initialize the RTS info table entry
                     */
                    RtsInfoPtr->RtsStatus      = SC_Status_EXECUTING;
                    RtsInfoPtr->CmdCtr         = 0;
                    RtsInfoPtr->CmdErrCtr      = 0;
                    RtsInfoPtr->NextCommandPtr = SC_ENTRY_OFFSET_FIRST;
                    RtsInfoPtr->UseCtr++;

                    /*
                     ** Get the absolute wakeup count for the RTS's next_command_tgt_wakeup
                     ** using the current wakeup count and the relative wakeup count.
                     */
                    RtsInfoPtr->NextCommandTgtWakeup = SC_ComputeAbsWakeup(RtsEntryPtr->WakeupCount);

                    /*
                     ** Last, Increment some global counters associated with the
                     ** starting of the RTS
                     */
                    SC_OperData.RtsCtrlBlckAddr->NumRtsActive++;
                    SC_OperData.HkPacket.Payload.RtsActiveCtr++;
                    SC_OperData.HkPacket.Payload.CmdCtr++;

                    if (Cmd->Payload.RtsNum <= SC_LAST_RTS_WITH_EVENTS)
                    {
                        CFE_EVS_SendEvent(SC_RTS_START_INF_EID, CFE_EVS_EventType_INFORMATION,
                                          "RTS Number %03u Started", SC_IDNUM_AS_UINT(RtsNum));
                    }
                    else
                    {
                        CFE_EVS_SendEvent(SC_STARTRTS_CMD_DBG_EID, CFE_EVS_EventType_DEBUG, "Start RTS #%u command",
                                          SC_IDNUM_AS_UINT(RtsNum));
                    }
                }
                else
                { /* the length field of the 1st cmd was bad */
                    CFE_EVS_SendEvent(
                        SC_STARTRTS_CMD_INVLD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                        "Start RTS %03u Rejected: Invld Len Field for 1st Cmd in Sequence. Invld Cmd Length = %lu",
                        SC_IDNUM_AS_UINT(RtsNum), (unsigned long)CmdLength);

                    SC_OperData.HkPacket.Payload.CmdErrCtr++;
                    SC_OperData.HkPacket.Payload.RtsActiveErrCtr++;

                } /* end if - check command number */
            }
            else
            { /* Cannot use the RTS now */

                CFE_EVS_SendEvent(SC_STARTRTS_CMD_NOT_LDED_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Start RTS %03u Rejected: RTS Not Loaded or In Use, Status: %lu",
                                  SC_IDNUM_AS_UINT(Cmd->Payload.RtsNum), (unsigned long)RtsInfoPtr->RtsStatus);

                SC_OperData.HkPacket.Payload.CmdErrCtr++;
                SC_OperData.HkPacket.Payload.RtsActiveErrCtr++;

            } /* end if */
        }
        else
        { /* the RTS is disabled */
            CFE_EVS_SendEvent(SC_STARTRTS_CMD_DISABLED_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Start RTS %03u Rejected: RTS Disabled", SC_IDNUM_AS_UINT(RtsNum));

            SC_OperData.HkPacket.Payload.CmdErrCtr++;
            SC_OperData.HkPacket.Payload.RtsActiveErrCtr++;
        } /* end if */
    }
    else
    { /* the rts id is invalid */
        CFE_EVS_SendEvent(SC_STARTRTS_CMD_INVALID_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Start RTS %03u Rejected: Invalid RTS ID", SC_IDNUM_AS_UINT(RtsNum));

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
    SC_RtsNum_t        FirstRtsNum;
    SC_RtsNum_t        LastRtsNum;
    SC_RtsIndex_t      FirstIndex; /* RTS array index */
    SC_RtsIndex_t      LastIndex;
    SC_RtsIndex_t      RtsIndex;
    int32              StartCount = 0;
    SC_RtsInfoEntry_t *RtsInfoPtr;

    FirstRtsNum = Cmd->Payload.FirstRtsNum;
    LastRtsNum  = Cmd->Payload.LastRtsNum;

    /* make sure the specified group is valid */
    if (SC_RtsNumValidateRange(FirstRtsNum, LastRtsNum))
    {
        /* convert RTS ID to RTS array index */
        FirstIndex = SC_RtsNumToIndex(FirstRtsNum);
        LastIndex  = SC_RtsNumToIndex(LastRtsNum);

        for (RtsIndex = FirstIndex; SC_IDX_LESS_OR_EQ(RtsIndex, LastIndex); SC_IDX_INCREMENT(RtsIndex))
        {
            RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

            /* make sure that RTS is not disabled, empty or executing */
            if (RtsInfoPtr->DisabledFlag == false)
            {
                if (RtsInfoPtr->RtsStatus == SC_Status_LOADED)
                {
                    /* initialize the RTS info table entry */
                    RtsInfoPtr->RtsStatus      = SC_Status_EXECUTING;
                    RtsInfoPtr->CmdCtr         = 0;
                    RtsInfoPtr->CmdErrCtr      = 0;
                    RtsInfoPtr->NextCommandPtr = SC_ENTRY_OFFSET_FIRST;
                    RtsInfoPtr->UseCtr++;

                    /* get absolute wakeup count for 1st cmd in the RTS */
                    RtsInfoPtr->NextCommandTgtWakeup =
                        SC_ComputeAbsWakeup(SC_GetRtsEntryAtOffset(RtsIndex, SC_ENTRY_OFFSET_FIRST)->Header.WakeupCount);

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
                        "Start RTS group error: rejected RTS ID %03u, RTS Not Loaded or In Use, Status: %lu",
                        SC_IDNUM_AS_UINT(SC_RtsIndexToNum(RtsIndex)), (unsigned long)RtsInfoPtr->RtsStatus);

                    SC_OperData.HkPacket.Payload.RtsActiveErrCtr++;

                } /* end if */
            }
            else
            { /* the RTS is disabled */
                CFE_EVS_SendEvent(SC_STARTRTSGRP_CMD_DISABLED_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Start RTS group error: rejected RTS ID %03u, RTS Disabled",
                                  SC_IDNUM_AS_UINT(SC_RtsIndexToNum(RtsIndex)));

                SC_OperData.HkPacket.Payload.RtsActiveErrCtr++;

            } /* end if */
        }

        /* success */
        CFE_EVS_SendEvent(SC_STARTRTSGRP_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Start RTS group: FirstID=%u, LastID=%u, Modified=%d", SC_IDNUM_AS_UINT(FirstRtsNum),
                          SC_IDNUM_AS_UINT(LastRtsNum), (int)StartCount);
        SC_OperData.HkPacket.Payload.CmdCtr++;
    }
    else
    { /* error */
        CFE_EVS_SendEvent(SC_STARTRTSGRP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Start RTS group error: FirstID=%u, LastID=%u", SC_IDNUM_AS_UINT(FirstRtsNum),
                          SC_IDNUM_AS_UINT(LastRtsNum));
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
    SC_RtsNum_t   RtsNum;   /* RTS number */
    SC_RtsIndex_t RtsIndex; /* RTS array index */

    RtsNum = Cmd->Payload.RtsNum;

    /* check the command parameter */
    if (SC_RtsNumIsValid(RtsNum))
    {
        /* convert RTS ID to RTS array index */
        RtsIndex = SC_RtsNumToIndex(RtsNum);

        /* stop the rts by calling a generic routine */
        SC_KillRts(RtsIndex);

        SC_OperData.HkPacket.Payload.CmdCtr++;

        CFE_EVS_SendEvent(SC_STOPRTS_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "RTS %03u Aborted",
                          SC_IDNUM_AS_UINT(RtsNum));
    }
    else
    { /* the specified RTS is invalid */

        /* the rts id is invalid */
        CFE_EVS_SendEvent(SC_STOPRTS_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Stop RTS %03u rejected: Invalid RTS ID",
                          SC_IDNUM_AS_UINT(RtsNum));

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
    SC_RtsNum_t        FirstRtsNum;
    SC_RtsNum_t        LastRtsNum;
    SC_RtsIndex_t      FirstIndex; /* RTS array index */
    SC_RtsIndex_t      LastIndex;
    SC_RtsIndex_t      RtsIndex;
    int32              StopCount = 0;
    SC_RtsInfoEntry_t *RtsInfoPtr;

    FirstRtsNum = Cmd->Payload.FirstRtsNum;
    LastRtsNum  = Cmd->Payload.LastRtsNum;

    /* make sure the specified group is valid */
    if (SC_RtsNumValidateRange(FirstRtsNum, LastRtsNum))
    {
        /* convert RTS ID to RTS array index */
        FirstIndex = SC_RtsNumToIndex(FirstRtsNum);
        LastIndex  = SC_RtsNumToIndex(LastRtsNum);

        for (RtsIndex = FirstIndex; SC_IDX_LESS_OR_EQ(RtsIndex, LastIndex); SC_IDX_INCREMENT(RtsIndex))
        {
            RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

            /* count the entries that were actually stopped */
            if (RtsInfoPtr->RtsStatus == SC_Status_EXECUTING)
            {
                SC_KillRts(RtsIndex);
                StopCount++;
            }
        }

        /* success */
        CFE_EVS_SendEvent(SC_STOPRTSGRP_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Stop RTS group: FirstID=%u, LastID=%u, Modified=%d", SC_IDNUM_AS_UINT(FirstRtsNum),
                          SC_IDNUM_AS_UINT(LastRtsNum), (int)StopCount);
        SC_OperData.HkPacket.Payload.CmdCtr++;
    }
    else
    { /* error */
        CFE_EVS_SendEvent(SC_STOPRTSGRP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Stop RTS group error: FirstID=%u, LastID=%u", SC_IDNUM_AS_UINT(FirstRtsNum),
                          SC_IDNUM_AS_UINT(LastRtsNum));
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
    SC_RtsNum_t        RtsNum;   /* RTS number */
    SC_RtsIndex_t      RtsIndex; /* RTS array index */
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsNum = Cmd->Payload.RtsNum;

    /* make sure tha specified rts is valid */
    if (SC_RtsNumIsValid(RtsNum))
    {
        /* convert RTS ID to RTS array index */
        RtsIndex   = SC_RtsNumToIndex(RtsNum);
        RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

        /* disable the RTS */
        RtsInfoPtr->DisabledFlag = true;

        /* update the command status */
        SC_OperData.HkPacket.Payload.CmdCtr++;

        CFE_EVS_SendEvent(SC_DISABLE_RTS_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Disabled RTS %03u", SC_IDNUM_AS_UINT(RtsNum));
    }
    else
    { /* it is not a valid RTS id */
        CFE_EVS_SendEvent(SC_DISRTS_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Disable RTS %03u Rejected: Invalid RTS ID",
                          SC_IDNUM_AS_UINT(RtsNum));

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
    SC_RtsNum_t        FirstRtsNum;
    SC_RtsNum_t        LastRtsNum;
    SC_RtsIndex_t      FirstIndex; /* RTS array index */
    SC_RtsIndex_t      LastIndex;
    SC_RtsIndex_t      RtsIndex;
    int32              DisableCount = 0;
    SC_RtsInfoEntry_t *RtsInfoPtr;

    FirstRtsNum = Cmd->Payload.FirstRtsNum;
    LastRtsNum  = Cmd->Payload.LastRtsNum;

    /* make sure the specified group is valid */
    if (SC_RtsNumValidateRange(FirstRtsNum, LastRtsNum))
    {
        /* convert RTS ID to RTS array index */
        FirstIndex = SC_RtsNumToIndex(FirstRtsNum);
        LastIndex  = SC_RtsNumToIndex(LastRtsNum);

        for (RtsIndex = FirstIndex; SC_IDX_LESS_OR_EQ(RtsIndex, LastIndex); SC_IDX_INCREMENT(RtsIndex))
        {
            RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

            /* count the entries that were actually disabled */
            if (RtsInfoPtr->DisabledFlag == false)
            {
                DisableCount++;
                RtsInfoPtr->DisabledFlag = true;
            }
        }

        /* success */
        CFE_EVS_SendEvent(SC_DISRTSGRP_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Disable RTS group: FirstID=%u, LastID=%u, Modified=%d", SC_IDNUM_AS_UINT(FirstRtsNum),
                          SC_IDNUM_AS_UINT(LastRtsNum), (int)DisableCount);
        SC_OperData.HkPacket.Payload.CmdCtr++;
    }
    else
    { /* error */
        CFE_EVS_SendEvent(SC_DISRTSGRP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Disable RTS group error: FirstID=%u, LastID=%u", SC_IDNUM_AS_UINT(FirstRtsNum),
                          SC_IDNUM_AS_UINT(LastRtsNum));
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
    SC_RtsNum_t        RtsNum;   /* RTS number */
    SC_RtsIndex_t      RtsIndex; /* RTS array index */
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsNum = Cmd->Payload.RtsNum;

    /* make sure the specified rts is valid */
    if (SC_RtsNumIsValid(RtsNum))
    {
        /* convert RTS ID to RTS array index */
        RtsIndex   = SC_RtsNumToIndex(RtsNum);
        RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

        /* re-enable the RTS */
        RtsInfoPtr->DisabledFlag = false;

        /* update the command status */
        SC_OperData.HkPacket.Payload.CmdCtr++;

        CFE_EVS_SendEvent(SC_ENABLE_RTS_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Enabled RTS %03u", SC_IDNUM_AS_UINT(RtsNum));
    }
    else
    { /* it is not a valid RTS id */
        CFE_EVS_SendEvent(SC_ENARTS_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Enable RTS %03u Rejected: Invalid RTS ID",
                          SC_IDNUM_AS_UINT(RtsNum));

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
    SC_RtsNum_t        FirstRtsNum;
    SC_RtsNum_t        LastRtsNum;
    SC_RtsIndex_t      FirstIndex; /* RTS array index */
    SC_RtsIndex_t      LastIndex;
    SC_RtsIndex_t      RtsIndex;
    int32              EnableCount = 0;
    SC_RtsInfoEntry_t *RtsInfoPtr;

    FirstRtsNum = Cmd->Payload.FirstRtsNum;
    LastRtsNum  = Cmd->Payload.LastRtsNum;

    /* make sure the specified group is valid */
    if (SC_RtsNumValidateRange(FirstRtsNum, LastRtsNum))
    {
        /* convert RTS ID to RTS array index */
        FirstIndex = SC_RtsNumToIndex(FirstRtsNum);
        LastIndex  = SC_RtsNumToIndex(LastRtsNum);

        for (RtsIndex = FirstIndex; SC_IDX_LESS_OR_EQ(RtsIndex, LastIndex); SC_IDX_INCREMENT(RtsIndex))
        {
            RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

            /* count the entries that were actually enabled */
            if (RtsInfoPtr->DisabledFlag == true)
            {
                EnableCount++;
                RtsInfoPtr->DisabledFlag = false;
            }
        }

        /* success */
        CFE_EVS_SendEvent(SC_ENARTSGRP_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Enable RTS group: FirstID=%u, LastID=%u, Modified=%d", SC_IDNUM_AS_UINT(FirstRtsNum),
                          SC_IDNUM_AS_UINT(LastRtsNum), (int)EnableCount);
        SC_OperData.HkPacket.Payload.CmdCtr++;
    }
    else
    { /* error */
        CFE_EVS_SendEvent(SC_ENARTSGRP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Enable RTS group error: FirstID=%u, LastID=%u", SC_IDNUM_AS_UINT(FirstRtsNum),
                          SC_IDNUM_AS_UINT(LastRtsNum));
        SC_OperData.HkPacket.Payload.CmdErrCtr++;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Kill an RTS and clear out its data                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_KillRts(SC_RtsIndex_t RtsIndex)
{
    SC_RtsInfoEntry_t *RtsInfoPtr;

    RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

    /* validate RTS array index */
    if (!SC_RtsIndexIsValid(RtsIndex))
    {
        CFE_EVS_SendEvent(SC_KILLRTS_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR, "RTS kill error: invalid RTS index %u",
                          SC_IDX_AS_UINT(RtsIndex));
    }
    else if (RtsInfoPtr->RtsStatus == SC_Status_EXECUTING)
    {
        /*
        ** Stop the RTS from executing
        */
        RtsInfoPtr->RtsStatus       = SC_Status_LOADED;
        RtsInfoPtr->NextCommandTgtWakeup = SC_MAX_WAKEUP_CNT;

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
/* Start an RTS on initialization                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_AutoStartRts(SC_RtsNum_t RtsNum)
{
    SC_StartRtsCmd_t CmdPkt; /* the command packet to start an RTS */

    memset(&CmdPkt, 0, sizeof(CmdPkt));

    /* validate RTS ID */
    if (SC_RtsNumIsValid(RtsNum))
    {
        /*
         ** Format the command packet to start the first RTS
         */
        CFE_MSG_Init(CFE_MSG_PTR(CmdPkt.CommandHeader), CFE_SB_ValueToMsgId(SC_CMD_MID), sizeof(CmdPkt));

        CFE_MSG_SetFcnCode(CFE_MSG_PTR(CmdPkt.CommandHeader), SC_START_RTS_CC);

        /*
         ** Get the RTS ID to start.
         */
        CmdPkt.Payload.RtsNum = RtsNum;

        /*
         ** Now send the command back to SC
         */
        CFE_SB_TransmitMsg(CFE_MSG_PTR(CmdPkt.CommandHeader), true);
    }
    else
    {
        CFE_EVS_SendEvent(SC_AUTOSTART_RTS_INV_ID_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RTS autostart error: invalid RTS ID %u", SC_IDNUM_AS_UINT(RtsNum));
    }
}
