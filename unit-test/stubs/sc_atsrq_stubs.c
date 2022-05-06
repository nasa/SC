
/*
 * Includes
 */

#include "sc_atsrq.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

bool SC_BeginAts(uint16 AtsId, uint16 TimeOffset)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_BeginAts), AtsId);
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_BeginAts), TimeOffset);
    return UT_DEFAULT_IMPL(SC_BeginAts);
}

void SC_StartAtsCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_StartAtsCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_StartAtsCmd);
}

void SC_StopAtsCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_StopAtsCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_StopAtsCmd);
}

void SC_KillAts(void)
{
    UT_DEFAULT_IMPL(SC_KillAts);
}

void SC_GroundSwitchCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_GroundSwitchCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_GroundSwitchCmd);
}

bool SC_InlineSwitch(void)
{
    return UT_DEFAULT_IMPL(SC_InlineSwitch);
}

void SC_ServiceSwitchPend(void)
{
    UT_DEFAULT_IMPL(SC_ServiceSwitchPend);
}

void SC_JumpAtsCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_JumpAtsCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_JumpAtsCmd);
}

void SC_ContinueAtsOnFailureCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_ContinueAtsOnFailureCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_ContinueAtsOnFailureCmd);
}

void SC_AppendAtsCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_AppendAtsCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_AppendAtsCmd);
}
