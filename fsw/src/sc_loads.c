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
void SC_LoadAts(SC_AtsIndex_t AtsIndex)
{
    SC_CommandNum_t               AtsCmdNum;         /* current ats entry command number */
    SC_EntryOffset_t              AtsEntryIndex;     /* index into the load for current ats entry */
    SC_EntryOffset_t              PendingEntryIndex; /* index into the load for current ats entry */
    SC_AtsEntry_t *               EntryPtr;          /* a pointer to an ats entry */
    CFE_MSG_Size_t                MessageSize     = 0;
    int32                         Result          = CFE_SUCCESS;
    bool                          StillProcessing = true;
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec; /* ATS entry location in table */

    /* validate ATS array index */
    if (!SC_AtsIndexIsValid(AtsIndex))
    {
        CFE_EVS_SendEvent(SC_LOADATS_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR, "ATS load error: invalid ATS index %d",
                          SC_IDX_AS_UINT(AtsIndex));
        return;
    }

    /*
     ** Initialize all structures
     */
    SC_InitAtsTables(AtsIndex);

    /* initialize pointers and counters */
    AtsInfoPtr    = SC_GetAtsInfoObject(AtsIndex);
    AtsEntryIndex = SC_ENTRY_OFFSET_FIRST;

    while (StillProcessing)
    {
        /*
         ** Make sure that the pointer as well as the primary packet
         ** header fit in the buffer, so a G.P fault is not caused.
         */
        if (SC_IDX_WITHIN_LIMIT(AtsEntryIndex, SC_ATS_BUFF_SIZE32))
        {
            /* get a pointer to the ats command in the table */
            EntryPtr = SC_GetAtsEntryAtOffset(AtsIndex, AtsEntryIndex);

            /* get the next command number from the buffer */
            AtsCmdNum = EntryPtr->Header.CmdNumber;

            if (SC_IDNUM_IS_NULL(AtsCmdNum))
            {
                /* end of the load reached */
                Result = CFE_SUCCESS;
                break;
            }

            if (!SC_AtsCommandNumIsValid(AtsCmdNum))
            {
                /* the cmd number is invalid */
                Result = SC_ERROR;
                break;
            }

            /* make sure the CmdPtr can fit in a whole Ats Cmd Header at the very least */
            if (!SC_IDX_WITHIN_LIMIT(AtsEntryIndex,
                                     1 + (SC_ATS_BUFF_SIZE32 - SC_ATS_HDR_WORDS))) /* jphfix - revisit? */
            {
                /* even the smallest command will not fit in the buffer */
                Result = SC_ERROR;
                break;
            }

            StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_CommandNumToIndex(AtsCmdNum));
            CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_CommandNumToIndex(AtsCmdNum));

            if (StatusEntryPtr->Status == SC_Status_EMPTY)
            {
                /* get message size */
                CFE_MSG_GetSize(CFE_MSG_PTR(EntryPtr->Msg), &MessageSize);

                /* if the length of the command is valid */
                if (MessageSize >= SC_PACKET_MIN_SIZE && MessageSize <= SC_PACKET_MAX_SIZE)
                {
                    /* get the length of the entry in WORDS (plus 1 to round byte len up to word len) */
                    PendingEntryIndex = SC_EntryOffsetAdvance(AtsEntryIndex, MessageSize + SC_ATS_HEADER_SIZE);

                    /* if the command does not run off of the end of the buffer */
                    if (SC_IDX_WITHIN_LIMIT(PendingEntryIndex, 1 + SC_ATS_BUFF_SIZE32))
                    {
                        /* set the command pointer in the command index table */
                        /* CmdNum starts at one....                          */

                        CmdOffsetRec->Offset = AtsEntryIndex;

                        /* set the command status to loaded in the command status table */
                        StatusEntryPtr->Status = SC_Status_LOADED;

                        /* increment the number of commands loaded */
                        AtsInfoPtr->NumberOfCommands++;

                        /* increment the ats_entry index to the next ats entry */
                        AtsEntryIndex = PendingEntryIndex;
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
     **   if the load was a success, need to build the tables
     */
    /* if the load finished without errors and there was at least one command */
    if ((Result == CFE_SUCCESS) && (AtsInfoPtr->NumberOfCommands > 0))
    {
        /* record the size of the load in the ATS info table */
        AtsInfoPtr->AtsSize = SC_IDX_AS_UINT(AtsEntryIndex); /* size in 32-bit WORDS */

        /* build the time index table */
        SC_BuildTimeIndexTable(AtsIndex);
    }
    else
    { /* there was an error */
        SC_InitAtsTables(AtsIndex);
    } /* end if */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Builds the time table for the ATS buffer                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_BuildTimeIndexTable(SC_AtsIndex_t AtsIndex)
{
    int32                 i;
    int32                 ListLength;
    SC_CommandIndex_t     CmdIdx;
    SC_AtsCmdNumRecord_t *AtsCmdNumRec;

    /* validate ATS array index */
    if (!SC_AtsIndexIsValid(AtsIndex))
    {
        CFE_EVS_SendEvent(SC_BUILD_TIME_IDXTBL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Build time index table error: invalid ATS index %u", SC_IDX_AS_UINT(AtsIndex));
        return;
    }

    /* initialize sorted list length */
    ListLength = 0;

    /* initialize sorted list contents */
    for (i = 0; i < SC_MAX_ATS_CMDS; i++)
    {
        AtsCmdNumRec         = SC_GetAtsCommandNumAtSeq(AtsIndex, SC_SEQUENCE_IDX_C(i));
        AtsCmdNumRec->CmdNum = SC_INVALID_CMD_NUMBER;

        /* add in-use command entries to time sorted list */
        CmdIdx = SC_COMMAND_IDX_C(i);
        if (!SC_IDX_EQUAL(SC_GetAtsEntryOffsetForCmd(AtsIndex, CmdIdx)->Offset, SC_ENTRY_OFFSET_INVALID))
        {
            SC_Insert(AtsIndex, CmdIdx, ListLength);
            ListLength++;
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Inserts and element into a sorted list                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_Insert(SC_AtsIndex_t AtsIndex, SC_CommandIndex_t NewCmdIndex, uint32 ListLength)
{
    SC_AtsEntryHeader_t *         EntryHeader;    /* ATS table entry pointer */
    SC_AbsTimeTag_t               NewCmdTime = 0; /* new command execution time */
    SC_AbsTimeTag_t               ListCmdTime;    /* list entry execution time */
    SC_CommandIndex_t             CmdIndex;       /* ATS command index (cmd num - 1) */
    SC_SeqIndex_t                 TimeBufIndex;
    SC_SeqIndex_t                 NextIndex;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec; /* ATS entry location in table */
    SC_AtsCmdNumRecord_t *        AtsCmdNumRec;

    /* validate ATS array index */
    if (!SC_AtsIndexIsValid(AtsIndex))
    {
        CFE_EVS_SendEvent(SC_INSERTATS_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ATS insert error: invalid ATS index %u", SC_IDX_AS_UINT(AtsIndex));
        return;
    }

    /* get execution time for new list entry */
    if (ListLength > 0)
    {
        /* first get the entry index in the selected ATS table for the new command */
        CmdOffsetRec = SC_GetAtsEntryOffsetForCmd(AtsIndex, NewCmdIndex);
        /* then get a pointer to the ATS entry */
        EntryHeader = &SC_GetAtsEntryAtOffset(AtsIndex, CmdOffsetRec->Offset)->Header;
        /* then get the execution time from the ATS entry for the new command */
        NewCmdTime = SC_GetAtsEntryTime(EntryHeader);
    }

    /* start at last element in the sorted by time list */
    TimeBufIndex = SC_SEQUENCE_IDX_C(ListLength - 1);

    while (SC_IDX_WITHIN_LIMIT(TimeBufIndex, ListLength))
    {
        /* first get the cmd index for this list entry */
        CmdIndex = SC_CommandNumToIndex(SC_GetAtsCommandNumAtSeq(AtsIndex, TimeBufIndex)->CmdNum);
        /* then get the entry index from the ATS table */
        CmdOffsetRec = SC_GetAtsEntryOffsetForCmd(AtsIndex, CmdIndex);
        /* then get a pointer to the ATS entry data */
        EntryHeader = &SC_GetAtsEntryAtOffset(AtsIndex, CmdOffsetRec->Offset)->Header;
        /* then get cmd execution time from the ATS entry */
        ListCmdTime = SC_GetAtsEntryTime(EntryHeader);

        /* compare time for this list entry to time for new cmd */
        if (SC_CompareAbsTime(ListCmdTime, NewCmdTime))
        {
            /* new cmd will execute before this list entry */

            /* move this list entry to make room for new cmd */
            NextIndex = TimeBufIndex;
            SC_IDX_INCREMENT(NextIndex);

            SC_GetAtsCommandNumAtSeq(AtsIndex, NextIndex)->CmdNum =
                SC_GetAtsCommandNumAtSeq(AtsIndex, TimeBufIndex)->CmdNum;

            /* back up to previous list entry (ok if -1) */
            SC_IDX_DECREMENT(TimeBufIndex);
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
    SC_IDX_INCREMENT(TimeBufIndex);

    AtsCmdNumRec         = SC_GetAtsCommandNumAtSeq(AtsIndex, TimeBufIndex);
    AtsCmdNumRec->CmdNum = SC_CommandIndexToNum(NewCmdIndex);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Clears out Ats Tables before a load                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_InitAtsTables(SC_AtsIndex_t AtsIndex)
{
    int32                         i;
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;
    SC_AtsCmdNumRecord_t *        AtsCmdNumRec;

    /* validate ATS array index */
    if (!SC_AtsIndexIsValid(AtsIndex))
    {
        CFE_EVS_SendEvent(SC_INIT_ATSTBL_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ATS table init error: invalid ATS index %u", SC_IDX_AS_UINT(AtsIndex));
        return;
    }

    /* loop through and set the ATS tables to zero */
    for (i = 0; i < SC_MAX_ATS_CMDS; i++)
    {
        StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, SC_COMMAND_IDX_C(i));
        CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, SC_COMMAND_IDX_C(i));
        AtsCmdNumRec   = SC_GetAtsCommandNumAtSeq(AtsIndex, SC_SEQUENCE_IDX_C(i));

        CmdOffsetRec->Offset   = SC_ENTRY_OFFSET_INVALID;
        StatusEntryPtr->Status = SC_Status_EMPTY;
        AtsCmdNumRec->CmdNum   = SC_INVALID_CMD_NUMBER;
    }

    /* initialize the pointers and counters   */
    AtsInfoPtr                   = SC_GetAtsInfoObject(AtsIndex);
    AtsInfoPtr->AtsSize          = 0;
    AtsInfoPtr->NumberOfCommands = 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initializes the info table entry for an RTS                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_LoadRts(SC_RtsIndex_t RtsIndex)
{
    SC_RtsInfoEntry_t *RtsInfoPtr;

    /* validate RTS array index */
    if (SC_RtsIndexIsValid(RtsIndex))
    {
        /* Clear out the RTS info table */
        RtsInfoPtr = SC_GetRtsInfoObject(RtsIndex);

        RtsInfoPtr->RtsStatus            = SC_Status_LOADED;
        RtsInfoPtr->UseCtr               = 0;
        RtsInfoPtr->CmdCtr               = 0;
        RtsInfoPtr->CmdErrCtr            = 0;
        RtsInfoPtr->NextCommandTgtWakeup = 0;
        RtsInfoPtr->NextCommandPtr       = SC_ENTRY_OFFSET_FIRST;

        /* Make sure the RTS is disabled */
        RtsInfoPtr->DisabledFlag = true;
    }
    else
    {
        CFE_EVS_SendEvent(SC_LOADRTS_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RTS table init error: invalid RTS index %u", SC_IDX_AS_UINT(RtsIndex));
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

    return Result;
}

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
    CFE_MSG_Size_t CmdSize = 0;
    uint16         IndexDelta;
    CFE_SB_MsgId_t MessageID = CFE_SB_INVALID_MSG_ID;

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

            CFE_MSG_GetSize(CFE_MSG_PTR(EntryPtr->Msg), &CmdSize);

            /* Add header size, round up to boundary, convert to index delta  */
            IndexDelta = (CmdSize + SC_RTS_HEADER_SIZE + SC_ROUND_UP_BYTES) / sizeof(Buffer32[0]);

            CFE_MSG_GetMsgId(CFE_MSG_PTR(EntryPtr->Msg), &MessageID);

            if (!CFE_SB_IsValidMsgId(MessageID))
            {
                if (EntryPtr->Header.WakeupCount == 0)
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
    return !Error;
}

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

    return Result;
}

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

    return Result;
}

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

            if (!SC_AtsCommandNumIsValid(EntryPtr->Header.CmdNumber))
            {
                /* End of valid command numbers */
                StillProcessing = false;
            }
            else
            {
                CFE_MSG_GetSize(CFE_MSG_PTR(EntryPtr->Msg), &CommandBytes);
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
    SC_OperData.HkPacket.Payload.AppendLoadCount++;
    SC_OperData.HkPacket.Payload.AppendEntryCount = EntryCount;
    SC_OperData.HkPacket.Payload.AppendByteCount  = EntryIndex * SC_BYTES_IN_ATS_APPEND_ENTRY;
    SC_AppData.AppendWordCount                    = EntryIndex;

    CFE_EVS_SendEvent(SC_UPDATE_APPEND_EID, CFE_EVS_EventType_INFORMATION,
                      "Update Append ATS Table: load count = %d, command count = %d, byte count = %d",
                      SC_OperData.HkPacket.Payload.AppendLoadCount, (int)EntryCount,
                      (int)EntryIndex * SC_BYTES_IN_WORD);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Append contents of Append ATS table to indicated ATS table      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_ProcessAppend(SC_AtsIndex_t AtsIndex)
{
    SC_AtsEntry_t *               EntryPtr;
    CFE_MSG_Size_t                CommandBytes = 0;
    SC_EntryOffset_t              EntryIndex;
    int32                         i;
    SC_CommandIndex_t             CmdIndex;
    SC_AtsInfoTable_t *           AtsInfoPtr;
    SC_AtsCmdStatusEntry_t *      StatusEntryPtr;
    SC_AtsCmdEntryOffsetRecord_t *CmdOffsetRec;

    /* validate ATS array index */
    if (!SC_AtsIndexIsValid(AtsIndex))
    {
        CFE_EVS_SendEvent(SC_PROCESS_APPEND_INV_INDEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ATS process append error: invalid ATS index %u", SC_IDX_AS_UINT(AtsIndex));
        return;
    }

    /* save index of free area at end of ATS table data */
    AtsInfoPtr = SC_GetAtsInfoObject(AtsIndex);
    EntryIndex = SC_ENTRY_OFFSET_C(AtsInfoPtr->AtsSize);

    /* copy Append table data to end of ATS table data */
    memcpy(SC_GetAtsEntryAtOffset(AtsIndex, EntryIndex), SC_OperData.AppendTblAddr,
           SC_AppData.AppendWordCount * SC_BYTES_IN_WORD);

    /* update size of ATS table data */
    AtsInfoPtr->AtsSize += SC_AppData.AppendWordCount;

    /* add appended entries to ats process tables */
    for (i = 0; i < SC_OperData.HkPacket.Payload.AppendEntryCount; i++)
    {
        /* get pointer to next appended entry */
        EntryPtr = SC_GetAtsEntryAtOffset(AtsIndex, EntryIndex);

        /* convert base one cmd number to base zero index */
        CmdIndex = SC_CommandNumToIndex(EntryPtr->Header.CmdNumber);

        StatusEntryPtr = SC_GetAtsStatusEntryForCommand(AtsIndex, CmdIndex);
        CmdOffsetRec   = SC_GetAtsEntryOffsetForCmd(AtsIndex, CmdIndex);

        /* count only new commands, not replaced commands */
        if (StatusEntryPtr->Status == SC_Status_EMPTY)
        {
            AtsInfoPtr->NumberOfCommands++;
        }

        /* update array of pointers to ats entries */
        CmdOffsetRec->Offset   = EntryIndex;
        StatusEntryPtr->Status = SC_Status_LOADED;

        /* update entry index to point to the next entry */
        CFE_MSG_GetSize(CFE_MSG_PTR(EntryPtr->Msg), &CommandBytes);
        EntryIndex = SC_EntryOffsetAdvance(EntryIndex, CommandBytes + SC_ATS_HEADER_SIZE);
    }

    /* rebuild time sorted list of commands */
    SC_BuildTimeIndexTable(AtsIndex);

    /* did we just append to an ats that was executing? */
    if ((SC_OperData.AtsCtrlBlckAddr->AtpState == SC_Status_EXECUTING) &&
        (SC_IDNUM_EQUAL(SC_OperData.AtsCtrlBlckAddr->CurrAtsNum, SC_AtsIndexToNum(AtsIndex))))
    {
        /*
        ** re-start the ats -- this will go thru the process of skipping
        **  past due entries (all of the old entries that had already
        **  been executed and all of the new entries with an old time)
        */
        if (SC_BeginAts(AtsIndex, 0))
        {
            SC_OperData.AtsCtrlBlckAddr->AtpState = SC_Status_EXECUTING;
        }
    }

    /* notify cFE that we have modified the ats table */
    CFE_TBL_Modified(SC_OperData.AtsTblHandle[SC_IDX_AS_UINT(AtsIndex)]);
}

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

    return Result;
}

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
    else if (SC_IDNUM_IS_NULL(EntryPtr->Header.CmdNumber))
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
    else if (!SC_AtsCommandNumIsValid(EntryPtr->Header.CmdNumber))
    {
        /* Error -- invalid command number */
        Result = SC_ERROR;

        CFE_EVS_SendEvent(SC_VERIFY_ATS_NUM_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Verify ATS Table error: invalid command number: buf index = %d, cmd num = %u",
                          (int)EntryIndex, SC_IDNUM_AS_UINT(EntryPtr->Header.CmdNumber));
    }
    else if ((EntryIndex + SC_ATS_HDR_WORDS) > BufferWords)
    {
        /* Error -- not enough room for smallest possible ATS entry */
        Result = SC_ERROR;

        CFE_EVS_SendEvent(SC_VERIFY_ATS_END_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Verify ATS Table error: buffer full: buf index = %d, cmd num = %u, buf words = %d",
                          (int)EntryIndex, SC_IDNUM_AS_UINT(EntryPtr->Header.CmdNumber), (int)BufferWords);
    }
    else
    {
        /* Start with the byte length of the command packet */
        CFE_MSG_GetSize(CFE_MSG_PTR(EntryPtr->Msg), &CommandBytes);

        /* Convert packet byte length to word length (round up odd bytes) */
        CommandWords = (CommandBytes + SC_ROUND_UP_BYTES) / SC_BYTES_IN_WORD;

        if ((CommandBytes < SC_PACKET_MIN_SIZE) || (CommandBytes > SC_PACKET_MAX_SIZE))
        {
            /* Error -- invalid command packet byte length */
            Result = SC_ERROR;

            CFE_EVS_SendEvent(SC_VERIFY_ATS_PKT_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Verify ATS Table error: invalid length: buf index = %d, cmd num = %u, pkt len = %d",
                              (int)EntryIndex, SC_IDNUM_AS_UINT(EntryPtr->Header.CmdNumber), (int)CommandBytes);
        }
        else if ((EntryIndex + SC_ATS_HDR_NOPKT_WORDS + CommandWords) > BufferWords)
        {
            /* Error -- packet must fit within buffer */
            Result = SC_ERROR;

            CFE_EVS_SendEvent(SC_VERIFY_ATS_BUF_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Verify ATS Table error: buffer overflow: buf index = %d, cmd num = %u, pkt len = %d",
                              (int)EntryIndex, SC_IDNUM_AS_UINT(EntryPtr->Header.CmdNumber), (int)CommandBytes);
        }
        else if (SC_OperData.AtsDupTestArray[SC_CommandNumToIndex(EntryPtr->Header.CmdNumber)] != SC_DUP_TEST_UNUSED)
        {
            /* Entry with duplicate command number is invalid */
            Result = SC_ERROR;

            CFE_EVS_SendEvent(SC_VERIFY_ATS_DUP_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Verify ATS Table error: dup cmd number: buf index = %d, cmd num = %u, dup index = %d",
                              (int)EntryIndex, SC_IDNUM_AS_UINT(EntryPtr->Header.CmdNumber),
                              (int)SC_OperData.AtsDupTestArray[SC_CommandNumToIndex(EntryPtr->Header.CmdNumber)]);
        }
        else
        {
            /* Compute length (in words) for this ATS table entry */
            Result = SC_ATS_HDR_NOPKT_WORDS + CommandWords;

            /* Mark this ATS command ID as in use at this table index */
            SC_OperData.AtsDupTestArray[SC_CommandNumToIndex(EntryPtr->Header.CmdNumber)] = EntryIndex;
        }
    }

    return Result;
}
