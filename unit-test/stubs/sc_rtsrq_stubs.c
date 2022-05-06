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
