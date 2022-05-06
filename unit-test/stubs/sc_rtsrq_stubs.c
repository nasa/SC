
/*
 * Includes
 */

#include "sc_rtsrq.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

void SC_StartRtsCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_StartRtsCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_StartRtsCmd);
}

#if (SC_ENABLE_GROUP_COMMANDS == TRUE)
void SC_StartRtsGrpCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_StartRtsGrpCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_StartRtsGrpCmd);
}

#endif
void SC_StopRtsCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_StopRtsCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_StopRtsCmd);
}

#if (SC_ENABLE_GROUP_COMMANDS == TRUE)
void SC_StopRtsGrpCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_StopRtsGrpCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_StopRtsGrpCmd);
}

#endif
void SC_DisableRtsCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_DisableRtsCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_DisableRtsCmd);
}

#if (SC_ENABLE_GROUP_COMMANDS == TRUE)
void SC_DisableRtsGrpCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_DisableRtsGrpCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_DisableRtsGrpCmd);
}
#endif

void SC_EnableRtsCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_EnableRtsCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_EnableRtsCmd);
}

#if (SC_ENABLE_GROUP_COMMANDS == TRUE)
void SC_EnableRtsGrpCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_EnableRtsGrpCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_EnableRtsGrpCmd);
}
#endif

void SC_KillRts(uint16 RtsIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_KillRts), RtsIndex);
    UT_DEFAULT_IMPL(SC_KillRts);
}

void SC_AutoStartRts(uint16 RtsNumber)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_AutoStartRts), RtsNumber);
    UT_DEFAULT_IMPL(SC_AutoStartRts);
}
