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

#include "cfe.h"
#include "sc_utils.h"
#include "sc_events.h"
#include "sc_msgids.h"
#include "sc_test_utils.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

/* sc_utils_tests globals */

static CFE_TIME_SysTime_t UT_TimeRefFunc(void)
{
    return (CFE_TIME_SysTime_t) {1234, 5678};
}

void SC_LookupTimeAccessor_Test(void)
{
    union
    {
        SC_TimeAccessor_t Obj;
        void *            Addr;
    } Accessor;

    Accessor.Addr = NULL;
    Accessor.Obj  = SC_LookupTimeAccessor(SC_TimeRef_USE_TAI);
    UtAssert_NOT_NULL(Accessor.Addr);

    Accessor.Addr = NULL;
    Accessor.Obj  = SC_LookupTimeAccessor(SC_TimeRef_MAX);
    UtAssert_NOT_NULL(Accessor.Addr);
}

void SC_GetCurrentTime_Test(void)
{
    SC_AppData.TimeRef     = (SC_TimeAccessor_t) {UT_TimeRefFunc};
    SC_AppData.CurrentTime = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(SC_GetCurrentTime());

    /* Verify results */
    UtAssert_UINT32_EQ(SC_AppData.CurrentTime, 1234);
}

void SC_GetAtsEntryTime_Test(void)
{
    SC_AtsEntryHeader_t Entry;
    Entry.TimeTag_MS = 0;
    Entry.TimeTag_LS = 10;

    /* Execute the function being tested */
    UtAssert_UINT32_EQ(SC_GetAtsEntryTime(&Entry), Entry.TimeTag_LS);
}

void SC_ComputeAbsTime_Test(void)
{
    SC_AppData.CurrentTime = 0;

    /* Execute the function being tested
     * Note the CFE_TIME_Add stub increments when status >= 0 */
    UtAssert_UINT32_EQ(SC_ComputeAbsTime(0), 1);
}

void SC_ComputeAbsWakeup_Test(void)
{
    SC_AppData.CurrentWakeupCount = 0;

    /* Execute the function being tested */
    UtAssert_UINT32_EQ(SC_ComputeAbsWakeup(1), 1);
}

void SC_CompareAbsTime_Test_True(void)
{
    SC_AbsTimeTag_t AbsTimeTag1 = {0};
    SC_AbsTimeTag_t AbsTimeTag2 = {0};

    UT_SetDeferredRetcode(UT_KEY(CFE_TIME_Compare), 1, CFE_TIME_A_GT_B);

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(SC_CompareAbsTime(AbsTimeTag1, AbsTimeTag2));
}

void SC_CompareAbsTime_Test_False(void)
{
    SC_AbsTimeTag_t AbsTimeTag1 = {0};
    SC_AbsTimeTag_t AbsTimeTag2 = {0};

    UT_SetDeferredRetcode(UT_KEY(CFE_TIME_Compare), 1, -1);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(SC_CompareAbsTime(AbsTimeTag1, AbsTimeTag2));
}

void SC_ToggleAtsIndex_Test(void)
{
    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(1);

    UtAssert_UINT32_EQ(SC_IDX_AS_UINT(SC_ToggleAtsIndex()), 1);

    SC_OperData.AtsCtrlBlckAddr->CurrAtsNum = SC_ATS_NUM_C(2);

    UtAssert_UINT32_EQ(SC_IDX_AS_UINT(SC_ToggleAtsIndex()), 0);
}

void UtTest_Setup(void)
{
    UtTest_Add(SC_LookupTimeAccessor_Test, SC_Test_Setup, SC_Test_TearDown, "SC_LookupTimeAccessor_Test");
    UtTest_Add(SC_GetCurrentTime_Test, SC_Test_Setup, SC_Test_TearDown, "SC_GetCurrentTime_Test");
    UtTest_Add(SC_GetAtsEntryTime_Test, SC_Test_Setup, SC_Test_TearDown, "SC_GetAtsEntryTime_Test");
    UtTest_Add(SC_ComputeAbsTime_Test, SC_Test_Setup, SC_Test_TearDown, "SC_ComputeAbsTime_Test");
    UtTest_Add(SC_ComputeAbsWakeup_Test, SC_Test_Setup, SC_Test_TearDown, "SC_ComputeAbsWakeup_Test");
    UtTest_Add(SC_CompareAbsTime_Test_True, SC_Test_Setup, SC_Test_TearDown, "SC_CompareAbsTime_Test_True");
    UtTest_Add(SC_CompareAbsTime_Test_False, SC_Test_Setup, SC_Test_TearDown, "SC_CompareAbsTime_Test_False");
    UtTest_Add(SC_ToggleAtsIndex_Test, SC_Test_Setup, SC_Test_TearDown, "SC_ToggleAtsIndex_Test");
}
