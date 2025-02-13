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
 * Auto-Generated stub implementations for functions defined in sc_utils header
 */

#include "sc_utils.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for SC_CompareAbsTime()
 * ----------------------------------------------------
 */
bool SC_CompareAbsTime(SC_AbsTimeTag_t AbsTime1, SC_AbsTimeTag_t AbsTime2)
{
    UT_GenStub_SetupReturnBuffer(SC_CompareAbsTime, bool);

    UT_GenStub_AddParam(SC_CompareAbsTime, SC_AbsTimeTag_t, AbsTime1);
    UT_GenStub_AddParam(SC_CompareAbsTime, SC_AbsTimeTag_t, AbsTime2);

    UT_GenStub_Execute(SC_CompareAbsTime, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_CompareAbsTime, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ComputeAbsTime()
 * ----------------------------------------------------
 */
SC_AbsTimeTag_t SC_ComputeAbsTime(uint32 RelTime)
{
    UT_GenStub_SetupReturnBuffer(SC_ComputeAbsTime, SC_AbsTimeTag_t);

    UT_GenStub_AddParam(SC_ComputeAbsTime, uint32, RelTime);

    UT_GenStub_Execute(SC_ComputeAbsTime, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_ComputeAbsTime, SC_AbsTimeTag_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ComputeAbsWakeup()
 * ----------------------------------------------------
 */
uint32 SC_ComputeAbsWakeup(uint32 RelWakeup)
{
    UT_GenStub_SetupReturnBuffer(SC_ComputeAbsWakeup, uint32);

    UT_GenStub_AddParam(SC_ComputeAbsWakeup, uint32, RelWakeup);

    UT_GenStub_Execute(SC_ComputeAbsWakeup, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_ComputeAbsWakeup, uint32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_GetAtsEntryTime()
 * ----------------------------------------------------
 */
SC_AbsTimeTag_t SC_GetAtsEntryTime(SC_AtsEntryHeader_t *Entry)
{
    UT_GenStub_SetupReturnBuffer(SC_GetAtsEntryTime, SC_AbsTimeTag_t);

    UT_GenStub_AddParam(SC_GetAtsEntryTime, SC_AtsEntryHeader_t *, Entry);

    UT_GenStub_Execute(SC_GetAtsEntryTime, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_GetAtsEntryTime, SC_AbsTimeTag_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_GetCurrentTime()
 * ----------------------------------------------------
 */
void SC_GetCurrentTime(void)
{
    UT_GenStub_Execute(SC_GetCurrentTime, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_LookupTimeAccessor()
 * ----------------------------------------------------
 */
SC_TimeAccessor_t SC_LookupTimeAccessor(SC_TimeRef_Enum_t TimeRef)
{
    UT_GenStub_SetupReturnBuffer(SC_LookupTimeAccessor, SC_TimeAccessor_t);

    UT_GenStub_AddParam(SC_LookupTimeAccessor, SC_TimeRef_Enum_t, TimeRef);

    UT_GenStub_Execute(SC_LookupTimeAccessor, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_LookupTimeAccessor, SC_TimeAccessor_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ToggleAtsIndex()
 * ----------------------------------------------------
 */
SC_AtsIndex_t SC_ToggleAtsIndex(void)
{
    UT_GenStub_SetupReturnBuffer(SC_ToggleAtsIndex, SC_AtsIndex_t);

    UT_GenStub_Execute(SC_ToggleAtsIndex, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_ToggleAtsIndex, SC_AtsIndex_t);
}
