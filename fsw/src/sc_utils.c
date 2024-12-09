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
 *   This file contains the utilty functions for Stored Command
 */

/**************************************************************************
 **
 ** Include section
 **
 **************************************************************************/

#include "cfe.h"
#include "sc_utils.h"
#include "sc_events.h"
#include "sc_msgids.h"
#include <string.h>

/**************************************************************************
 **
 ** Functions
 **
 **************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Look up the time accessor corresponding to the chosen reference */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
SC_TimeAccessor_t SC_LookupTimeAccessor(SC_TimeRef_Enum_t TimeRef)
{
    static const SC_TimeAccessor_t TIMEREF_LOOKUP[SC_TimeRef_MAX] = {[SC_TimeRef_USE_CFE_TIME] = {CFE_TIME_GetTime},
                                                                     [SC_TimeRef_USE_TAI]      = {CFE_TIME_GetTAI},
                                                                     [SC_TimeRef_USE_UTC]      = {CFE_TIME_GetUTC}};

    if (TimeRef >= SC_TimeRef_MAX)
    {
        TimeRef = SC_TimeRef_USE_CFE_TIME;
    }

    return TIMEREF_LOOKUP[TimeRef];
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Get the Current time from CFE TIME                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_GetCurrentTime(void)
{
    CFE_TIME_SysTime_t TempTime;

    /* Use SC defined time */
    TempTime = SC_AppData.TimeRef.GetTime();

    /* We don't care about subseconds */
    SC_AppData.CurrentTime = TempTime.Seconds;
}

SC_AbsTimeTag_t SC_GetAtsEntryTime(SC_AtsEntryHeader_t *Entry)
{
    /*
    ** ATS Entry Header looks like this...
    **
    **    uint16 Pad;
    **    uint16 CmdNumber;
    **
    **    uint16 TimeTag_MS;
    **    uint16 TimeTag_LS;
    **
    **    CFE_SB_Buffer_t Buffer;
    **
    ** The command packet data is variable length,
    **    only the command packet header is shown here.
    */

    return ((Entry->TimeTag_MS << 16) + Entry->TimeTag_LS);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Compute Absolute time from relative time                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
SC_AbsTimeTag_t SC_ComputeAbsTime(uint32 RelTime)
{
    CFE_TIME_SysTime_t AbsoluteTimeWSubs;
    CFE_TIME_SysTime_t RelTimeWSubs;
    CFE_TIME_SysTime_t ResultTimeWSubs;
    /*
     ** get the current time
     */
    AbsoluteTimeWSubs.Seconds    = SC_AppData.CurrentTime;
    AbsoluteTimeWSubs.Subseconds = 0;

    RelTimeWSubs.Seconds    = RelTime;
    RelTimeWSubs.Subseconds = 0;
    /*
     ** add the relative time the current time
     */
    ResultTimeWSubs = CFE_TIME_Add(AbsoluteTimeWSubs, RelTimeWSubs);

    /* We don't need subseconds */
    return ResultTimeWSubs.Seconds;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Compute absolute wakeup count from relative wakeup count        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 SC_ComputeAbsWakeup(uint32 RelWakeup)
{
    return SC_AppData.CurrentWakeupCount + RelWakeup;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  Compare absolute times                                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool SC_CompareAbsTime(SC_AbsTimeTag_t AbsTime1, SC_AbsTimeTag_t AbsTime2)
{
    bool               Status;
    CFE_TIME_SysTime_t Time1WSubs;
    CFE_TIME_SysTime_t Time2WSubs;
    CFE_TIME_Compare_t Result;

    Time1WSubs.Seconds    = AbsTime1;
    Time1WSubs.Subseconds = 0;

    Time2WSubs.Seconds    = AbsTime2;
    Time2WSubs.Subseconds = 0;

    Result = CFE_TIME_Compare(Time1WSubs, Time2WSubs);

    if (Result == CFE_TIME_A_GT_B)
    {
        Status = true;
    }
    else
    {
        Status = false;
    }

    return Status;
}

SC_AtsIndex_t SC_ToggleAtsIndex(void)
{
    SC_AtsIndex_t CurrAtsIndex = SC_AtsNumToIndex(SC_OperData.AtsCtrlBlckAddr->CurrAtsNum);

    SC_IDX_INCREMENT(CurrAtsIndex);
    if (!SC_AtsIndexIsValid(CurrAtsIndex))
    {
        CurrAtsIndex = SC_ATS_IDX_C(0);
    }

    return CurrAtsIndex;
}
