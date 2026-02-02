/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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
CFE_Status_t SC_AppendAtsCmd(const SC_AppendAtsCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_AppendAtsCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_AppendAtsCmd, const SC_AppendAtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_AppendAtsCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_AppendAtsCmd, CFE_Status_t);
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
CFE_Status_t SC_ContinueAtsOnFailureCmd(const SC_ContinueAtsOnFailureCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_ContinueAtsOnFailureCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_ContinueAtsOnFailureCmd, const SC_ContinueAtsOnFailureCmd_t *, Cmd);

    UT_GenStub_Execute(SC_ContinueAtsOnFailureCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_ContinueAtsOnFailureCmd, CFE_Status_t);
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
CFE_Status_t SC_JumpAtsCmd(const SC_JumpAtsCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_JumpAtsCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_JumpAtsCmd, const SC_JumpAtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_JumpAtsCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_JumpAtsCmd, CFE_Status_t);
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
CFE_Status_t SC_StartAtsCmd(const SC_StartAtsCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_StartAtsCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_StartAtsCmd, const SC_StartAtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_StartAtsCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_StartAtsCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_StopAtsCmd()
 * ----------------------------------------------------
 */
CFE_Status_t SC_StopAtsCmd(const SC_StopAtsCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_StopAtsCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_StopAtsCmd, const SC_StopAtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_StopAtsCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_StopAtsCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_SwitchAtsCmd()
 * ----------------------------------------------------
 */
CFE_Status_t SC_SwitchAtsCmd(const SC_SwitchAtsCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_SwitchAtsCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_SwitchAtsCmd, const SC_SwitchAtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_SwitchAtsCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_SwitchAtsCmd, CFE_Status_t);
}
