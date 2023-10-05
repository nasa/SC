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
#include "cfe_tbl_msg.h"
#include "sc_app.h"
#include "sc_cmds.h"
#include "sc_atsrq.h"
#include "sc_rtsrq.h"
#include "sc_loads.h"
#include "sc_utils.h"
#include "sc_state.h"
#include "sc_msgids.h"
#include "sc_events.h"
#include "sc_version.h"
#include "sc_rts.h"

/**************************************************************************
 **
 ** Functions
 **
 **************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Processes a command from the ATS                                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_ProcessAtpCmd(void)
{
    int32             EntryIndex; /* ATS entry location in table */
    uint8             AtsIndex;   /* ATS selection index */
    uint32            CmdIndex;   /* ATS command index */
    char              TempAtsChar;
    CFE_Status_t      Result;
    bool              AbortATS = false;
    SC_AtsEntry_t *   EntryPtr;
    CFE_SB_MsgId_t    MessageID   = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;
    bool              ChecksumValid;

    /*
     ** The following conditions must be met before the ATS command will be
     ** executed:
     ** 1.) The next time is <= the current time
     ** 2.) The next processor number = ATP
     ** 3.) The atp is currently EXECUTING
     */

    if ((SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING) && (SC_AppData.NextProcNumber == SC_ATP) &&
        (!SC_CompareAbsTime(SC_AppData.NextCmdTime[SC_ATP], SC_AppData.CurrentTime)))
    {
        /*
         ** Get a pointer to the next ats command
         */
        AtsIndex   = SC_ATS_NUM_TO_INDEX(SC_OperData.AtsCtrlBlckAddr->AtsNumber); /* remember 0..1 */
        CmdIndex   = SC_ATS_CMD_NUM_TO_INDEX(SC_OperData.AtsCtrlBlckAddr->CmdNumber);
        EntryIndex = SC_AppData.AtsCmdIndexBuffer[AtsIndex][CmdIndex];
        EntryPtr   = (SC_AtsEntry_t *)&SC_OperData.AtsTblAddr[AtsIndex][EntryIndex];

        /*
         ** Make sure the command has not been executed, skipped or has any other bad status
         */
        if (SC_OperData.AtsCmdStatusTblAddr[AtsIndex][CmdIndex] == SC_LOADED)
        {
            /*
             ** Make sure the command number matches what the command
             ** number is supposed to be
             */
            if (EntryPtr->Header.CmdNumber == (SC_ATS_CMD_INDEX_TO_NUM(CmdIndex)))
            {
                /*
                 * Check the checksum on the command
                 * If this feature is disabled, just skip and assume valid
                 * Note that the checksum will be re-computed when the message is sent.
                 */
                ChecksumValid = SC_AppData.EnableHeaderUpdate;
                if (!SC_AppData.EnableHeaderUpdate)
                {
                    /* If header update is NOT enabled, confirm this table entry has a valid checksum already */
                    CFE_MSG_ValidateChecksum(&EntryPtr->Msg, &ChecksumValid);
                }
                if (ChecksumValid)
                {
                    /*
                     ** Count the command for the rate limiter
                     */
                    SC_OperData.NumCmdsSec++;

                    /*
                     **  First check to see if the command is a switch command,
                     **  if it is, then execute the command now instead of sending
                     **  it out on the Software Bus (this is the only exception to
                     **  way stored commands are sent out).
                     **  A switch command located within an ATS is handled differently
                     **  than a switch command by the ground controller. If the switch
                     **  command is by the ground controller, SC waits until the next
                     **  second to prevent overlapping of two ATSs from duplicating
                     **  commands. If the Switch command is located within an ATS,
                     **  SC immediately executes the switch command.
                     */

                    CFE_MSG_GetMsgId(&EntryPtr->Msg, &MessageID);
                    CFE_MSG_GetFcnCode(&EntryPtr->Msg, &CommandCode);

                    if (CommandCode == SC_SWITCH_ATS_CC && CFE_SB_MsgIdToValue(MessageID) == SC_CMD_MID)
                    {
                        /*
                         ** call the ground switch module
                         */
                        if (SC_InlineSwitch())
                        {
                            /*
                             ** Increment the counter and update the status for
                             ** this command
                             */
                            SC_OperData.AtsCmdStatusTblAddr[AtsIndex][CmdIndex] = SC_EXECUTED;
                            SC_OperData.HkPacket.Payload.AtsCmdCtr++;
                        }
                        else
                        { /* the switch failed for some reason */

                            SC_OperData.AtsCmdStatusTblAddr[AtsIndex][CmdIndex] = SC_FAILED_DISTRIB;
                            SC_OperData.HkPacket.Payload.AtsCmdErrCtr++;
                            SC_OperData.HkPacket.Payload.LastAtsErrSeq = SC_OperData.AtsCtrlBlckAddr->AtsNumber;
                            SC_OperData.HkPacket.Payload.LastAtsErrCmd = SC_OperData.AtsCtrlBlckAddr->CmdNumber;

                        } /* end if */
                    }
                    else
                    {
                        Result = CFE_SB_TransmitMsg(&EntryPtr->Msg, SC_AppData.EnableHeaderUpdate);

                        if (Result == CFE_SUCCESS)
                        {
                            /* The command sent OK */
                            SC_OperData.AtsCmdStatusTblAddr[AtsIndex][CmdIndex] = SC_EXECUTED;
                            SC_OperData.HkPacket.Payload.AtsCmdCtr++;
                        }
                        else
                        { /* the command had Software Bus problems */
                            SC_OperData.AtsCmdStatusTblAddr[AtsIndex][CmdIndex] = SC_FAILED_DISTRIB;
                            SC_OperData.HkPacket.Payload.AtsCmdErrCtr++;
                            SC_OperData.HkPacket.Payload.LastAtsErrSeq = SC_OperData.AtsCtrlBlckAddr->AtsNumber;
                            SC_OperData.HkPacket.Payload.LastAtsErrCmd = SC_OperData.AtsCtrlBlckAddr->CmdNumber;

                            CFE_EVS_SendEvent(SC_ATS_DIST_ERR_EID, CFE_EVS_EventType_ERROR,
                                              "ATS Command Distribution Failed, Cmd Number: %d, SB returned: 0x%08X",
                                              EntryPtr->Header.CmdNumber, (unsigned int)Result);

                            /* Mark this ATS for abortion */
                            AbortATS = true;
                        }
                    }
                }
                else
                { /* the checksum failed */
                    /*
                     ** Send an event message to report the invalid command status
                     */
                    CFE_EVS_SendEvent(SC_ATS_CHKSUM_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "ATS Command Failed Checksum: Command #%d Skipped", EntryPtr->Header.CmdNumber);
                    /*
                     ** Increment the ATS error counter
                     */
                    SC_OperData.HkPacket.Payload.AtsCmdErrCtr++;

                    /*
                     ** Update the last ATS error information structure
                     */
                    SC_OperData.HkPacket.Payload.LastAtsErrSeq = SC_OperData.AtsCtrlBlckAddr->AtsNumber;
                    SC_OperData.HkPacket.Payload.LastAtsErrCmd = SC_OperData.AtsCtrlBlckAddr->CmdNumber;

                    /* update the command status index table */
                    SC_OperData.AtsCmdStatusTblAddr[AtsIndex][CmdIndex] = SC_FAILED_CHECKSUM;

                    if (SC_OperData.HkPacket.Payload.ContinueAtsOnFailureFlag == false)
                    {
                        /* Mark this ATS for abortion */
                        AbortATS = true;
                    }
                } /* end checksum test */
            }
            else
            { /* the command number does not match */
                /*
                 ** Send an event message to report the invalid command status
                 */

                CFE_EVS_SendEvent(SC_ATS_MSMTCH_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "ATS Command Number Mismatch: Command Skipped, expected: %d received: %d",
                                  (int)SC_ATS_CMD_INDEX_TO_NUM(CmdIndex), EntryPtr->Header.CmdNumber);
                /*
                 ** Increment the ATS error counter
                 */
                SC_OperData.HkPacket.Payload.AtsCmdErrCtr++;

                /*
                 ** Update the last ATS error information structure
                 */
                SC_OperData.HkPacket.Payload.LastAtsErrSeq = SC_OperData.AtsCtrlBlckAddr->AtsNumber;
                SC_OperData.HkPacket.Payload.LastAtsErrCmd = SC_OperData.AtsCtrlBlckAddr->CmdNumber;

                /* update the command status index table */
                SC_OperData.AtsCmdStatusTblAddr[AtsIndex][CmdIndex] = SC_SKIPPED;

                /* Mark this ATS for abortion */
                AbortATS = true;
            } /* end if  the command number does not match */
        }
        else /* command isn't marked as loaded */
        {
            /*
             ** Send an event message to report the invalid command status
             */
            CFE_EVS_SendEvent(SC_ATS_SKP_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid ATS Command Status: Command Skipped, Status: %d",
                              SC_OperData.AtsCmdStatusTblAddr[AtsIndex][CmdIndex]);
            /*
             ** Increment the ATS error counter
             */
            SC_OperData.HkPacket.Payload.AtsCmdErrCtr++;

            /*
             ** Update the last ATS error information structure
             */
            SC_OperData.HkPacket.Payload.LastAtsErrSeq = SC_OperData.AtsCtrlBlckAddr->AtsNumber;
            SC_OperData.HkPacket.Payload.LastAtsErrCmd = SC_OperData.AtsCtrlBlckAddr->CmdNumber;

            /* Do Not Mark this ATS for abortion. The command could be marked as EXECUTED
               if we alerady jumped back in time */

        } /* end if */

        if (AbortATS == true)
        {
            if (SC_OperData.AtsCtrlBlckAddr->AtsNumber == SC_ATSA)
            {
                TempAtsChar = 'A';
            }
            else
            {
                TempAtsChar = 'B';
            }

            CFE_EVS_SendEvent(SC_ATS_ABT_ERR_EID, CFE_EVS_EventType_ERROR, "ATS %c Aborted", TempAtsChar);

            /* Stop the ATS from executing */
            SC_KillAts();
            SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag = false;
        }
        else
        {
            /*
            ** Get the next ATS command set up to execute
            */
            SC_GetNextAtsCommand();
        }

    } /* end if next ATS command time */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Processes a command from an RTS                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_ProcessRtpCommand(void)
{
    SC_RtsEntry_t *EntryPtr;  /* a pointer to an RTS entry header */
    uint16         RtsIndex;  /* the RTS index for the cmd */
    uint16         CmdOffset; /* the location of the cmd    */
    CFE_Status_t   Result;
    bool           ChecksumValid;

    /*
     ** The following conditions must be met before a RTS command is executed:
     ** 1.) The next command time must be <= the current time
     ** 2.) The next processor number must be SC_RTP
     ** 3.) The RTS number in the RTP control block must be valid and
     ** 4.) the RTS must be EXECUTING
     */

    if ((SC_AppData.NextCmdTime[SC_AppData.NextProcNumber] <= SC_AppData.CurrentTime) &&
        (SC_AppData.NextProcNumber == SC_RTP) && (SC_OperData.RtsCtrlBlckAddr->RtsNumber > 0) &&
        (SC_OperData.RtsCtrlBlckAddr->RtsNumber <= SC_NUMBER_OF_RTS) &&
        (SC_OperData.RtsInfoTblAddr[SC_RTS_NUM_TO_INDEX(SC_OperData.RtsCtrlBlckAddr->RtsNumber)].RtsStatus ==
         SC_EXECUTING))
    {
        /*
         ** Count the command for the rate limiter
         ** even if the command fails
         */
        SC_OperData.NumCmdsSec++;

        /* convert the RTS number so that it can be directly indexed into the table*/
        RtsIndex = SC_RTS_NUM_TO_INDEX(SC_OperData.RtsCtrlBlckAddr->RtsNumber);

        /*
         ** Get the Command offset within the RTS
         */
        CmdOffset = SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr;

        /*
         ** Get a pointer to the RTS entry using the RTS number and the offset
         */
        EntryPtr = (SC_RtsEntry_t *)&SC_OperData.RtsTblAddr[RtsIndex][CmdOffset];

        ChecksumValid = SC_AppData.EnableHeaderUpdate;
        if (!SC_AppData.EnableHeaderUpdate)
        {
            /* If header update is NOT enabled, confirm this table entry has a valid checksum already */
            CFE_MSG_ValidateChecksum(&EntryPtr->Msg, &ChecksumValid);
        }
        if (ChecksumValid)
        {
            /*
             ** Try Sending the command on the Software Bus
             */

            Result = CFE_SB_TransmitMsg(&EntryPtr->Msg, SC_AppData.EnableHeaderUpdate);

            if (Result == CFE_SUCCESS)
            {
                /* the command was sent OK */
                SC_OperData.HkPacket.Payload.RtsCmdCtr++;
                SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr++;

                /*
                 ** Get the next command.
                 */
                SC_GetNextRtsCommand();
            }
            else
            { /* the software bus return code was bad */

                /*
                 ** Send an event message to report the invalid command status
                 */
                CFE_EVS_SendEvent(SC_RTS_DIST_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "RTS %03d Command Distribution Failed: RTS Stopped. SB returned 0x%08X",
                                  (int)SC_OperData.RtsCtrlBlckAddr->RtsNumber, (unsigned int)Result);

                SC_OperData.HkPacket.Payload.RtsCmdErrCtr++;
                SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr++;
                SC_OperData.HkPacket.Payload.LastRtsErrSeq = SC_OperData.RtsCtrlBlckAddr->RtsNumber;
                SC_OperData.HkPacket.Payload.LastRtsErrCmd = CmdOffset;

                /*
                 ** Stop the RTS from executing
                 */
                SC_KillRts(RtsIndex);

            } /* end if */
        }
        else
        { /* the checksum failed */

            /*
             ** Send an event message to report the invalid command status
             */
            CFE_EVS_SendEvent(SC_RTS_CHKSUM_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RTS %03d Command Failed Checksum: RTS Stopped",
                              (int)SC_OperData.RtsCtrlBlckAddr->RtsNumber);
            /*
            ** Update the RTS command error counter and last RTS error info
            */
            SC_OperData.HkPacket.Payload.RtsCmdErrCtr++;
            SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr++;
            SC_OperData.HkPacket.Payload.LastRtsErrSeq = SC_OperData.RtsCtrlBlckAddr->RtsNumber;
            SC_OperData.HkPacket.Payload.LastRtsErrCmd = CmdOffset;

            /*
             ** Stop the RTS from executing
             */
            SC_KillRts(RtsIndex);
        } /* end if */
    }     /* end if */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Sends Housekeeping Data                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_SendHkPacket(void)
{
    uint16 i;

    /*
     ** fill in the free bytes in each ATS
     */
    SC_OperData.HkPacket.Payload.AtpFreeBytes[SC_ATS_NUM_TO_INDEX(SC_ATSA)] =
        (SC_ATS_BUFF_SIZE32 * SC_BYTES_IN_WORD) -
        (SC_OperData.AtsInfoTblAddr[SC_ATS_NUM_TO_INDEX(SC_ATSA)].AtsSize * SC_BYTES_IN_WORD);
    SC_OperData.HkPacket.Payload.AtpFreeBytes[SC_ATS_NUM_TO_INDEX(SC_ATSB)] =
        (SC_ATS_BUFF_SIZE32 * SC_BYTES_IN_WORD) -
        (SC_OperData.AtsInfoTblAddr[SC_ATS_NUM_TO_INDEX(SC_ATSB)].AtsSize * SC_BYTES_IN_WORD);

    /*
     **
     ** fill in the ATP Control Block information
     **
     */

    SC_OperData.HkPacket.Payload.AtsNumber = SC_OperData.AtsCtrlBlckAddr->AtsNumber;

    SC_OperData.HkPacket.Payload.AtpState       = SC_OperData.AtsCtrlBlckAddr->AtpState;
    SC_OperData.HkPacket.Payload.AtpCmdNumber   = SC_OperData.AtsCtrlBlckAddr->CmdNumber;
    SC_OperData.HkPacket.Payload.SwitchPendFlag = SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag;

    SC_OperData.HkPacket.Payload.NextAtsTime = SC_AppData.NextCmdTime[SC_ATP];

    /*
     ** Fill out the RTP control block information
     */

    SC_OperData.HkPacket.Payload.NumRtsActive = SC_OperData.RtsCtrlBlckAddr->NumRtsActive;
    SC_OperData.HkPacket.Payload.RtsNumber    = SC_OperData.RtsCtrlBlckAddr->RtsNumber;
    SC_OperData.HkPacket.Payload.NextRtsTime  = SC_AppData.NextCmdTime[SC_RTP];

    /*
     ** Fill out the RTS status bit mask
     ** First clear out the status mask
     */
    for (i = 0; i < (SC_NUMBER_OF_RTS + (SC_NUMBER_OF_RTS_IN_UINT16 - 1)) / SC_NUMBER_OF_RTS_IN_UINT16; i++)
    {
        SC_OperData.HkPacket.Payload.RtsExecutingStatus[i] = 0;
        SC_OperData.HkPacket.Payload.RtsDisabledStatus[i]  = 0;

    } /* end for */

    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        if (SC_OperData.RtsInfoTblAddr[i].DisabledFlag == true)
        {
            SC_OperData.HkPacket.Payload.RtsDisabledStatus[i / SC_NUMBER_OF_RTS_IN_UINT16] |=
                (1 << (i % SC_NUMBER_OF_RTS_IN_UINT16));
        }
        if (SC_OperData.RtsInfoTblAddr[i].RtsStatus == SC_EXECUTING)
        {
            SC_OperData.HkPacket.Payload.RtsExecutingStatus[i / SC_NUMBER_OF_RTS_IN_UINT16] |=
                (1 << (i % SC_NUMBER_OF_RTS_IN_UINT16));
        }
    } /* end for */

    /* send the status packet */
    CFE_SB_TimeStampMsg(&SC_OperData.HkPacket.TlmHeader.Msg);
    CFE_SB_TransmitMsg(&SC_OperData.HkPacket.TlmHeader.Msg, true);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Send HK Command                                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_SendHkCmd(const SC_SendHkCmd_t *Cmd)
{
    /* set during init to power on or processor reset auto-exec RTS */
    if (SC_AppData.AutoStartRTS != 0)
    {
        /* make sure the selected auto-exec RTS is enabled */
        if (SC_OperData.RtsInfoTblAddr[SC_RTS_NUM_TO_INDEX(SC_AppData.AutoStartRTS)].RtsStatus == SC_LOADED)
        {
            SC_OperData.RtsInfoTblAddr[SC_RTS_NUM_TO_INDEX(SC_AppData.AutoStartRTS)].DisabledFlag = false;
        }

        /* send ground cmd to have SC start the RTS */
        SC_AutoStartRts(SC_AppData.AutoStartRTS);

        /* only start it once */
        SC_AppData.AutoStartRTS = 0;
    }

    /* request from health and safety for housekeeping status */
    SC_SendHkPacket();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset Counters Command                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_ResetCountersCmd(const SC_ResetCountersCmd_t *Cmd)
{
    CFE_EVS_SendEvent(SC_RESET_DEB_EID, CFE_EVS_EventType_DEBUG, "Reset counters command");

    SC_OperData.HkPacket.Payload.CmdCtr          = 0;
    SC_OperData.HkPacket.Payload.CmdErrCtr       = 0;
    SC_OperData.HkPacket.Payload.AtsCmdCtr       = 0;
    SC_OperData.HkPacket.Payload.AtsCmdErrCtr    = 0;
    SC_OperData.HkPacket.Payload.RtsCmdCtr       = 0;
    SC_OperData.HkPacket.Payload.RtsCmdErrCtr    = 0;
    SC_OperData.HkPacket.Payload.RtsActiveCtr    = 0;
    SC_OperData.HkPacket.Payload.RtsActiveErrCtr = 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* 1Hz Wakeup Command                                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_OneHzWakeupCmd(const SC_OneHzWakeupCmd_t *Cmd)
{
    bool IsThereAnotherCommandToExecute = false;

    /*
     * Time to execute a command in the SC memory
     */
    do
    {
        /*
         *  Check to see if there is an ATS switch Pending, if so service it.
         */
        if (SC_OperData.AtsCtrlBlckAddr->SwitchPendFlag == true)
        {
            SC_ServiceSwitchPend();
        }

        if (SC_AppData.NextProcNumber == SC_ATP)
        {
            SC_ProcessAtpCmd();
        }
        else
        {
            if (SC_AppData.NextProcNumber == SC_RTP)
            {
                SC_ProcessRtpCommand();
            }
        }

        SC_UpdateNextTime();
        if ((SC_AppData.NextProcNumber == SC_NONE) ||
            (SC_AppData.NextCmdTime[SC_AppData.NextProcNumber] > SC_AppData.CurrentTime))
        {
            SC_OperData.NumCmdsSec         = 0;
            IsThereAnotherCommandToExecute = false;
        }
        else /* Command needs to run immediately */
        {
            if (SC_OperData.NumCmdsSec >= SC_MAX_CMDS_PER_SEC)
            {
                SC_OperData.NumCmdsSec         = 0;
                IsThereAnotherCommandToExecute = false;
            }
            else
            {
                IsThereAnotherCommandToExecute = true;
            }
        }
    } while (IsThereAnotherCommandToExecute);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* No Op Command                                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_NoopCmd(const SC_NoopCmd_t *Cmd)
{
    SC_OperData.HkPacket.Payload.CmdCtr++;
    CFE_EVS_SendEvent(SC_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "No-op command. Version %d.%d.%d.%d",
                      SC_MAJOR_VERSION, SC_MINOR_VERSION, SC_REVISION, SC_MISSION_REV);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Table Manage Request Command (sent by cFE Table Services)       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_ManageTableCmd(const SC_ManageTableCmd_t *Cmd)
{
    int32 ArrayIndex;
    int32 TableID = Cmd->Payload.Parameter;

    /* Manage selected table as appropriate for each table type */
    if ((TableID >= SC_TBL_ID_ATS_0) && (TableID < (SC_TBL_ID_ATS_0 + SC_NUMBER_OF_ATS)))
    {
        ArrayIndex = TableID - SC_TBL_ID_ATS_0;
        SC_ManageAtsTable(ArrayIndex);
    }
    else if (TableID == SC_TBL_ID_APPEND)
    {
        SC_ManageTable(APPEND, -1);
    }
    else if ((TableID >= SC_TBL_ID_RTS_0) && (TableID < (SC_TBL_ID_RTS_0 + SC_NUMBER_OF_RTS)))
    {
        ArrayIndex = TableID - SC_TBL_ID_RTS_0;
        SC_ManageRtsTable(ArrayIndex);
    }
    else if (TableID == SC_TBL_ID_RTS_INFO)
    {
        /* No need to release dump only table pointer */
        CFE_TBL_Manage(SC_OperData.RtsInfoHandle);
    }
    else if (TableID == SC_TBL_ID_RTP_CTRL)
    {
        /* No need to release dump only table pointer */
        CFE_TBL_Manage(SC_OperData.RtsCtrlBlckHandle);
    }
    else if (TableID == SC_TBL_ID_ATS_INFO)
    {
        /* No need to release dump only table pointer */
        CFE_TBL_Manage(SC_OperData.AtsInfoHandle);
    }
    else if (TableID == SC_TBL_ID_ATP_CTRL)
    {
        /* No need to release dump only table pointer */
        CFE_TBL_Manage(SC_OperData.AtsCtrlBlckHandle);
    }
    else if ((TableID >= SC_TBL_ID_ATS_CMD_0) && (TableID < (SC_TBL_ID_ATS_CMD_0 + SC_NUMBER_OF_ATS)))
    {
        /* No need to release dump only table pointer */
        ArrayIndex = TableID - SC_TBL_ID_ATS_CMD_0;
        CFE_TBL_Manage(SC_OperData.AtsCmdStatusHandle[ArrayIndex]);
    }
    else
    {
        /* Invalid table ID */
        CFE_EVS_SendEvent(SC_TABLE_MANAGE_ID_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Table manage command packet error: table ID = %d", (int)TableID);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Allow cFE Table Services to manage loadable RTS table           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_ManageRtsTable(int32 ArrayIndex)
{
    /* validate array index */
    if (ArrayIndex >= SC_NUMBER_OF_RTS)
    {
        CFE_EVS_SendEvent(SC_TABLE_MANAGE_RTS_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RTS table manage error: invalid RTS index %d", ArrayIndex);
        return;
    }

    SC_ManageTable(RTS, ArrayIndex);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Allow cFE Table Services to manage loadable ATS table           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_ManageAtsTable(int32 ArrayIndex)
{
    /* validate array index */
    if (ArrayIndex >= SC_NUMBER_OF_ATS)
    {
        CFE_EVS_SendEvent(SC_TABLE_MANAGE_ATS_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ATS table manage error: invalid ATS index %d", ArrayIndex);
        return;
    }

    SC_ManageTable(ATS, ArrayIndex);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Allow cFE Table Services to manage loadable table    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_ManageTable(SC_TableType type, int32 ArrayIndex)
{
    CFE_Status_t     Result;
    CFE_TBL_Handle_t TblHandle;
    uint32 **        TblAddr;
    void *           TblPtrNew;

    switch (type)
    {
        case ATS:
            TblHandle = SC_OperData.AtsTblHandle[ArrayIndex];
            TblAddr   = &SC_OperData.AtsTblAddr[ArrayIndex];
            break;
        case RTS:
            TblHandle = SC_OperData.RtsTblHandle[ArrayIndex];
            TblAddr   = &SC_OperData.RtsTblAddr[ArrayIndex];
            break;
        case APPEND:
        default:
            TblHandle = SC_OperData.AppendTblHandle;
            TblAddr   = &SC_OperData.AppendTblAddr;
            break;
    }

    /* Release table data pointer */
    CFE_TBL_ReleaseAddress(TblHandle);

    /* Allow cFE to manage table */
    CFE_TBL_Manage(TblHandle);

    /* Re-acquire table data pointer */
    Result   = CFE_TBL_GetAddress(&TblPtrNew, TblHandle);
    *TblAddr = TblPtrNew; /* Note that CFE_TBL_GetAddress() sets this to NULL if it fails */
    if (Result == CFE_TBL_INFO_UPDATED)
    {
        /* Process new table data */
        if (type == ATS)
        {
            SC_LoadAts(ArrayIndex);
        }
        else if (type == RTS)
        {
            SC_LoadRts(ArrayIndex);
        }
        else
        {
            SC_UpdateAppend();
        }
    }
    else if ((Result != CFE_SUCCESS) && (Result != CFE_TBL_ERR_NEVER_LOADED))
    {
        /* Ignore successful dump or validate and cmds before first activate. */
        if (type == ATS)
        {
            CFE_EVS_SendEvent(SC_TABLE_MANAGE_ATS_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ATS table manage process error: ATS = %d, Result = 0x%X",
                              (int)SC_RTS_INDEX_TO_NUM(ArrayIndex), (unsigned int)Result);
        }
        else if (type == RTS)
        {
            CFE_EVS_SendEvent(SC_TABLE_MANAGE_RTS_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RTS table manage process error: RTS = %d, Result = 0x%X",
                              (int)SC_RTS_INDEX_TO_NUM(ArrayIndex), (unsigned int)Result);
        }
        else
        {
            CFE_EVS_SendEvent(SC_TABLE_MANAGE_APPEND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ATS Append table manage process error: Result = 0x%X", (unsigned int)Result);
        }
    }

} /* End SC_ManageTable() */
