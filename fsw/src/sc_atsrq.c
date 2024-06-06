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
 *     This file contains functions to handle all of the ATS
 *     executive requests and internal reuqests to control
 *     the ATP and ATSs.
 */

/**************************************************************************
 **
 ** Include section
 **
 **************************************************************************/

#include "cfe.h"
#include "sc_atsrq.h"
#include "sc_loads.h"
#include "sc_utils.h"
#include "sc_events.h"

/**************************************************************************
 **
 ** Functions
 **
 **************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Starts an ATS                                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_StartAtsCmd(const SC_StartAtsCmd_t *Cmd)
{
    SC_AtsNum_t        AtsNum;   /* ATS ID */
    SC_AtsIndex_t      AtsIndex; /* ATS array index */
    SC_AtsInfoTable_t *AtsInfoPtr;

    AtsNum = Cmd->Payload.AtsNum;

    /* validate ATS ID */
    if (SC_AtsNumIsValid(AtsNum))
    {
        /* convert ATS ID to array index */
        AtsIndex   = SC_AtsNumToIndex(AtsNum);
        AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

        /* make sure that there is no ATS running on the ATP */
        if (SC_OperData.AtsCtrlBlckAddr->AtpState == SC_Status_IDLE)
        {
            /* make sure the specified ATS is ready */
            if (AtsInfoPtr->NumberOfCommands > 0)
            {
                /* start the ats */
                if (SC_BeginAts(AtsIndex, 0))
                {
                    /* finish the ATP control block .. */
                    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_EXECUTING;

                    /* increment the command request counter */
                    SC_OperData.HkPacket.Payload.CmdCtr++;

                    CFE_EVS_SendEvent(SC_STARTATS_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                                      "ATS %c Execution Started", SC_IDX_AS_CHAR(AtsIndex));
                }
                else
                { /* could not start the ats, all commands were skipped */
                    /* event message was sent from SC_BeginAts */
                    /* increment the command request error counter */
                    /* SC_OperData.AtsCtrlBlckAddr->AtpState is updated in SC_KillAts */
                    SC_OperData.HkPacket.Payload.CmdErrCtr++;

                } /* end if */
            }
            else
            { /* the ats didn't have any commands in it */

                CFE_EVS_SendEvent(SC_STARTATS_CMD_NOT_LDED_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Start ATS Rejected: ATS %c Not Loaded", SC_IDX_AS_CHAR(AtsIndex));

                /* increment the command request error counter */
                SC_OperData.HkPacket.Payload.CmdErrCtr++;

            } /* end if */
        }
        else
        { /* the ATS is being used */

            CFE_EVS_SendEvent(SC_STARTATS_CMD_NOT_IDLE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Start ATS Rejected: ATP is not Idle");
            /* increment the command request error counter */
            SC_OperData.HkPacket.Payload.CmdErrCtr++;

        } /* end if */
    }
    else
    { /* the specified ATS id is not valid */

        CFE_EVS_SendEvent(SC_STARTATS_CMD_INVLD_ID_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Start ATS %u Rejected: Invalid ATS ID", SC_IDNUM_AS_UINT(AtsNum));

        /* increment the command request error counter */
        SC_OperData.HkPacket.Payload.CmdErrCtr++;

    } /* end if */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*   Stop the currently executing ATS                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_StopAtsCmd(const SC_StopAtsCmd_t *Cmd)
{
    int32 Result = SC_ERROR;

    /*
     ** Check if the ATS ID is valid
     */
    if (SC_AtsNumIsValid(SC_OperData.AtsCtrlBlckAddr->CurrAtsNum))
    {
        Result = CFE_SUCCESS;
    }

    if (Result == CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_STOPATS_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "ATS %c stopped",
                          SC_IDX_AS_CHAR(SC_AtsNumToIndex(SC_OperData.AtsCtrlBlckAddr->CurrAtsNum)));
    }
    else
    {
        CFE_EVS_SendEvent(SC_STOPATS_NO_ATS_INF_EID, CFE_EVS_EventType_INFORMATION, "There is no ATS running to stop");
    }

    /* Stop the ATS from executing */
    SC_KillAts();

    /* clear the global switch pend flag */
    SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = false;

    SC_OperData.HkPacket.Payload.CmdCtr++;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Function for starting an ATS                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool SC_BeginAts(SC_AtsIndex_t AtsIndex, uint16 TimeOffset)
{
    SC_AtsEntryHeader_t *         Entry;           /* ATS table entry pointer */
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;    /* ATS entry location in table */
    SC_AbsTimeTag_t               ListCmdTime = 0; /* list entry execution time */
    SC_SeqIndex_t                 TimeIndex;       /* the current time buffer index */
    SC_CommandIndex_t             CmdIndex;        /* ATS command index */
    bool                          ReturnCode;
    SC_AbsTimeTag_t               TimeToStartAts; /* the REAL time to start the ATS */
    uint16                        CmdsSkipped = 0;
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;

    /* validate ATS array index */
    if (!SC_AtsIndexIsValid(AtsIndex))
    {
        CFE_EVS_SendEvent(SC_BEGINATS_INVLD_INDEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Begin ATS error: invalid ATS index %u", SC_IDX_AS_UINT(AtsIndex));
        return false;
    }

    AtsInfoPtr     = SC_GetAtsInfoObject(AtsIndex);
    TimeToStartAts = SC_ComputeAbsTime(TimeOffset);

    /*
     ** Loop through the commands until a time tag is found that
     ** has a time greater than or equal to the current time OR
     ** all of the commands have been skipped
     */
    TimeIndex = SC_SEQUENCE_IDX_FIRST; /* pointer into the time index table */
    CmdIndex  = SC_COMMAND_IDX_C(0);   /* updated in loop */

    while (SC_IDX_WITHIN_LIMIT(TimeIndex, AtsInfoPtr->NumberOfCommands))
    {
        /* first get the cmd index at this list entry */
        CmdIndex = SC_CommandNumToIndex(SC_GetAtsCommandNumAtSeq(AtsIndex, TimeIndex)->CmdNum);
        if (!SC_AtsCommandIndexIsValid(CmdIndex))
        {
            SC_IDX_INCREMENT(TimeIndex);
            continue;
        }

        /* then get the entry index from the cmd index table */
        CmdOffsetRec = SC_GetAtsEntryOffsetForCmd(AtsIndex, CmdIndex);
        /* then get a pointer to the ATS entry data */
        Entry = &SC_GetAtsEntryAtOffset(AtsIndex, CmdOffsetRec->Offset)->Header;
        /* then get cmd execution time from the ATS entry */
        ListCmdTime = SC_GetAtsEntryTime(Entry);

        /* compare ATS start time to this list entry time */
        if (SC_CompareAbsTime(TimeToStartAts, ListCmdTime))
        {
            /* start time is greater than this list entry time */
            StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, CmdIndex);

            StatusEntryPtr->Status = SC_Status_SKIPPED;
            CmdsSkipped++;
            SC_IDX_INCREMENT(TimeIndex);
        }
        else
        {
            /* start time is less than or equal to this list entry */
            break;
        }
    }

    /*
     ** Check to see if the whole ATS was skipped
     */
    if (!SC_IDX_WITHIN_LIMIT(TimeIndex, AtsInfoPtr->NumberOfCommands))
    {
        CFE_EVS_SendEvent(SC_ATS_SKP_ALL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "All ATS commands were skipped, ATS stopped");

        /* stop the ats */
        SC_KillAts();

        ReturnCode = false;
    }
    else
    { /* there is at least one command to execute */

        /*
         ** Initialize the ATP Control Block.
         */
        /* leave the atp state alone, it will be updated by the caller */
        SC_OperData.AtsCtrlBlckAddr->CurrAtsNum   = SC_AtsIndexToNum(AtsIndex);
        SC_OperData.AtsCtrlBlckAddr->CmdNumber    = SC_CommandIndexToNum(CmdIndex);
        SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr = TimeIndex;

        /* send an event for number of commands skipped */
        CFE_EVS_SendEvent(SC_ATS_ERR_SKP_DBG_EID, CFE_EVS_EventType_DEBUG, "ATS started, skipped %d commands",
                          CmdsSkipped);
        /*
         ** Set the next command time for the ATP
         */
        SC_AppData.NextCmdTime[SC_Process_ATP] = ListCmdTime;

        ReturnCode = true;

    } /* end if */

    return ReturnCode;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Function for stopping the running ATS  & clearing data         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_KillAts(void)
{
    SC_AtsIndex_t      AtsIndex;
    SC_AtsInfoTable_t *AtsInfoPtr;

    /*
     * Check if the ATS ID is valid
     */
    AtsIndex = SC_AtsNumToIndex(SC_OperData.AtsCtrlBlckAddr->CurrAtsNum);
    if (SC_AtsIndexIsValid(AtsIndex) && SC_OperData.AtsCtrlBlckAddr->AtpState != SC_Status_IDLE)
    {
        /* Increment the ats use counter */
        AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

        AtsInfoPtr->AtsUseCtr++;
    }
    /*
     ** Reset the state in the atp control block
     */
    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_IDLE;

    /* reset the time of the next ats command */
    SC_AppData.NextCmdTime[SC_Process_ATP] = SC_MAX_TIME;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process an ATS Switch                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_SwitchAtsCmd(const SC_SwitchAtsCmd_t *Cmd)
{
    SC_AtsIndex_t      NewAtsIndex; /* the index of the ats to switch to*/
    SC_AtsInfoTable_t *AtsInfoPtr;

    /* make sure that an ATS is running on the ATP */
    if (SC_AtsNumIsValid(SC_OperData.AtsCtrlBlckAddr->CurrAtsNum) &&
        SC_OperData.AtsCtrlBlckAddr->AtpState == SC_Status_EXECUTING)
    {
        /* get the ATS to switch to */
        NewAtsIndex = SC_ToggleAtsIndex();
        AtsInfoPtr  = SC_GetAtsInfoObject(NewAtsIndex);

        /* Now check to see if the new ATS has commands in it */
        if (AtsInfoPtr->NumberOfCommands > 0)
        {
            /* set the global switch pend flag */
            SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = true;

            /* update the command counter */
            SC_OperData.HkPacket.Payload.CmdCtr++;

            CFE_EVS_SendEvent(SC_SWITCH_ATS_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "Switch ATS is Pending");
        }
        else
        { /* the other ATS does not have any commands in it */

            CFE_EVS_SendEvent(SC_SWITCH_ATS_CMD_NOT_LDED_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Switch ATS Failure: Destination ATS Not Loaded");

            /* update command error counter */
            SC_OperData.HkPacket.Payload.CmdErrCtr++;

            SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = false;

        } /* end if */
    }
    else
    { /* the ATP is not currently executing any commands */

        CFE_EVS_SendEvent(SC_SWITCH_ATS_CMD_IDLE_ERR_EID, CFE_EVS_EventType_ERROR, "Switch ATS Rejected: ATP is idle");

        /* update the command error counter */
        SC_OperData.HkPacket.Payload.CmdErrCtr++;

        SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = false;

    } /* end if */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Function for switching ATS's when each have commands in to      */
/* execute in the same second.                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_ServiceSwitchPend(void)
{
    SC_AtsIndex_t      NewAtsIndex; /* the ats index that we are switching to */
    SC_AtsIndex_t      OldAtsIndex; /* the ats index we are switching from */
    SC_AtsInfoTable_t *AtsInfoPtr;

    /*
     **  See if it is time to switch the ATS
     */
    if (SC_CompareAbsTime(SC_AppData.NextCmdTime[SC_Process_ATP], SC_AppData.CurrentTime))
    {
        /* make sure that an ATS is still running on the ATP */
        if (SC_OperData.AtsCtrlBlckAddr->AtpState == SC_Status_EXECUTING)
        {
            /* get the ATS number to switch to and from */
            OldAtsIndex = SC_AtsNumToIndex(SC_OperData.AtsCtrlBlckAddr->CurrAtsNum);
            NewAtsIndex = SC_ToggleAtsIndex();
            AtsInfoPtr  = SC_GetAtsInfoObject(NewAtsIndex);

            /* Now check to see if the new ATS has commands in it */
            if (AtsInfoPtr->NumberOfCommands > 0)
            {
                /* stop the current ATS */
                SC_KillAts();

                /*
                 ** Start the new ATS: Notice that we are starting the new
                 ** ATS with a one second offset from the current second,
                 ** This prevents commands that were executed the same
                 ** second that this command was received from being repeated.
                 */
                if (SC_BeginAts(NewAtsIndex, 1))
                {
                    SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_EXECUTING;

                    CFE_EVS_SendEvent(SC_ATS_SERVICE_SWTCH_INF_EID, CFE_EVS_EventType_INFORMATION,
                                      "ATS Switched from %c to %c", SC_IDX_AS_CHAR(OldAtsIndex),
                                      SC_IDX_AS_CHAR(NewAtsIndex));

                } /* end if */
            }
            else
            { /* the other ATS does not have any commands in it */

                CFE_EVS_SendEvent(SC_SERVICE_SWITCH_ATS_CMD_LDED_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Switch ATS Failure: Destination ATS is empty");
            } /* end if */
        }
        else
        { /* the ATP is not currently executing any commands */
            /* this should only happen if the switch flag gets */
            /* corrupted some how                              */

            CFE_EVS_SendEvent(SC_ATS_SERVICE_SWITCH_IDLE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Switch ATS Rejected: ATP is idle");
        } /* end if */

        /* in any case, this flag will need to be cleared */
        SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = false;

    } /* end if */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Switches from one ATS to the other when there are no commands   */
/* to be executed in the same second of the switch                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool SC_InlineSwitch(void)
{
    SC_AtsIndex_t      NewAtsIndex; /* the index of the ats to switch to*/
    SC_AtsIndex_t      OldAtsIndex; /* the index of the ats to switch from*/
    bool               ReturnCode;  /* return code for function */
    SC_AtsInfoTable_t *AtsInfoPtr;

    /* figure out which ATS to switch to */
    NewAtsIndex = SC_ToggleAtsIndex();

    /* Save the ATS number to switch FROM */
    OldAtsIndex = SC_AtsNumToIndex(SC_OperData.AtsCtrlBlckAddr->CurrAtsNum);

    /* Now check to see if the new ATS has commands in it */
    AtsInfoPtr = SC_GetAtsInfoObject(NewAtsIndex);
    if (AtsInfoPtr->NumberOfCommands > 0)
    {
        /*
         ** Stop the current ATS
         */
        SC_KillAts();

        /*
         ** Start up the other ATS
         */
        if (SC_BeginAts(NewAtsIndex, 0))
        {
            SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_STARTING;

            CFE_EVS_SendEvent(SC_ATS_INLINE_SWTCH_INF_EID, CFE_EVS_EventType_INFORMATION, "ATS Switched from %c to %c",
                              SC_IDX_AS_CHAR(OldAtsIndex), SC_IDX_AS_CHAR(NewAtsIndex));

            /*
             **  Update the command counter and return code
             */
            SC_OperData.HkPacket.Payload.CmdCtr++;
            ReturnCode = true;
        }
        else
        { /* all of the commands in the new ats were skipped */

            /*
             ** update the command error counter
             */
            SC_OperData.HkPacket.Payload.CmdErrCtr++;
            ReturnCode = false;

        } /* end if */
    }
    else
    { /* the other ATS does not have any commands in it */
        CFE_EVS_SendEvent(SC_ATS_INLINE_SWTCH_NOT_LDED_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Switch ATS Failure: Destination ATS Not Loaded");
        /*
         ** update the ATS error counter
         */
        SC_OperData.HkPacket.Payload.CmdErrCtr++;
        ReturnCode = false;

    } /* end if */

    /* clear out the global ground-switch pend flag */
    SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = false;

    return ReturnCode;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Jump an ATS forward in time                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_JumpAtsCmd(const SC_JumpAtsCmd_t *Cmd)
{
    SC_AtsEntryHeader_t *         Entry;        /* ATS table entry pointer */
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec; /* ATS entry location in table */
    SC_AbsTimeTag_t               JumpTime;     /* the time to jump to in the ATS */
    SC_AbsTimeTag_t               ListCmdTime;  /* list entry execution time */
    SC_AtsIndex_t                 AtsIndex;     /* index of the ATS that is running */
    SC_SeqIndex_t                 TimeIndex;    /* the current time buffer index */
    SC_CommandIndex_t             CmdIndex;     /* ATS command index (cmd num - 1) */
    char                          TimeBuffer[CFE_TIME_PRINTED_STRING_SIZE];
    CFE_TIME_SysTime_t            NewTime;
    uint16                        NumSkipped;
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;

    if (SC_OperData.AtsCtrlBlckAddr->AtpState == SC_Status_EXECUTING)
    {
        JumpTime   = Cmd->Payload.NewTime;
        AtsIndex   = SC_AtsNumToIndex(SC_OperData.AtsCtrlBlckAddr->CurrAtsNum);
        AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

        /*
         ** Loop through the commands until a time tag is found
         ** that has a time greater than or equal to the current time OR
         ** all of the commands have been skipped
         */
        TimeIndex  = SC_SEQUENCE_IDX_FIRST;
        CmdIndex   = SC_COMMAND_IDX_C(0); /* updated in loop */
        NumSkipped = 0;

        while (SC_IDX_WITHIN_LIMIT(TimeIndex, AtsInfoPtr->NumberOfCommands))
        {
            /* first get the cmd index at this list entry */
            CmdIndex = SC_CommandNumToIndex(SC_GetAtsCommandNumAtSeq(AtsIndex, TimeIndex)->CmdNum);
            if (!SC_AtsCommandIndexIsValid(CmdIndex))
            {
                SC_IDX_INCREMENT(TimeIndex);
                continue;
            }

            /* then get the entry index from the cmd index table */
            CmdOffsetRec = SC_GetAtsEntryOffsetForCmd(AtsIndex, CmdIndex);
            /* then get a pointer to the ATS entry data */
            Entry = &SC_GetAtsEntryAtOffset(AtsIndex, CmdOffsetRec->Offset)->Header;
            /* then get cmd execution time from the ATS entry */
            ListCmdTime = SC_GetAtsEntryTime(Entry);

            /* compare ATS jump time to this list entry time */
            if (SC_CompareAbsTime(JumpTime, ListCmdTime))
            {
                /* jump time is greater than this list entry time */

                /*
                ** If the ATS command is loaded and ready to run, then
                **  mark the command as being skipped
                **  if the command has any other status, SC_Status_SKIPPED, SC_Status_EXECUTED,
                **   etc, then leave the status alone.
                */
                StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, CmdIndex);
                if (StatusEntryPtr->Status == SC_Status_LOADED)
                {
                    StatusEntryPtr->Status = SC_Status_SKIPPED;
                    NumSkipped++;
                }

                SC_IDX_INCREMENT(TimeIndex);
            }
            else
            {
                /* jump time is less than or equal to this list entry */
                CFE_EVS_SendEvent(SC_JUMPATS_CMD_LIST_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "Jump Cmd: Jump time less than or equal to list entry %u", SC_IDX_AS_UINT(CmdIndex));
                break;
            }
        }

        /*
         ** Check to see if the whole ATS was skipped
         */
        if (!SC_IDX_WITHIN_LIMIT(TimeIndex, AtsInfoPtr->NumberOfCommands))
        {
            CFE_EVS_SendEvent(SC_JUMPATS_CMD_STOPPED_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Jump Cmd: All ATS commands were skipped, ATS stopped");

            SC_OperData.HkPacket.Payload.CmdErrCtr++;

            /* stop the ats */
            SC_KillAts();
        }
        else
        { /* there is at least one command to execute */

            /*
             ** Update the ATP Control Block entries.
             */
            SC_OperData.AtsCtrlBlckAddr->CmdNumber    = SC_CommandIndexToNum(CmdIndex);
            SC_OperData.AtsCtrlBlckAddr->TimeIndexPtr = TimeIndex;

            /*
             ** Set the next command time for the ATP
             */
            SC_AppData.NextCmdTime[SC_Process_ATP] = ListCmdTime;

            SC_OperData.HkPacket.Payload.CmdCtr++;

            /* print out the date in a readable format */
            NewTime.Seconds    = ListCmdTime;
            NewTime.Subseconds = 0;

            CFE_TIME_Print((char *)&TimeBuffer, NewTime);

            CFE_EVS_SendEvent(SC_JUMP_ATS_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "Next ATS command time in the ATP was set to %s", TimeBuffer);
            if (NumSkipped > 0)
            {
                /* We skipped come commands, but not all of them */
                CFE_EVS_SendEvent(SC_JUMP_ATS_SKIPPED_DBG_EID, CFE_EVS_EventType_DEBUG,
                                  "Jump Cmd: Skipped %d ATS commands", NumSkipped);
            }

        } /* end if */
    }

    else
    { /*  There is not a running ATS */

        CFE_EVS_SendEvent(SC_JUMPATS_CMD_NOT_ACT_ERR_EID, CFE_EVS_EventType_ERROR, "ATS Jump Failed: No active ATS");
        SC_OperData.HkPacket.Payload.CmdErrCtr++;

    } /* end if */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Continue ATS on Checksum Failure Cmd                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_ContinueAtsOnFailureCmd(const SC_ContinueAtsOnFailureCmd_t *Cmd)
{
    SC_AtsCont_Enum_t State;

    State = Cmd->Payload.ContinueState;

    if (State != SC_AtsCont_TRUE && State != SC_AtsCont_FALSE)
    {
        SC_OperData.HkPacket.Payload.CmdErrCtr++;

        CFE_EVS_SendEvent(SC_CONT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Continue ATS On Failure command  failed, invalid state: %lu", (unsigned long)State);
    }
    else
    {
        SC_OperData.HkPacket.Payload.ContinueAtsOnFailureFlag = State;

        SC_OperData.HkPacket.Payload.CmdCtr++;

        CFE_EVS_SendEvent(SC_CONT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Continue-ATS-On-Failure command, State: %lu",
                          (unsigned long)State);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Append to selected ATS                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_AppendAtsCmd(const SC_AppendAtsCmd_t *Cmd)
{
    SC_AtsIndex_t      AtsIndex; /* index (not ID) of target ATS */
    SC_AtsInfoTable_t *AtsInfoPtr;

    if (!SC_AtsNumIsValid(Cmd->Payload.AtsNum))
    {
        /* invalid target ATS selection */
        SC_OperData.HkPacket.Payload.CmdErrCtr++;

        CFE_EVS_SendEvent(SC_APPEND_CMD_ARG_ERR_EID, CFE_EVS_EventType_ERROR, "Append ATS error: invalid ATS ID = %u",
                          SC_IDNUM_AS_UINT(Cmd->Payload.AtsNum));

        return;
    }

    /* create base zero array index from base one ID value */
    AtsIndex   = SC_AtsNumToIndex(Cmd->Payload.AtsNum);
    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);

    if (AtsInfoPtr->NumberOfCommands == 0)
    {
        /* target ATS table is empty */
        SC_OperData.HkPacket.Payload.CmdErrCtr++;

        CFE_EVS_SendEvent(SC_APPEND_CMD_TGT_ERR_EID, CFE_EVS_EventType_ERROR, "Append ATS %c error: ATS table is empty",
                          SC_IDX_AS_CHAR(AtsIndex));
    }
    else if (SC_OperData.HkPacket.Payload.AppendEntryCount == 0)
    {
        /* append table is empty */
        SC_OperData.HkPacket.Payload.CmdErrCtr++;

        CFE_EVS_SendEvent(SC_APPEND_CMD_SRC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Append ATS %c error: Append table is empty", SC_IDX_AS_CHAR(AtsIndex));
    }
    else if ((AtsInfoPtr->AtsSize + SC_AppData.AppendWordCount) > SC_ATS_BUFF_SIZE32)
    {
        /* not enough room in ATS buffer for Append table data */
        SC_OperData.HkPacket.Payload.CmdErrCtr++;

        CFE_EVS_SendEvent(SC_APPEND_CMD_FIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Append ATS %c error: ATS size = %d, Append size = %d, ATS buffer = %d",
                          SC_IDX_AS_CHAR(AtsIndex), (int)AtsInfoPtr->AtsSize, SC_AppData.AppendWordCount,
                          SC_ATS_BUFF_SIZE32);
    }
    else
    {
        /* store ATS selection from most recent ATS Append command */
        SC_OperData.HkPacket.Payload.AppendCmdArg = Cmd->Payload.AtsNum;

        /* copy append data and re-calc timing data */
        SC_ProcessAppend(AtsIndex);

        /* increment command success counter */
        SC_OperData.HkPacket.Payload.CmdCtr++;

        CFE_EVS_SendEvent(SC_APPEND_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Append ATS %c command: %d ATS entries appended", SC_IDX_AS_CHAR(AtsIndex),
                          SC_OperData.HkPacket.Payload.AppendEntryCount);
    }
}
