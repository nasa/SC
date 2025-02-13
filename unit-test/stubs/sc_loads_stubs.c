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
 *
 * Auto-Generated stub implementations for functions defined in sc_loads header
 */

#include "sc_loads.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for SC_BuildTimeIndexTable()
 * ----------------------------------------------------
 */
void SC_BuildTimeIndexTable(SC_AtsIndex_t AtsIndex)
{
    UT_GenStub_AddParam(SC_BuildTimeIndexTable, SC_AtsIndex_t, AtsIndex);

    UT_GenStub_Execute(SC_BuildTimeIndexTable, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_InitAtsTables()
 * ----------------------------------------------------
 */
void SC_InitAtsTables(SC_AtsIndex_t AtsIndex)
{
    UT_GenStub_AddParam(SC_InitAtsTables, SC_AtsIndex_t, AtsIndex);

    UT_GenStub_Execute(SC_InitAtsTables, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_Insert()
 * ----------------------------------------------------
 */
void SC_Insert(SC_AtsIndex_t AtsIndex, SC_CommandIndex_t NewCmdIndex, uint32 ListLength)
{
    UT_GenStub_AddParam(SC_Insert, SC_AtsIndex_t, AtsIndex);
    UT_GenStub_AddParam(SC_Insert, SC_CommandIndex_t, NewCmdIndex);
    UT_GenStub_AddParam(SC_Insert, uint32, ListLength);

    UT_GenStub_Execute(SC_Insert, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_LoadAts()
 * ----------------------------------------------------
 */
void SC_LoadAts(SC_AtsIndex_t AtsIndex)
{
    UT_GenStub_AddParam(SC_LoadAts, SC_AtsIndex_t, AtsIndex);

    UT_GenStub_Execute(SC_LoadAts, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_LoadRts()
 * ----------------------------------------------------
 */
void SC_LoadRts(SC_RtsIndex_t RtsIndex)
{
    UT_GenStub_AddParam(SC_LoadRts, SC_RtsIndex_t, RtsIndex);

    UT_GenStub_Execute(SC_LoadRts, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ParseRts()
 * ----------------------------------------------------
 */
bool SC_ParseRts(uint32 Buffer32[])
{
    UT_GenStub_SetupReturnBuffer(SC_ParseRts, bool);

    UT_GenStub_Execute(SC_ParseRts, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_ParseRts, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ProcessAppend()
 * ----------------------------------------------------
 */
void SC_ProcessAppend(SC_AtsIndex_t AtsIndex)
{
    UT_GenStub_AddParam(SC_ProcessAppend, SC_AtsIndex_t, AtsIndex);

    UT_GenStub_Execute(SC_ProcessAppend, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_UpdateAppend()
 * ----------------------------------------------------
 */
void SC_UpdateAppend(void)
{
    UT_GenStub_Execute(SC_UpdateAppend, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ValidateAppend()
 * ----------------------------------------------------
 */
int32 SC_ValidateAppend(void *TableData)
{
    UT_GenStub_SetupReturnBuffer(SC_ValidateAppend, int32);

    UT_GenStub_AddParam(SC_ValidateAppend, void *, TableData);

    UT_GenStub_Execute(SC_ValidateAppend, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_ValidateAppend, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ValidateAts()
 * ----------------------------------------------------
 */
int32 SC_ValidateAts(void *TableData)
{
    UT_GenStub_SetupReturnBuffer(SC_ValidateAts, int32);

    UT_GenStub_AddParam(SC_ValidateAts, void *, TableData);

    UT_GenStub_Execute(SC_ValidateAts, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_ValidateAts, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ValidateRts()
 * ----------------------------------------------------
 */
int32 SC_ValidateRts(void *TableData)
{
    UT_GenStub_SetupReturnBuffer(SC_ValidateRts, int32);

    UT_GenStub_AddParam(SC_ValidateRts, void *, TableData);

    UT_GenStub_Execute(SC_ValidateRts, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_ValidateRts, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_VerifyAtsEntry()
 * ----------------------------------------------------
 */
int32 SC_VerifyAtsEntry(uint32 *Buffer32, int32 EntryIndex, int32 BufferWords)
{
    UT_GenStub_SetupReturnBuffer(SC_VerifyAtsEntry, int32);

    UT_GenStub_AddParam(SC_VerifyAtsEntry, uint32 *, Buffer32);
    UT_GenStub_AddParam(SC_VerifyAtsEntry, int32, EntryIndex);
    UT_GenStub_AddParam(SC_VerifyAtsEntry, int32, BufferWords);

    UT_GenStub_Execute(SC_VerifyAtsEntry, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_VerifyAtsEntry, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_VerifyAtsTable()
 * ----------------------------------------------------
 */
int32 SC_VerifyAtsTable(uint32 *Buffer32, int32 BufferWords)
{
    UT_GenStub_SetupReturnBuffer(SC_VerifyAtsTable, int32);

    UT_GenStub_AddParam(SC_VerifyAtsTable, uint32 *, Buffer32);
    UT_GenStub_AddParam(SC_VerifyAtsTable, int32, BufferWords);

    UT_GenStub_Execute(SC_VerifyAtsTable, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_VerifyAtsTable, int32);
}
