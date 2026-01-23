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
 * Auto-Generated stub implementations for functions defined in sc_rtsrq header
 */

#include "sc_rtsrq.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for SC_AutoStartRts()
 * ----------------------------------------------------
 */
void SC_AutoStartRts(SC_RtsNum_t RtsNum)
{
    UT_GenStub_AddParam(SC_AutoStartRts, SC_RtsNum_t, RtsNum);

    UT_GenStub_Execute(SC_AutoStartRts, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_DisableRtsCmd()
 * ----------------------------------------------------
 */
CFE_Status_t SC_DisableRtsCmd(const SC_DisableRtsCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_DisableRtsCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_DisableRtsCmd, const SC_DisableRtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_DisableRtsCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_DisableRtsCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_DisableRtsGrpCmd()
 * ----------------------------------------------------
 */
CFE_Status_t SC_DisableRtsGrpCmd(const SC_DisableRtsGrpCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_DisableRtsGrpCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_DisableRtsGrpCmd, const SC_DisableRtsGrpCmd_t *, Cmd);

    UT_GenStub_Execute(SC_DisableRtsGrpCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_DisableRtsGrpCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_EnableRtsCmd()
 * ----------------------------------------------------
 */
CFE_Status_t SC_EnableRtsCmd(const SC_EnableRtsCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_EnableRtsCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_EnableRtsCmd, const SC_EnableRtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_EnableRtsCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_EnableRtsCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_EnableRtsGrpCmd()
 * ----------------------------------------------------
 */
CFE_Status_t SC_EnableRtsGrpCmd(const SC_EnableRtsGrpCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_EnableRtsGrpCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_EnableRtsGrpCmd, const SC_EnableRtsGrpCmd_t *, Cmd);

    UT_GenStub_Execute(SC_EnableRtsGrpCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_EnableRtsGrpCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_KillRts()
 * ----------------------------------------------------
 */
void SC_KillRts(SC_RtsIndex_t RtsIndex)
{
    UT_GenStub_AddParam(SC_KillRts, SC_RtsIndex_t, RtsIndex);

    UT_GenStub_Execute(SC_KillRts, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_StartRtsCmd()
 * ----------------------------------------------------
 */
CFE_Status_t SC_StartRtsCmd(const SC_StartRtsCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_StartRtsCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_StartRtsCmd, const SC_StartRtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_StartRtsCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_StartRtsCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_StartRtsGrpCmd()
 * ----------------------------------------------------
 */
CFE_Status_t SC_StartRtsGrpCmd(const SC_StartRtsGrpCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_StartRtsGrpCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_StartRtsGrpCmd, const SC_StartRtsGrpCmd_t *, Cmd);

    UT_GenStub_Execute(SC_StartRtsGrpCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_StartRtsGrpCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_StopRtsCmd()
 * ----------------------------------------------------
 */
CFE_Status_t SC_StopRtsCmd(const SC_StopRtsCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_StopRtsCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_StopRtsCmd, const SC_StopRtsCmd_t *, Cmd);

    UT_GenStub_Execute(SC_StopRtsCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_StopRtsCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_StopRtsGrpCmd()
 * ----------------------------------------------------
 */
CFE_Status_t SC_StopRtsGrpCmd(const SC_StopRtsGrpCmd_t *Cmd)
{
    UT_GenStub_SetupReturnBuffer(SC_StopRtsGrpCmd, CFE_Status_t);

    UT_GenStub_AddParam(SC_StopRtsGrpCmd, const SC_StopRtsGrpCmd_t *, Cmd);

    UT_GenStub_Execute(SC_StopRtsGrpCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_StopRtsGrpCmd, CFE_Status_t);
}
