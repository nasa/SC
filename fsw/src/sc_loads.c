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
 *   This file contains functions to handle validation of TBL tables,
 *   as well as setting up Stored Command's internal data structures for
 *   those tables
 */

/**************************************************************************
 **
 ** Include section
 **
 **************************************************************************/

#include "cfe.h"
#include "sc_app.h"
#include "sc_loads.h"
#include "sc_atsrq.h"
#include "sc_utils.h"
#include "sc_events.h"
#include <string.h>

/**************************************************************************
 **
 ** Local #defines
 **
 **************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Load the ATS from its table to memory                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_LoadAts(uint16 AtsIndex)
{
    uint16         AtsEntryWords; /* current ats entry length in words */
    uint16         AtsCmdNum;     /* current ats entry command number */
    uint16         AtsEntryIndex; /* index into the load for current ats entry */
    SC_AtsEntry_t *EntryPtr;      /* a pointer to an ats entry */
    uint32 *       AtsTablePtr;   /* pointer to the start of the Ats table */
    CFE_MSG_Size_t MessageSize     = 0;
    int32          Result          = CFE_SUCCESS;
    bool           StillProcessing = true;

    /* validate ATS array index */
    if (AtsIndex >= SC_NUMBER_OF_ATS)
    {
        CFE_EVS_SendEvent(SC_LOADATS_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR, "ATS load error: invalid ATS index %d",
                          AtsIndex);
        return;
    }

    /*
     ** Initialize all structrures
     */
    SC_InitAtsTables(AtsIndex);

    /* initialize pointers and counters */
    AtsTablePtr   = SC_OperData.AtsTblAddr[AtsIndex];
    AtsEntryIndex = 0;

    while (StillProcessing)
    {
        /*
         ** Make sure that the pointer as well as the primary packet
         ** header fit in the buffer, so a G.P fault is not caused.
         */
        if (AtsEntryIndex < SC_ATS_BUFF_SIZE32)
        {
            /* get a pointer to the ats command in the table */
            EntryPtr = (SC_AtsEntry_t *)&AtsTablePtr[AtsEntryIndex];

            /* get the next command number from the buffer */
            AtsCmdNum = EntryPtr->Header.CmdNumber;

            if (AtsCmdNum == 0)
            {
                /* end of the load reached */
                Result          = CFE_SUCCESS;
                StillProcessing = false;
            }

            /* make sure the CmdPtr can fit in a whole Ats Cmd Header at the very least */
            else if (AtsEntryIndex > (SC_ATS_BUFF_SIZE32 - SC_ATS_HDR_WORDS))
            {
                /* even the smallest command will not fit in the buffer */
                Result          = SC_ERROR;
                StillProcessing = false;
            } /* else if the cmd number is valid and the command */
            /* has not already been loaded                     */
            else if (AtsCmdNum <= SC_MAX_ATS_CMDS &&
                     SC_OperData.AtsCmdStatusTblAddr[AtsIndex][SC_ATS_CMD_NUM_TO_INDEX(AtsCmdNum)] == SC_EMPTY)
            {
                /* get message size */
                CFE_MSG_GetSize(&EntryPtr->Msg, &MessageSize);

                /* if the length of the command is valid */
                if (MessageSize >= SC_PACKET_MIN_SIZE && MessageSize <= SC_PACKET_MAX_SIZE)
                {
                    /* get the length of the entry in WORDS (plus 1 to round byte len up to word len) */
                    AtsEntryWords = ((MessageSize + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD) + SC_ATS_HDR_NOPKT_WORDS;

                    /* if the command does not run off of the end of the buffer */
                    if (AtsEntryIndex + AtsEntryWords <= SC_ATS_BUFF_SIZE32)
                    {
                        /* set the command pointer in the command index table */
                        /* CmdNum starts at one....                          */

                        SC_AppData.AtsCmdIndexBuffer[AtsIndex][SC_ATS_CMD_NUM_TO_INDEX(AtsCmdNum)] = AtsEntryIndex;

                        /* set the command status to loaded in the command status table */
                        SC_OperData.AtsCmdStatusTblAddr[AtsIndex][SC_ATS_CMD_NUM_TO_INDEX(AtsCmdNum)] = SC_LOADED;

                        /* increment the number of commands loaded */
                        SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands++;

                        /* increment the ats_entry index to the next ats entry */
                        AtsEntryIndex = AtsEntryIndex + AtsEntryWords;
                    }
                    else
                    { /* the command runs off the end of the buffer */
                        Result          = SC_ERROR;
                        StillProcessing = false;
                    } /* end if */
                }
                else
                { /* the command length was invalid */
                    Result          = SC_ERROR;
                    StillProcessing = false;
                } /* end if */
            }
            else
            { /* the cmd number is invalid */
                Result          = SC_ERROR;
                StillProcessing = false;
            } /* end if */
        }
        else
        {
            /*
             ** We encountered a load exactly as long as the buffer.
             ** AtsEntryIndex cannot exceed SC_ATS_BUFF_SIZE32 here.
             ** A command pointer over the end of the buffer will be
             ** identified above before AtsEntryIndex is incremented.
             ** Consequently this block does not require verifying
             ** AtsEntryIndex >= SC_ATS_BUFF_SIZE32.
             */

            Result          = CFE_SUCCESS;
            StillProcessing = false;
        } /*end else */
    }     /* end while */

    /*
     **   Now the commands are parsed through, need to build the tables
     **   if the load was a sucess, need to build the tables
     */
    /* if the load finished without errors and there was at least one command */
    if ((Result == CFE_SUCCESS) && (SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands > 0))
    {
        /* record the size of the load in the ATS info table */
        SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize = AtsEntryIndex; /* size in 32-bit WORDS */

        /* build the time index table */
        SC_BuildTimeIndexTable(AtsIndex);
    }
    else
    { /* there was an error */
        SC_InitAtsTables(AtsIndex);
    } /* end if */

} /* end SC_LoadAts */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Builds the time table for the ATS buffer                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_BuildTimeIndexTable(uint16 AtsIndex)
{
    int32 i;
    int32 ListLength;

    /* validate ATS array index */
    if (AtsIndex >= SC_NUMBER_OF_ATS)
    {
        CFE_EVS_SendEvent(SC_BUILD_TIME_IDXTBL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Build time index table error: invalid ATS index %d", AtsIndex);
        return;
    }

    /* initialize sorted list length */
    ListLength = 0;

    /* initialize sorted list contents */
    for (i = 0; i < SC_MAX_ATS_CMDS; i++)
    {
        SC_AppData.AtsTimeIndexBuffer[AtsIndex][i] = SC_INVALID_CMD_NUMBER;

        /* add in-use command entries to time sorted list */
        if (SC_AppData.AtsCmdIndexBuffer[AtsIndex][i] != SC_ERROR)
        {
            SC_Insert(AtsIndex, i, ListLength);
            ListLength++;
        }
    }

} /* end SC_BuildTimeIndexTable */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Inserts and element into a sorted list                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_Insert(uint16 AtsIndex, uint32 NewCmdIndex, uint32 ListLength)
{
    SC_AtsEntryHeader_t *EntryHeader;    /* ATS table entry pointer */
    SC_AbsTimeTag_t      NewCmdTime = 0; /* new command execution time */
    SC_AbsTimeTag_t      ListCmdTime;    /* list entry execution time */
    uint32               CmdIndex;       /* ATS command index (cmd num - 1) */
    uint32               EntryIndex;     /* ATS entry location in table */
    int32                TimeBufIndex;   /* this must be signed */

    /* validate ATS array index */
    if (AtsIndex >= SC_NUMBER_OF_ATS)
    {
        CFE_EVS_SendEvent(SC_INSERTATS_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ATS insert error: invalid ATS index %d", AtsIndex);
        return;
    }

    /* get execution time for new list entry */
    if (ListLength > 0)
    {
        /* first get the entry index in the selected ATS table for the new command */
        EntryIndex = SC_AppData.AtsCmdIndexBuffer[AtsIndex][NewCmdIndex];
        /* then get a pointer to the ATS entry */
        EntryHeader = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][EntryIndex];
        /* then get the execution time from the ATS entry for the new command */
        NewCmdTime = SC_GetAtsEntryTime(EntryHeader);
    }

    /* start at last element in the sorted by time list */
    TimeBufIndex = ListLength - 1;

    while (TimeBufIndex >= 0)
    {
        /* first get the cmd index for this list entry */
        CmdIndex = SC_ATS_CMD_NUM_TO_INDEX(SC_AppData.AtsTimeIndexBuffer[AtsIndex][TimeBufIndex]);
        /* then get the entry index from the ATS table */
        EntryIndex = SC_AppData.AtsCmdIndexBuffer[AtsIndex][CmdIndex];
        /* then get a pointer to the ATS entry data */
        EntryHeader = (SC_AtsEntryHeader_t *)&SC_OperData.AtsTblAddr[AtsIndex][EntryIndex];
        /* then get cmd execution time from the ATS entry */
        ListCmdTime = SC_GetAtsEntryTime(EntryHeader);

        /* compare time for this list entry to time for new cmd */
        if (SC_CompareAbsTime(ListCmdTime, NewCmdTime))
        {
            /* new cmd will execute before this list entry */

            /* move this list entry to make room for new cmd */
            SC_AppData.AtsTimeIndexBuffer[AtsIndex][TimeBufIndex + 1] =
                SC_AppData.AtsTimeIndexBuffer[AtsIndex][TimeBufIndex];

            /* back up to previous list entry (ok if -1) */
            TimeBufIndex--;
        }
        else
        {
            /* new cmd will execute at same time or after this list entry */
            break;
        }
    }

    /*
    ** TimeBufIndex is now one slot before the target slot...
    **   if new cmd time is earlier than all other entries
    **     then TimeBufIndex is -1 and all others have been moved
    **   else only entries with later times have been moved
    ** In either case, there is an empty slot next to TimeBufIndex
    */
    SC_AppData.AtsTimeIndexBuffer[AtsIndex][TimeBufIndex + 1] = SC_ATS_CMD_INDEX_TO_NUM(NewCmdIndex);
} /* end SC_Insert */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Clears out Ats Tables before a load                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_InitAtsTables(uint16 AtsIndex)
{
    int32 i;

    /* validate ATS array index */
    if (AtsIndex >= SC_NUMBER_OF_ATS)
    {
        CFE_EVS_SendEvent(SC_INIT_ATSTBL_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ATS table init error: invalid ATS index %d", AtsIndex);
        return;
    }

    /* loop through and set the ATS tables to zero */
    for (i = 0; i < SC_MAX_ATS_CMDS; i++)
    {
        SC_AppData.AtsCmdIndexBuffer[AtsIndex][i]    = SC_ERROR;
        SC_OperData.AtsCmdStatusTblAddr[AtsIndex][i] = SC_EMPTY;
        SC_AppData.AtsTimeIndexBuffer[AtsIndex][i]   = SC_INVALID_CMD_NUMBER;
    }

    /* initialize the pointers and counters   */
    SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize          = 0;
    SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands = 0;

} /* end SC_InitAtsTables */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initializes the info table entry for an RTS                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_LoadRts(uint16 RtsIndex)
{
    /* validate RTS array index */
    if (RtsIndex < SC_NUMBER_OF_RTS)
    {
        /* Clear out the RTS info table */
        SC_OperData.RtsInfoTblAddr[RtsIndex].RtsStatus       = SC_LOADED;
        SC_OperData.RtsInfoTblAddr[RtsIndex].UseCtr          = 0;
        SC_OperData.RtsInfoTblAddr[RtsIndex].CmdCtr          = 0;
        SC_OperData.RtsInfoTblAddr[RtsIndex].CmdErrCtr       = 0;
        SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandTime = 0;
        SC_OperData.RtsInfoTblAddr[RtsIndex].NextCommandPtr  = 0;

        /* Make sure the RTS is disabled */
        SC_OperData.RtsInfoTblAddr[RtsIndex].DisabledFlag = true;
    }
    else
    {
        CFE_EVS_SendEvent(SC_LOADRTS_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RTS table init error: invalid RTS index %d", RtsIndex);
        return;
    }

} /* SC_LoadRts */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Validate ATS table data                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 SC_ValidateAts(void *TableData)
{
    int32 Result;

    /* Common ATS table verify function needs size of this table */
    Result = SC_VerifyAtsTable((uint32 *)TableData, SC_ATS_BUFF_SIZE32);

    return (Result);

} /* end SC_ValidateAts */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Parses the RTS to make sure it looks good                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool SC_ParseRts(uint32 Buffer32[])
{
    uint16         i;
    bool           Done;
    bool           Error;
    SC_RtsEntry_t *EntryPtr;
    CFE_MSG_Size_t CmdSize    = 0;
    uint16         IndexDelta = 0;
    CFE_SB_MsgId_t MessageID  = CFE_SB_INVALID_MSG_ID;

    i    = 0;
    Done = Error = false;
    while (Error == false && Done == false)
    {

        /*
         ** Check to see if a minimum command fits within an RTS
         */
        if (i <= (SC_RTS_BUFF_SIZE32 - SC_RTS_HDR_WORDS))
        {

            /*
             ** Cast a header to the RTS buffer current location
             ** and get the size of the packet
             */
            EntryPtr = (SC_RtsEntry_t *)&Buffer32[i];

            CFE_MSG_GetSize(&EntryPtr->Msg, &CmdSize);

            /* Add header size, round up to boundary, convert to index delta  */
            IndexDelta = (CmdSize + SC_RTS_HEADER_SIZE + SC_ROUND_UP_BYTES) / sizeof(Buffer32[0]);

            CFE_MSG_GetMsgId(&EntryPtr->Msg, &MessageID);

            if (!CFE_SB_IsValidMsgId(MessageID))
            {
                if (EntryPtr->Header.TimeTag == 0)
                {
                    Done = true; /* assumed end of file */
                }
                else
                {
                    CFE_EVS_SendEvent(SC_RTS_INVLD_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "RTS cmd loaded with invalid MID at %d", i);
                    Error = true; /* invalid message id */
                }
            }
            else
            {
                /* check to see if the length field in the RTS is valid */
                if (CmdSize < SC_PACKET_MIN_SIZE || CmdSize > SC_PACKET_MAX_SIZE)
                {
                    CFE_EVS_SendEvent(SC_RTS_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "RTS cmd loaded with invalid length at %d, len: %d", i, (int)CmdSize);

                    Error = true; /* Length error */
                }

                else if ((i + IndexDelta) > SC_RTS_BUFF_SIZE32)
                {
                    CFE_EVS_SendEvent(SC_RTS_LEN_BUFFER_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "RTS cmd at %d runs off end of buffer", i);
                    Error = true; /* command runs off of the end of the buffer */
                }

                else if ((i + IndexDelta) == SC_RTS_BUFF_SIZE32)
                {
                    Done = true;
                }
                else
                { /* command fits in buffer */

                    i += IndexDelta;

                } /* end if */

            } /* endif */
        }
        else
        { /* command does not fit in the buffer */

            /*
             ** If it looks like there is data, reject the load,
             ** if it looks empty then we are done
             */
            if (Buffer32[i] == 0)
            {
                Done = true;
            }
            else
            {
                CFE_EVS_SendEvent(SC_RTS_LEN_TOO_LONG_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "RTS cmd loaded won't fit in buffer at %d", i);
                Error = true;
            }
        } /* endif */

    } /* endwhile */

    /*
     ** finished, report results
     */

    /* If Error was true   , then SC_ParseRts must return false    */
    return (!Error);

} /* end SC_ParseRts */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Validate an RTS                                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 SC_ValidateRts(void *TableData)
{

    uint32 *TableDataPtr;
    int32   Result = CFE_SUCCESS;

    TableDataPtr = (uint32 *)TableData;

    /*
     ** make a rough check on the first command to see if there is
     ** something in the buffer
     */
    if (SC_ParseRts(TableDataPtr) == false)
    {
        /* event message is put out by Parse RTS */
        Result = SC_ERROR;
    }

    return (Result);

} /* end SC_ValidateRts */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Validate Append ATS table data                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 SC_ValidateAppend(void *TableData)
{
    int32 Result;

    /* Common ATS table verify function needs size of this table */
    Result = SC_VerifyAtsTable((uint32 *)TableData, SC_APPEND_BUFF_SIZE32);

    return (Result);

} /* end SC_ValidateAppend */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Have new Append ATS table data, update Append ATS Info table    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_UpdateAppend(void)
{
    SC_AtsEntry_t *EntryPtr;
    CFE_MSG_Size_t CommandBytes = 0;
    int32          CommandWords;
    int32          EntryIndex      = 0;
    int32          EntryCount      = 0;
    bool           StillProcessing = true;

    /* Count Append ATS table entries and get total size */
    while (StillProcessing)
    {
        if (EntryIndex >= SC_APPEND_BUFF_SIZE32)
        {
            /* End of Append ATS table buffer */
            StillProcessing = false;
        }
        else
        {
            EntryPtr = (SC_AtsEntry_t *)&SC_OperData.AppendTblAddr[EntryIndex];

            if ((EntryPtr->Header.CmdNumber == 0) || (EntryPtr->Header.CmdNumber > SC_MAX_ATS_CMDS))
            {
                /* End of valid command numbers */
                StillProcessing = false;
            }
            else
            {
                CFE_MSG_GetSize(&EntryPtr->Msg, &CommandBytes);
                CommandWords = (CommandBytes + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD;

                if ((CommandBytes < SC_PACKET_MIN_SIZE) || (CommandBytes > SC_PACKET_MAX_SIZE))
                {
                    /* Entry command packet must have a valid length */
                    StillProcessing = false;
                }
                else if ((EntryIndex + SC_ATS_HDR_NOPKT_WORDS + CommandWords) > SC_APPEND_BUFF_SIZE32)
                {
                    /* Entry command packet must fit within ATS append table buffer */
                    StillProcessing = false;
                }
                else
                {
                    /* Compute buffer index for next Append ATS table entry */
                    EntryIndex += (SC_ATS_HDR_NOPKT_WORDS + CommandWords);
                    EntryCount++;
                }
            }
        }
    }

    /* Results will also be reported in HK */
    SC_OperData.HkPacket.AppendLoadCount++;
    SC_OperData.HkPacket.AppendEntryCount = EntryCount;
    SC_OperData.HkPacket.AppendByteCount  = EntryIndex * SC_BYTES_IN_ATS_APPEND_ENTRY;
    SC_AppData.AppendWordCount            = EntryIndex;

    CFE_EVS_SendEvent(SC_UPDATE_APPEND_EID, CFE_EVS_EventType_INFORMATION,
                      "Update Append ATS Table: load count = %d, command count = %d, byte count = %d",
                      SC_OperData.HkPacket.AppendLoadCount, (int)EntryCount, (int)EntryIndex * SC_BYTES_IN_WORD);
    return;

} /* end SC_UpdateAppend */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Append contents of Append ATS table to indicated ATS table      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_ProcessAppend(uint16 AtsIndex)
{
    SC_AtsEntry_t *EntryPtr;
    CFE_MSG_Size_t CommandBytes = 0;
    int32          CommandWords;
    int32          EntryIndex;
    int32          i;
    uint16         CmdIndex;

    /* validate ATS array index */
    if (AtsIndex >= SC_NUMBER_OF_ATS)
    {
        CFE_EVS_SendEvent(SC_PROCESS_APPEND_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ATS process append error: invalid ATS index %d", AtsIndex);
        return;
    }

    /* save index of free area at end of ATS table data */
    EntryIndex = SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize;

    /* copy Append table data to end of ATS table data */
    memcpy(&SC_OperData.AtsTblAddr[AtsIndex][EntryIndex], SC_OperData.AppendTblAddr,
           SC_AppData.AppendWordCount * SC_BYTES_IN_WORD);

    /* update size of ATS table data */
    SC_OperData.AtsInfoTblAddr[AtsIndex].AtsSize += SC_AppData.AppendWordCount;

    /* add appended entries to ats process tables */
    for (i = 0; i < SC_OperData.HkPacket.AppendEntryCount; i++)
    {
        /* get pointer to next appended entry */
        EntryPtr = (SC_AtsEntry_t *)&SC_OperData.AtsTblAddr[AtsIndex][EntryIndex];

        /* convert base one cmd number to base zero index */
        CmdIndex = SC_ATS_CMD_NUM_TO_INDEX(EntryPtr->Header.CmdNumber);

        /* count only new commands, not replaced commands */
        if (SC_OperData.AtsCmdStatusTblAddr[AtsIndex][CmdIndex] == SC_EMPTY)
        {
            SC_OperData.AtsInfoTblAddr[AtsIndex].NumberOfCommands++;
        }

        /* update array of pointers to ats entries */
        SC_AppData.AtsCmdIndexBuffer[AtsIndex][CmdIndex]    = EntryIndex;
        SC_OperData.AtsCmdStatusTblAddr[AtsIndex][CmdIndex] = SC_LOADED;

        /* update entry index to point to the next entry */
        CFE_MSG_GetSize(&EntryPtr->Msg, &CommandBytes);
        CommandWords = (CommandBytes + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD;
        EntryIndex += (SC_ATS_HDR_NOPKT_WORDS + CommandWords);
    }

    /* rebuild time sorted list of commands */
    SC_BuildTimeIndexTable(AtsIndex);

    /* did we just append to an ats that was executing? */
    if ((SC_OperData.AtsCtrlBlckAddr->AtpState == SC_EXECUTING) &&
        (SC_OperData.AtsCtrlBlckAddr->AtsNumber == (SC_ATS_INDEX_TO_NUM(AtsIndex))))
    {
        /*
        ** re-start the ats -- this will go thru the process of skipping
        **  past due entries (all of the old entries that had already
        **  been executed and all of the new entries with an old time)
        */
        if (SC_BeginAts(AtsIndex, 0))
        {
            SC_OperData.AtsCtrlBlckAddr->AtpState = SC_EXECUTING;
        }
    }

    /* notify cFE that we have modified the ats table */
    CFE_TBL_Modified(SC_OperData.AtsTblHandle[AtsIndex]);

    return;

} /* end SC_ProcessAppend */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Verify contents of ATS table data                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 SC_VerifyAtsTable(uint32 *Buffer32, int32 BufferWords)
{
    int32 Result       = CFE_SUCCESS;
    int32 BufferIndex  = 0;
    int32 CommandCount = 0;
    int32 i;

    bool StillProcessing = true;

    /* Initialize all command numbers as unused */
    for (i = 0; i < SC_MAX_ATS_CMDS; i++)
    {
        SC_OperData.AtsDupTestArray[i] = SC_DUP_TEST_UNUSED;
    }

    while (StillProcessing)
    {
        /* Verify the ATS table entry at the current buffer index */
        Result = SC_VerifyAtsEntry(Buffer32, BufferIndex, BufferWords);

        if (Result == SC_ERROR)
        {
            /* Entry at current buffer index is invalid */
            StillProcessing = false;
        }
        else if (Result == CFE_SUCCESS)
        {
            /* No more entries -- end of buffer or cmd num = 0 */
            StillProcessing = false;
        }
        else
        {
            /* Result is size (in words) of this entry */
            BufferIndex += Result;
            CommandCount++;
        }
    }

    if (Result == CFE_SUCCESS)
    {
        if (CommandCount == 0)
        {
            /* Table must contain at least one valid entry */
            Result = SC_ERROR;

            CFE_EVS_SendEvent(SC_VERIFY_ATS_MPT_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Verify ATS Table error: table is empty");
        }
        else
        {
            CFE_EVS_SendEvent(SC_VERIFY_ATS_EID, CFE_EVS_EventType_INFORMATION,
                              "Verify ATS Table: command count = %d, byte count = %d", (int)CommandCount,
                              (int)BufferIndex * SC_BYTES_IN_WORD);
        }
    }

    return (Result);

} /* end SC_VerifyAtsTable */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify contents of one ATS table entry                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 SC_VerifyAtsEntry(uint32 *Buffer32, int32 EntryIndex, int32 BufferWords)
{
    SC_AtsEntry_t *EntryPtr;
    CFE_MSG_Size_t CommandBytes = 0;
    int32          CommandWords;
    int32          Result = CFE_SUCCESS;

    EntryPtr = (SC_AtsEntry_t *)&Buffer32[EntryIndex];

    /*
    ** Verify the ATS table entry located at the indicated buffer offset
    */
    if (EntryIndex >= BufferWords)
    {
        /*
        ** The process logic will prevent the index from ever exceeding
        **  the size of the buffer due to bad table data content.  Still,
        **  we must include the "greater than" in the test above to
        **  protect against our own potential coding errors.
        */

        /* All done -- end of ATS buffer */
        Result = CFE_SUCCESS;
    }
    else if (EntryPtr->Header.CmdNumber == 0)
    {
        /*
        ** If there is at least one word remaining in the buffer then it
        **  is OK to test the command number without fear of accessing
        **  past the end of valid data because the command number is the
        **  first element in an ATS entry structure.
        */

        /* All done -- end of in-use portion of buffer */
        Result = CFE_SUCCESS;
    }
    else if (EntryPtr->Header.CmdNumber > SC_MAX_ATS_CMDS)
    {
        /* Error -- invalid command number */
        Result = SC_ERROR;

        CFE_EVS_SendEvent(SC_VERIFY_ATS_NUM_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Verify ATS Table error: invalid command number: buf index = %d, cmd num = %d",
                          (int)EntryIndex, EntryPtr->Header.CmdNumber);
    }
    else if ((EntryIndex + SC_ATS_HDR_WORDS) > BufferWords)
    {
        /* Error -- not enough room for smallest possible ATS entry */
        Result = SC_ERROR;

        CFE_EVS_SendEvent(SC_VERIFY_ATS_END_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Verify ATS Table error: buffer full: buf index = %d, cmd num = %d, buf words = %d",
                          (int)EntryIndex, EntryPtr->Header.CmdNumber, (int)BufferWords);
    }
    else
    {
        /* Start with the byte length of the command packet */
        CFE_MSG_GetSize(&EntryPtr->Msg, &CommandBytes);

        /* Convert packet byte length to word length (round up odd bytes) */
        CommandWords = (CommandBytes + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD;

        if ((CommandBytes < SC_PACKET_MIN_SIZE) || (CommandBytes > SC_PACKET_MAX_SIZE))
        {
            /* Error -- invalid command packet byte length */
            Result = SC_ERROR;

            CFE_EVS_SendEvent(SC_VERIFY_ATS_PKT_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Verify ATS Table error: invalid length: buf index = %d, cmd num = %d, pkt len = %d",
                              (int)EntryIndex, EntryPtr->Header.CmdNumber, (int)CommandBytes);
        }
        else if ((EntryIndex + SC_ATS_HDR_NOPKT_WORDS + CommandWords) > BufferWords)
        {
            /* Error -- packet must fit within buffer */
            Result = SC_ERROR;

            CFE_EVS_SendEvent(SC_VERIFY_ATS_BUF_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Verify ATS Table error: buffer overflow: buf index = %d, cmd num = %d, pkt len = %d",
                              (int)EntryIndex, EntryPtr->Header.CmdNumber, (int)CommandBytes);
        }
        else if (SC_OperData.AtsDupTestArray[SC_ATS_CMD_NUM_TO_INDEX(EntryPtr->Header.CmdNumber)] != SC_DUP_TEST_UNUSED)
        {
            /* Entry with duplicate command number is invalid */
            Result = SC_ERROR;

            CFE_EVS_SendEvent(SC_VERIFY_ATS_DUP_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Verify ATS Table error: dup cmd number: buf index = %d, cmd num = %d, dup index = %d",
                              (int)EntryIndex, EntryPtr->Header.CmdNumber,
                              (int)SC_OperData.AtsDupTestArray[SC_ATS_CMD_NUM_TO_INDEX(EntryPtr->Header.CmdNumber)]);
        }
        else
        {
            /* Compute length (in words) for this ATS table entry */
            Result = SC_ATS_HDR_NOPKT_WORDS + CommandWords;

            /* Mark this ATS command ID as in use at this table index */
            SC_OperData.AtsDupTestArray[SC_ATS_CMD_NUM_TO_INDEX(EntryPtr->Header.CmdNumber)] = EntryIndex;
        }
    }

    return (Result);

} /* End of SC_VerifyAtsEntry */

/************************/
/*  End of File Comment */
/************************/
