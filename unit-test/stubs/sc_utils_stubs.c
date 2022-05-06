
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

SC_AbsTimeTag_t SC_ComputeAbsTime(uint16 RelTime)
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
