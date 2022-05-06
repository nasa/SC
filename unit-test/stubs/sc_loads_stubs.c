/************************************************************************
 * NASA Docket No. GSC-18,924-1, and identified as “Core Flight
 * System (cFS) Stored Command Application version 3.1.0”
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

/*
 * Includes
 */

#include "sc_loads.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

bool SC_ParseRts(uint32 Buffer[])
{
    UT_Stub_RegisterContext(UT_KEY(SC_ParseRts), Buffer);
    return UT_DEFAULT_IMPL(SC_ParseRts);
}

void SC_BuildTimeIndexTable(uint16 AtsIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_BuildTimeIndexTable), AtsIndex);
    UT_DEFAULT_IMPL(SC_BuildTimeIndexTable);
}

void SC_Insert(uint16 AtsIndex, uint32 NewCmdIndex, uint32 ListLength)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_Insert), AtsIndex);
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_Insert), NewCmdIndex);
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_Insert), ListLength);
    UT_DEFAULT_IMPL(SC_Insert);
}

void SC_InitAtsTables(uint16 AtsIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_InitAtsTables), AtsIndex);
    UT_DEFAULT_IMPL(SC_InitAtsTables);
}

int32 SC_VerifyAtsTable(uint32 *Buffer, int32 BufferWords)
{
    UT_Stub_RegisterContext(UT_KEY(SC_VerifyAtsTable), Buffer);
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_VerifyAtsTable), BufferWords);
    return UT_DEFAULT_IMPL(SC_VerifyAtsTable);
}

int32 SC_VerifyAtsEntry(uint32 *Buffer, int32 EntryIndex, int32 BufferWords)
{
    UT_Stub_RegisterContext(UT_KEY(SC_VerifyAtsEntry), Buffer);
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_VerifyAtsEntry), EntryIndex);
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_VerifyAtsEntry), BufferWords);
    return UT_DEFAULT_IMPL(SC_VerifyAtsEntry);
}

void SC_LoadAts(uint16 AtsIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_LoadAts), AtsIndex);
    UT_DEFAULT_IMPL(SC_LoadAts);
}

int32 SC_ValidateAts(void *TableData)
{
    UT_Stub_RegisterContext(UT_KEY(SC_ValidateAts), TableData);
    return UT_DEFAULT_IMPL(SC_ValidateAts);
}

int32 SC_ValidateAppend(void *TableData)
{
    UT_Stub_RegisterContext(UT_KEY(SC_ValidateAppend), TableData);
    return UT_DEFAULT_IMPL(SC_ValidateAppend);
}

void SC_UpdateAppend(void)
{
    UT_DEFAULT_IMPL(SC_UpdateAppend);
}

void SC_ProcessAppend(uint16 AtsIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_ProcessAppend), AtsIndex);
    UT_DEFAULT_IMPL(SC_ProcessAppend);
}

void SC_LoadRts(uint16 RtsIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_LoadRts), RtsIndex);
    UT_DEFAULT_IMPL(SC_LoadRts);
}

int32 SC_ValidateRts(void *TableData)
{
    UT_Stub_RegisterContext(UT_KEY(SC_ValidateRts), TableData);
    return UT_DEFAULT_IMPL(SC_ValidateRts);
}
