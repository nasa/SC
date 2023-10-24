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
 *   This file contains functions to handle getting the next time of
 *   commands for the ATP and RTP  as well as updating the time for
 *   Stored Command.
 */

/**************************************************************************
 **
 ** Include section
 **
 **************************************************************************/

#include "cfe.h"
#include "sc_app.h"
#include "sc_atsrq.h"
#include "sc_rtsrq.h"
#include "sc_state.h"
#include "sc_utils.h"
#include "sc_events.h"
#include "sc_msgdefs.h"
#include "sc_tbldefs.h"
#include <string.h>

/**************************************************************************
 **
 ** Local #defines
 **
 **************************************************************************/

/* used for RTS table iteration */
#define SC_INVALID_RTS_INDEX 0xFFFF

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Gets the time of the next RTS command                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_GetNextRtsTime(void)
{
    int16           i;        /* loop counter MUST be SIGNED !*/
    uint16          NextRts;  /* the next rts to schedule */
    SC_AbsTimeTag_t NextTime; /* the next time for the RTS */

    NextRts  = SC_INVALID_RTS_INDEX;
    NextTime = SC_MAX_TIME;

    /*
     ** Go through the table backwards to account for the RTS priority
     ** Lower number RTSs get higher priority
     ** Backward processing ensures selection of the lowest RTS number
     ** when multiple RTSs have the same next command time
     */
    for (i = SC_NUMBER_OF_RTS - 1; i >= 0; i--)
    {
        if (SC_OperData.RtsInfoTblAddr[i].RtsStatus == SC_EXECUTING)
        {
            if (SC_OperData.RtsInfoTblAddr[i].NextCommandTime <= NextTime)
            {
                NextTime = SC_OperData.RtsInfoTblAddr[i].NextCommandTime;
                NextRts  = i;
            } /* end if */
        }     /* end if */
    }         /* end for */

    if (NextRts == SC_INVALID_RTS_INDEX)
    {
        SC_OperData.RtsCtrlBlckAddr->RtsNumber = SC_INVALID_RTS_NUMBER;
        SC_AppData.NextCmdTime[SC_RTP]         = SC_MAX_TIME;
    }
    else
    {
        SC_OperData.RtsCtrlBlckAddr->RtsNumber = NextRts + 1;
        SC_AppData.NextCmdTime[SC_RTP]         = NextTime;
    } /* end if */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Decides whether an RTS or ATS command gets scheduled next       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_UpdateNextTime(void)
{
    /*
     ** First, find out which RTS needs to run next
     */
    SC_GetNextRtsTime();

    /*
     ** Start out with a default, no processors need to run next
     */
    SC_AppData.NextProcNumber = SC_NONE;

    /*
     ** Check to see if the ATP needs to schedule commands
     */
    if (SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING)
    {
        SC_AppData.NextProcNumber = SC_ATP;
    }
    /*
     ** Last, check to see if there is an RTS that needs to schedule commands
     ** This is determined by the RTS number in the RTP control block
     ** If it is zero, there is no RTS that needs to run
     */
    if (SC_OperData.RtsCtrlBlckAddr->RtsNumber > 0)
    {
        /*
         ** If the RTP needs to send commands, only send them if
         ** the RTP time is less than the ATP time. Otherwise
         ** the ATP has priority
         */
        if (SC_AppData.NextCmdTime[SC_RTP] < SC_AppData.NextCmdTime[SC_ATP])
        {
            SC_AppData.NextProcNumber = SC_RTP;
        }
    } /* end if */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Gets the next RTS Command                                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_GetNextRtsCommand(void)
{
    uint16         RtsIndex;
    uint16         CmdOffset;
    SC_RtsEntry_t *EntryPtr;
    CFE_MSG_Size_t CmdLength = 0;

    /*
     ** Make sure that the RTP is executing some RTS
     */

    if ((SC_OperData.RtsCtrlBlckAddr->RtsNumber > 0) && (SC_OperData.RtsCtrlBlckAddr->RtsNumber <= SC_NUMBER_OF_RTS))
    {
        /* Get the index of the rts that is running */
        RtsIndex = SC_RTS_NUM_TO_INDEX(SC_OperData.RtsCtrlBlckAddr->RtsNumber);
        /*
         ** Find out if the RTS is EXECUTING or just STARTED
         */
        if (SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus == SC_EXECUTING)
        {
            /*
             ** Get the information needed to find the next command
             */
            CmdOffset = SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr;
            EntryPtr  = (SC_RtsEntry_t *)&SC_OperData.RtsTblAddr[RtsIndex][CmdOffset];

            CFE_MSG_GetSize(CFE_MSG_PTR(EntryPtr->Msg), &CmdLength);
            CmdLength += SC_RTS_HEADER_SIZE;

            /*
             ** calculate the new command offset and new command length
             ** Cmd Length is in bytes, so we convert it to words
             ** (plus 1 to round byte len up to word len)
             */

            CmdOffset = CmdOffset + ((CmdLength + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD);
            /*
             ** if the end of the buffer is not reached.
             ** This check is made to make sure that at least the minimum
             ** Sized packet fits in the buffer. It assures we are not reading
             ** bogus length info from other data.
             */

            /* If at least the header for a command plus the RTS header can fit in the buffer */
            if (CmdOffset <= (SC_RTS_BUFF_SIZE32 - SC_RTS_HDR_WORDS))
            {
                /*
                 ** Get the next RTS command
                 */
                EntryPtr = (SC_RtsEntry_t *)&SC_OperData.RtsTblAddr[RtsIndex][CmdOffset];

                /*
                 ** get the length of the new command
                 */
                CFE_MSG_GetSize(CFE_MSG_PTR(EntryPtr->Msg), &CmdLength);
                CmdLength += SC_RTS_HEADER_SIZE;

                /*
                 ** Check to see if the command length is less than the size of a header.
                 ** This indicates that there are no more commands
                 */

                if ((CmdLength - SC_RTS_HEADER_SIZE) >= (SC_PACKET_MIN_SIZE))
                {
                    /*
                     ** Check to see if the command length is too big
                     ** If it is , then there is an error with the command
                     */
                    if ((CmdLength - SC_RTS_HEADER_SIZE) <= SC_PACKET_MAX_SIZE)
                    {
                        /*
                         ** Last Check is to check to see if the command
                         ** runs off of the end of the buffer
                         ** (plus 1 to round byte len up to word len)
                         */
                        if (CmdOffset + ((CmdLength + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD) <= SC_RTS_BUFF_SIZE32)
                        {
                            /*
                             ** Everything passed!
                             ** Update the proper next command time for that RTS
                             */
                            SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime =
                                SC_ComputeAbsTime(EntryPtr->Header.TimeTag);

                            /*
                             ** Update the appropriate RTS info table current command pointer
                             */
                            SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr = CmdOffset;
                        }
                        else
                        { /* the command runs past the end of the buffer */

                            /*
                             ** Having a command that runs off of the end of the buffer
                             ** is an error condition, so record it
                             */
                            SC_OperData.HkPacket.Payload.RtsCmdErrCtr++;
                            SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr++;
                            SC_OperData.HkPacket.Payload.LastRtsErrSeq = SC_OperData.RtsCtrlBlckAddr->RtsNumber;
                            SC_OperData.HkPacket.Payload.LastRtsErrCmd = CmdOffset;

                            /*
                             ** Stop the RTS from executing
                             */
                            SC_KillRts(RtsIndex);
                            CFE_EVS_SendEvent(SC_RTS_LNGTH_ERR_EID, CFE_EVS_EventType_ERROR,
                                              "Cmd Runs passed end of table, RTS %03d Aborted",
                                              SC_OperData.RtsCtrlBlckAddr->RtsNumber);

                        } /* end if the command runs off the end of the buffer */
                    }
                    else
                    { /* the command length is too large */

                        /* update the error information */
                        SC_OperData.HkPacket.Payload.RtsCmdErrCtr++;
                        SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr++;
                        SC_OperData.HkPacket.Payload.LastRtsErrSeq = SC_OperData.RtsCtrlBlckAddr->RtsNumber;
                        SC_OperData.HkPacket.Payload.LastRtsErrCmd = CmdOffset;

                        /* Stop the RTS from executing */
                        SC_KillRts(RtsIndex);
                        CFE_EVS_SendEvent(SC_RTS_CMD_LNGTH_ERR_EID, CFE_EVS_EventType_ERROR,
                                          "Invalid Length Field in RTS Command, RTS %03d Aborted. Length: %u, Max: %d",
                                          SC_OperData.RtsCtrlBlckAddr->RtsNumber,
                                          (unsigned int)(CmdLength - (uint16)SC_RTS_HEADER_SIZE), SC_PACKET_MAX_SIZE);

                    } /* end if the command length is invalid */
                }
                else
                { /* The command length is zero indicating no more cmds */
                    /*
                     **  This is not an error condition, so stop the RTS
                     */

                    /* Stop the RTS from executing */
                    SC_KillRts(RtsIndex);
                    if ((SC_OperData.RtsCtrlBlckAddr->RtsNumber) <= SC_LAST_RTS_WITH_EVENTS)
                    {
                        CFE_EVS_SendEvent(SC_RTS_COMPL_INF_EID, CFE_EVS_EventType_INFORMATION,
                                          "RTS %03d Execution Completed", SC_OperData.RtsCtrlBlckAddr->RtsNumber);
                    }
                }
            }
            else
            { /* The end of the RTS buffer has been reached... */
                /* Stop the RTS from executing */
                SC_KillRts(RtsIndex);
                if ((SC_OperData.RtsCtrlBlckAddr->RtsNumber) <= SC_LAST_RTS_WITH_EVENTS)
                {
                    CFE_EVS_SendEvent(SC_RTS_COMPL_INF_EID, CFE_EVS_EventType_INFORMATION,
                                      "RTS %03d Execution Completed", SC_OperData.RtsCtrlBlckAddr->RtsNumber);
                }

            } /* end if */

        } /* end if the RTS status is EXECUTING */

    } /* end if the RTS number is valid */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Gets the next ATS Command                                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_GetNextAtsCommand(void)
{
    uint16         AtsIndex;  /* ats array index */
    uint16         TimeIndex; /* a time index pointer */
    uint16         CmdIndex;  /* ats command array index */
    SC_AtsEntry_t *EntryPtr;

    if (SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING)
    {
        /*
         ** Get the information that is needed to find the next command
         */
        AtsIndex  = SC_ATS_NUM_TO_INDEX(SC_OperData.AtsCtrlBlckAddr->AtsNumber);
        TimeIndex = SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr + 1;

        /*
         ** Check to see if there are more ATS commands
         */
        if (TimeIndex < SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands)
        {
            /* get the information for the next command in the ATP control block */
            SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr = TimeIndex;
            SC_OperData.AtsCtrlBlckAddr->CmdNumber    = SC_AppData.AtsTimeIndexBuffer[AtsIndex][TimeIndex];

            /* update the next command time */
            CmdIndex =
                SC_AppData.AtsCmdIndexBuffer[AtsIndex][SC_ATS_CMD_NUM_TO_INDEX(SC_OperData.AtsCtrlBlckAddr->CmdNumber)];
            EntryPtr                       = (SC_AtsEntry_t *)&SC_OperData.AtsTblAddr[AtsIndex][CmdIndex];
            SC_AppData.NextCmdTime[SC_ATP] = SC_GetAtsEntryTime(&EntryPtr->Header);
        }
        else
        { /* the end is near... of the ATS buffer that is */

            /* stop the ATS */
            SC_KillAts();
            CFE_EVS_SendEvent(SC_ATS_COMPL_INF_EID, CFE_EVS_EventType_INFORMATION, "ATS %c Execution Completed",
                              (AtsIndex ? 'B' : 'A'));

            /* stop any switch that is pending */
            /* because we just ran out of commands and are stopping the ATS */
            /* and for the safe switch pend, that is a no-no */
            SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = false;

        } /* end if */
    }
    else if (SC_OperData.AtsCtrlBlckAddr->AtpState == SC_STARTING)
    {
        /*
         ** The SC_STARTING state is entered when an ATS inline
         ** switch has occurred and there are no commands to
         ** execute in the same second that the switch occurs.
         ** The state is transitioned here to SC_EXECUTING to
         ** commence execution of the new ATS on the next 1Hz
         ** command processing cycle.
         */
        SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;

    } /* end if ATS is EXECUTING*/
}
