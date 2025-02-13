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
 * Auto-Generated stub implementations for functions defined in sc_atsrq header
 */

#include "sc_atsrq.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for SC_AppendAtsCmd()
 * ----------------------------------------------------
 */
void SC_AppendAtsCmd(const SC_AppendAtsCmd_t *Cmd)
{
    UT_GenStub_AddParam(SC_AppendAtsCmd, const SC_AppendAtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_AppendAtsCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_BeginAts()
 * ----------------------------------------------------
 */
bool SC_BeginAts(SC_AtsIndex_t AtsIndex, uint16 TimeOffset)
{
    UT_GenStub_SetupReturnBuffer(SC_BeginAts, bool);

    UT_GenStub_AddParam(SC_BeginAts, SC_AtsIndex_t, AtsIndex);
    UT_GenStub_AddParam(SC_BeginAts, uint16, TimeOffset);

    UT_GenStub_Execute(SC_BeginAts, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_BeginAts, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ContinueAtsOnFailureCmd()
 * ----------------------------------------------------
 */
void SC_ContinueAtsOnFailureCmd(const SC_ContinueAtsOnFailureCmd_t *Cmd)
{
    UT_GenStub_AddParam(SC_ContinueAtsOnFailureCmd, const SC_ContinueAtsOnFailureCmd_t *, Cmd);

    UT_GenStub_Execute(SC_ContinueAtsOnFailureCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_InlineSwitch()
 * ----------------------------------------------------
 */
bool SC_InlineSwitch(void)
{
    UT_GenStub_SetupReturnBuffer(SC_InlineSwitch, bool);

    UT_GenStub_Execute(SC_InlineSwitch, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_InlineSwitch, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_JumpAtsCmd()
 * ----------------------------------------------------
 */
void SC_JumpAtsCmd(const SC_JumpAtsCmd_t *Cmd)
{
    UT_GenStub_AddParam(SC_JumpAtsCmd, const SC_JumpAtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_JumpAtsCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_KillAts()
 * ----------------------------------------------------
 */
void SC_KillAts(void)
{
    UT_GenStub_Execute(SC_KillAts, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ServiceSwitchPend()
 * ----------------------------------------------------
 */
void SC_ServiceSwitchPend(void)
{
    UT_GenStub_Execute(SC_ServiceSwitchPend, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_StartAtsCmd()
 * ----------------------------------------------------
 */
void SC_StartAtsCmd(const SC_StartAtsCmd_t *Cmd)
{
    UT_GenStub_AddParam(SC_StartAtsCmd, const SC_StartAtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_StartAtsCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_StopAtsCmd()
 * ----------------------------------------------------
 */
void SC_StopAtsCmd(const SC_StopAtsCmd_t *Cmd)
{
    UT_GenStub_AddParam(SC_StopAtsCmd, const SC_StopAtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_StopAtsCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_SwitchAtsCmd()
 * ----------------------------------------------------
 */
void SC_SwitchAtsCmd(const SC_SwitchAtsCmd_t *Cmd)
{
    UT_GenStub_AddParam(SC_SwitchAtsCmd, const SC_SwitchAtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_SwitchAtsCmd, Basic, NULL);
}
