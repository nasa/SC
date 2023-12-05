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
#ifndef SC_LOADS_H
#define SC_LOADS_H

#include "cfe.h"
#include "sc_index_types.h"

/**
 * \brief Parses an RTS to see if it is valid
 *
 *  \par Description
 *        This routine is called to validate an RTS buffer. It parses through
 *        the RTS to make sure all of the commands look in reasonable shape.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]    Buffer32 Pointer to the area to validate
 *
 *  \return Boolean RTS valid status
 *  \retval true  RTS was valid
 *  \retval false RTS was NOT valid
 */
bool SC_ParseRts(uint32 Buffer32[]);

/**
 * \brief Buids the Time index buffer for the ATS
 *
 *  \par Description
 *        This routine builds the ATS Time Index Table after an ATS buffer
 *        has been loaded and the ATS Command Index Table has been built.
 *        This routine will take the commands that are pointed to by the
 *        pointers in the command index table and sort the commands by
 *        time order.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]    AtsIndex        ATS array index
 */
void SC_BuildTimeIndexTable(SC_AtsIndex_t AtsIndex);

/**
 * \brief Inserts an item in a sorted list
 *
 *  \par Description
 *        This function will insert a new element into the list of
 *        ATS commands sorted by execution time.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]    AtsIndex        ATS array index selection
 *  \param [in]    NewCmdIndex     ATS command index for new list element
 *  \param [in]    ListLength      Number of elements currently in list
 */
void SC_Insert(SC_AtsIndex_t AtsIndex, SC_CommandIndex_t NewCmdIndex, uint32 ListLength);

/**
 * \brief Initializes ATS tables before a load starts
 *
 *  \par Description
 *        This function simply clears out the ats tables in preparation
 *        for a load.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]    AtsIndex        ATS array index
 */
void SC_InitAtsTables(SC_AtsIndex_t AtsIndex);

/**
 * \brief Validation function for ATS or Append ATS table data
 *
 *  \par Description
 *        This routine is called to validate the contents of an ATS
 *        or Apppend ATS table.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \return Table validation result
 *  \retval #CFE_SUCCESS Table validation success
 *  \retval #SC_ERROR    Table not valid
 */
int32 SC_VerifyAtsTable(uint32 *Buffer32, int32 BufferWords);

/**
 * \brief Validation function for a single ATS or Append ATS table entry
 *
 *  \par Description
 *        This routine is called to validate the contents of a
 *        single ATS or Append ATS table entry.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \return Positive integer equal to table entry length (in words) or
 *  \retval #CFE_SUCCESS Empty entry
 *  \retval #SC_ERROR    Entry is invalid
 */
int32 SC_VerifyAtsEntry(uint32 *Buffer32, int32 EntryIndex, int32 BufferWords);

/**
 * \brief Loads an ATS into the data structures in SC
 *
 *  \par Description
 *        This routine is called when the SC app gets a new ATS table.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]    AtsIndex            ATS table array index (base zero)
 */
void SC_LoadAts(SC_AtsIndex_t AtsIndex);

/**
 * \brief Validation function for an ATS
 *
 *  \par Description
 *        This routine is called from the cFE Table Services and passed
 *        as a parameter in the cFE Table Registration call.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \return Table validation result
 *  \retval #CFE_SUCCESS Table validation success
 *  \retval #SC_ERROR    Table not valid
 */
int32 SC_ValidateAts(void *TableData);

/**
 * \brief Validation function for the Append ATS Table
 *
 *  \par Description
 *        This routine is called from the cFE Table Services as part of
 *        the table load/validate/commit process.  The function pointer
 *        is passed as a parameter in the cFE Table Registration call.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \return Table validation result
 *  \retval #CFE_SUCCESS Table validation success
 *  \retval #SC_ERROR    Table not valid
 */
int32 SC_ValidateAppend(void *TableData);

/**
 * \brief Updates Append ATS Info table per new contents of Append ATS table
 *
 *  \par Description
 *        This routine is called when the SC app receives notification
 *        from cFE Table Services that the Append ATS table contents
 *        have been updated.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 */
void SC_UpdateAppend(void);

/**
 * \brief Appends contents of Append ATS table to indicated ATS table
 *
 *  \par Description
 *        This routine is called from the Append ATS command handler to
 *        append the contents of the Append ATS table to the end of the
 *        indicated ATS table.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]    AtsIndex            ATS table array index (base zero)
 */
void SC_ProcessAppend(SC_AtsIndex_t AtsIndex);

/**
 * \brief Loads an RTS into the data structures in SC
 *
 *  \par Description
 *        This routine is called when the SC app gets a new RTS table.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]    RtsIndex            RTS table array index (base zero)
 */
void SC_LoadRts(SC_RtsIndex_t RtsIndex);

/**
 * \brief Validation function for an RTS
 *
 *  \par Description
 *        This routine is called from the cFE Table Services and passed
 *        as a parameter in the cFE Table Registration call.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \return Table validation result
 *  \retval #CFE_SUCCESS Table validation success
 *  \retval #SC_ERROR    Table not valid
 */
int32 SC_ValidateRts(void *TableData);

#endif
