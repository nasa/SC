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

/*
 * Includes
 */

#include "sc_utils.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

void SC_GetCurrentTime(void)
{
    UT_DEFAULT_IMPL(SC_GetCurrentTime);
}

SC_AbsTimeTag_t SC_GetAtsEntryTime(SC_AtsEntryHeader_t *Entry)
{
    UT_Stub_RegisterContext(UT_KEY(SC_GetAtsEntryTime), Entry);
    return UT_DEFAULT_IMPL(SC_GetAtsEntryTime);
}

SC_AbsTimeTag_t SC_ComputeAbsTime(SC_RelTimeTag_t RelTime)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_ComputeAbsTime), RelTime);
    return UT_DEFAULT_IMPL(SC_ComputeAbsTime);
}

bool SC_CompareAbsTime(SC_AbsTimeTag_t AbsTime1, SC_AbsTimeTag_t AbsTime2)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_CompareAbsTime), AbsTime1);
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_CompareAbsTime), AbsTime2);
    return UT_DEFAULT_IMPL(SC_CompareAbsTime);
}

bool SC_VerifyCmdLength(const CFE_MSG_Message_t *Msg, size_t ExpectedLength)
{
    UT_Stub_RegisterContext(UT_KEY(SC_VerifyCmdLength), Msg);
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_VerifyCmdLength), ExpectedLength);
    return UT_DEFAULT_IMPL(SC_VerifyCmdLength);
}

uint16 SC_ToggleAtsIndex(void)
{
    return UT_DEFAULT_IMPL(SC_ToggleAtsIndex);
}
