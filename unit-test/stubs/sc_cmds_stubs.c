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

#include "sc_cmds.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

void SC_TableManageCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_TableManageCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_TableManageCmd);
}

void SC_ManageRtsTable(int32 ArrayIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_ManageRtsTable), ArrayIndex);
    UT_DEFAULT_IMPL(SC_ManageRtsTable);
}

void SC_ManageAtsTable(int32 ArrayIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(SC_ManageAtsTable), ArrayIndex);
    UT_DEFAULT_IMPL(SC_ManageAtsTable);
}

void SC_ManageAppendTable(void)
{
    UT_DEFAULT_IMPL(SC_ManageAppendTable);
}

void SC_ProcessRequest(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_ProcessRequest), CmdPacket);
    UT_DEFAULT_IMPL(SC_ProcessRequest);
}

void SC_ProcessCommand(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_ProcessCommand), CmdPacket);
    UT_DEFAULT_IMPL(SC_ProcessCommand);
}

void SC_NoOpCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_NoOpCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_NoOpCmd);
}

void SC_ResetCountersCmd(const CFE_SB_Buffer_t *CmdPacket)
{
    UT_Stub_RegisterContext(UT_KEY(SC_ResetCountersCmd), CmdPacket);
    UT_DEFAULT_IMPL(SC_ResetCountersCmd);
}

void SC_SendHkPacket(void)
{
    UT_DEFAULT_IMPL(SC_SendHkPacket);
}

void SC_ProcessAtpCmd(void)
{
    UT_DEFAULT_IMPL(SC_ProcessAtpCmd);
}

void SC_ProcessRtpCommand(void)
{
    UT_DEFAULT_IMPL(SC_ProcessRtpCommand);
}
