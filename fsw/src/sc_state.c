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
 *   This file contains functions to handle getting the next time
 *   or wakeup count of commands for the ATP and RTP, as well as
 *   updating the time for Stored Command.
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
#define SC_INVALID_RTS_INDEX ((SC_RtsIndex_t) {-1})

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Gets the wakeup count of the next RTS command                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_GetNextRtsTime(void)
{
    int16              i;             /* loop counter MUST be SIGNED !*/
    SC_RtsIndex_t      NextRts;       /* the next rts to schedule */
    uint32             NextWakeupCnt; /* the next wakeup count for the RTS */
    SC_RtsInfoEntry_t *RtsInfoPtr;

    NextRts  = SC_INVALID_RTS_INDEX;
    NextWakeupCnt = SC_MAX_WAKEUP_CNT;

    /*
     ** Go through the table backwards to account for the RTS priority
     ** Lower number RTSs get higher priority
     ** Backward processing ensures selection of the lowest RTS number
     ** when multiple RTSs have the same next command wakeup count
     */
    for (i = SC_NUMBER_OF_RTS - 1; i >= 0; i--)
    {
        RtsInfoPtr = SC_GetRtsInfoObject(SC_RTS_IDX_C(i));
        if (RtsInfoPtr->RtsStatus == SC_Status_EXECUTING)
        {
            if (RtsInfoPtr->NextCommandTgtWakeup <= NextWakeupCnt)
            {
                NextWakeupCnt = RtsInfoPtr->NextCommandTgtWakeup;
                NextRts  = SC_RTS_IDX_C(i);
            } /* end if */
        }     /* end if */
    }         /* end for */

    if (!SC_RtsIndexIsValid(NextRts))
    {
        SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RTS_NUM_NULL;
        SC_AppData.NextCmdTime[SC_Process_RTP]  = SC_MAX_WAKEUP_CNT;
    }
    else
    {
        SC_OperData.RtsCtrlBlckAddr->CurrRtsNum = SC_RtsIndexToNum(NextRts);
        SC_AppData.NextCmdTime[SC_Process_RTP]  = NextWakeupCnt;
    } /* end if */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Gets the next RTS Command                                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_GetNextRtsCommand(void)
{
    SC_RtsIndex_t      RtsIndex;
    SC_EntryOffset_t   CmdOffset;
    SC_EntryOffset_t   PendingOffset;
    SC_RtsEntry_t *    EntryPtr;
    CFE_MSG_Size_t     CmdLength = 0;
    SC_RtsInfoEntry_t *RtsInfoPtr;

    /*
     ** Make sure that the RTP is executing some RTS
     */
    RtsIndex = SC_RtsNumToIndex(SC_OperData.RtsCtrlBlckAddr->CurrRtsNum);
    if (SC_RtsIndexIsValid(RtsIndex))
    {
        RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

        /*
         ** Find out if the RTS is EXECUTING or just STARTED
         */
        if (RtsInfoPtr->RtsStatus == SC_Status_EXECUTING)
        {
            /*
             ** Get the information needed to find the next command
             */
            CmdOffset = RtsInfoPtr->NextCommandPtr;
            EntryPtr  = SC_GetRtsEntryAtOffset(RtsIndex, CmdOffset);

            CFE_MSG_GetSize(CFE_MSG_PTR(EntryPtr->Msg), &CmdLength);
            CmdLength += SC_RTS_HEADER_SIZE;

            /*
             ** calculate the new command offset and new command length
             ** Cmd Length is in bytes, so we convert it to words
             ** (plus 1 to round byte len up to word len)
             */

            CmdOffset = SC_EntryOffsetAdvance(CmdOffset, CmdLength);
            /*
             ** if the end of the buffer is not reached.
             ** This check is made to make sure that at least the minimum
             ** Sized packet fits in the buffer. It assures we are not reading
             ** bogus length info from other data.
             */

            /* If at least the header for a command plus the RTS header can fit in the buffer */
            if (SC_IDX_WITHIN_LIMIT(CmdOffset, 1 + SC_RTS_BUFF_SIZE32 - SC_RTS_HDR_WORDS))
            {
                /*
                 ** Get the next RTS command
                 */
                EntryPtr = SC_GetRtsEntryAtOffset(RtsIndex, CmdOffset);

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
                        PendingOffset = SC_EntryOffsetAdvance(CmdOffset, CmdLength);
                        if (SC_IDX_WITHIN_LIMIT(PendingOffset, 1 + SC_RTS_BUFF_SIZE32))
                        {
                            /*
                             ** Everything passed!
                             ** Update the proper next command wakeup count for that RTS
                             */
                            RtsInfoPtr->NextCommandTgtWakeup = SC_ComputeAbsWakeup(EntryPtr->Header.WakeupCount);

                            /*
                             ** Update the appropriate RTS info table current command pointer
                             */
                            RtsInfoPtr->NextCommandPtr = CmdOffset;
                        }
                        else
                        { /* the command runs past the end of the buffer */

                            /*
                             ** Having a command that runs off of the end of the buffer
                             ** is an error condition, so record it
                             */
                            SC_OperData.HkPacket.Payload.RtsCmdErrCtr++;
                            RtsInfoPtr->CmdErrCtr++;
                            SC_OperData.HkPacket.Payload.LastRtsErrSeq = SC_OperData.RtsCtrlBlckAddr->CurrRtsNum;
                            SC_OperData.HkPacket.Payload.LastRtsErrCmd = CmdOffset;

                            /*
                             ** Stop the RTS from executing
                             */
                            SC_KillRts(RtsIndex);
                            CFE_EVS_SendEvent(SC_RTS_LNGTH_ERR_EID, CFE_EVS_EventType_ERROR,
                                              "Cmd Runs passed end of table, RTS %03u Aborted",
                                              SC_IDNUM_AS_UINT(SC_OperData.RtsCtrlBlckAddr->CurrRtsNum));

                        } /* end if the command runs off the end of the buffer */
                    }
                    else
                    { /* the command length is too large */

                        /* update the error information */
                        SC_OperData.HkPacket.Payload.RtsCmdErrCtr++;
                        RtsInfoPtr->CmdErrCtr++;
                        SC_OperData.HkPacket.Payload.LastRtsErrSeq = SC_OperData.RtsCtrlBlckAddr->CurrRtsNum;
                        SC_OperData.HkPacket.Payload.LastRtsErrCmd = CmdOffset;

                        /* Stop the RTS from executing */
                        SC_KillRts(RtsIndex);
                        CFE_EVS_SendEvent(SC_RTS_CMD_LNGTH_ERR_EID, CFE_EVS_EventType_ERROR,
                                          "Invalid Length Field in RTS Command, RTS %03u Aborted. Length: %u, Max: %d",
                                          SC_IDNUM_AS_UINT(SC_OperData.RtsCtrlBlckAddr->CurrRtsNum),
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
                    if (SC_RtsNumHasEvent(SC_OperData.RtsCtrlBlckAddr->CurrRtsNum))
                    {
                        CFE_EVS_SendEvent(SC_RTS_COMPL_INF_EID, CFE_EVS_EventType_INFORMATION,
                                          "RTS %03u Execution Completed",
                                          SC_IDNUM_AS_UINT(SC_OperData.RtsCtrlBlckAddr->CurrRtsNum));
                    }
                }
            }
            else
            { /* The end of the RTS buffer has been reached... */
                /* Stop the RTS from executing */
                SC_KillRts(RtsIndex);
                if (SC_RtsNumHasEvent(SC_OperData.RtsCtrlBlckAddr->CurrRtsNum))
                {
                    CFE_EVS_SendEvent(SC_RTS_COMPL_INF_EID, CFE_EVS_EventType_INFORMATION,
                                      "RTS %03u Execution Completed",
                                      SC_IDNUM_AS_UINT(SC_OperData.RtsCtrlBlckAddr->CurrRtsNum));
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
    SC_AtsIndex_t                 AtsIndex;  /* ats array index */
    SC_SeqIndex_t                 TimeIndex; /* a time index pointer */
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;
    SC_AtsEntry_t *               EntryPtr;
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdNumRecord_t *        AtsCmdNumRec;
    SC_CommandIndex_t             CurrCmdIndex;

    if (SC_OperData.AtsCtrlBlckAddr->AtpState == SC_Status_EXECUTING)
    {
        /*
         ** Get the information that is needed to find the next command
         */
        AtsIndex   = SC_AtsNumToIndex(SC_OperData.AtsCtrlBlckAddr->CurrAtsNum);
        TimeIndex  = SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr;
        AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);
        SC_IDX_INCREMENT(TimeIndex);

        /*
         ** Check to see if there are more ATS commands
         */
        if (SC_IDX_WITHIN_LIMIT(TimeIndex, AtsInfoPtr->NumberOfCommands))
        {
            /* get the information for the next command in the ATP control block */
            AtsCmdNumRec = SC_GetAtsCommandNumAtSeq(AtsIndex, TimeIndex);
            CurrCmdIndex = SC_CommandNumToIndex(AtsCmdNumRec->CmdNum);

            SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr = TimeIndex;
            SC_OperData.AtsCtrlBlckAddr->CmdNumber    = AtsCmdNumRec->CmdNum;

            /* update the next command time */
            CmdOffsetRec = SC_GetAtsEntryOffsetForCmd(AtsIndex, CurrCmdIndex);

            EntryPtr = SC_GetAtsEntryAtOffset(AtsIndex, CmdOffsetRec->Offset);

            SC_AppData.NextCmdTime[SC_Process_ATP] = SC_GetAtsEntryTime(&EntryPtr->Header);
        }
        else
        { /* the end is near... of the ATS buffer that is */

            /* stop the ATS */
            SC_KillAts();
            CFE_EVS_SendEvent(SC_ATS_COMPL_INF_EID, CFE_EVS_EventType_INFORMATION, "ATS %c Execution Completed",
                              SC_IDX_AS_CHAR(AtsIndex));

            /* stop any switch that is pending */
            /* because we just ran out of commands and are stopping the ATS */
            /* and for the safe switch pend, that is a no-no */
            SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = false;

        } /* end if */
    }
    else if (SC_OperData.AtsCtrlBlckAddr->AtpState == SC_Status_STARTING)
    {
        /*
         ** The SC_Status_STARTING state is entered when an ATS inline
         ** switch has occurred and there are no commands to
         ** execute in the same second that the switch occurs.
         ** The state is transitioned here to SC_Status_EXECUTING to
         ** commence execution of the new ATS on the next wakeup
         ** command processing cycle.
         */
        SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_EXECUTING;

    } /* end if ATS is EXECUTING*/
}
