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
