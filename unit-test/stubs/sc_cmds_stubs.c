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
 * Auto-Generated stub implementations for functions defined in sc_cmds header
 */

#include "sc_cmds.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ManageAtsTable()
 * ----------------------------------------------------
 */
void SC_ManageAtsTable(int32 ArrayIndex)
{
    UT_GenStub_AddParam(SC_ManageAtsTable, int32, ArrayIndex);

    UT_GenStub_Execute(SC_ManageAtsTable, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ManageRtsTable()
 * ----------------------------------------------------
 */
void SC_ManageRtsTable(int32 ArrayIndex)
{
    UT_GenStub_AddParam(SC_ManageRtsTable, int32, ArrayIndex);

    UT_GenStub_Execute(SC_ManageRtsTable, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ManageTable()
 * ----------------------------------------------------
 */
void SC_ManageTable(SC_TableType type, int32 ArrayIndex)
{
    UT_GenStub_AddParam(SC_ManageTable, SC_TableType, type);
    UT_GenStub_AddParam(SC_ManageTable, int32, ArrayIndex);

    UT_GenStub_Execute(SC_ManageTable, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ManageTableCmd()
 * ----------------------------------------------------
 */
void SC_ManageTableCmd(const SC_ManageTableCmd_t *Cmd)
{
    UT_GenStub_AddParam(SC_ManageTableCmd, const SC_ManageTableCmd_t *, Cmd);

    UT_GenStub_Execute(SC_ManageTableCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_NoopCmd()
 * ----------------------------------------------------
 */
void SC_NoopCmd(const SC_NoopCmd_t *Cmd)
{
    UT_GenStub_AddParam(SC_NoopCmd, const SC_NoopCmd_t *, Cmd);

    UT_GenStub_Execute(SC_NoopCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_WakeupCmd()
 * ----------------------------------------------------
 */
void SC_WakeupCmd(const SC_WakeupCmd_t *Cmd)
{
    UT_GenStub_AddParam(SC_WakeupCmd, const SC_WakeupCmd_t *, Cmd);

    UT_GenStub_Execute(SC_WakeupCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ProcessAtpCmd()
 * ----------------------------------------------------
 */
void SC_ProcessAtpCmd(void)
{
    UT_GenStub_Execute(SC_ProcessAtpCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ProcessRtpCommand()
 * ----------------------------------------------------
 */
void SC_ProcessRtpCommand(void)
{
    UT_GenStub_Execute(SC_ProcessRtpCommand, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ResetCountersCmd()
 * ----------------------------------------------------
 */
void SC_ResetCountersCmd(const SC_ResetCountersCmd_t *Cmd)
{
    UT_GenStub_AddParam(SC_ResetCountersCmd, const SC_ResetCountersCmd_t *, Cmd);

    UT_GenStub_Execute(SC_ResetCountersCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_SendHkCmd()
 * ----------------------------------------------------
 */
void SC_SendHkCmd(const SC_SendHkCmd_t *Cmd)
{
    UT_GenStub_AddParam(SC_SendHkCmd, const SC_SendHkCmd_t *, Cmd);

    UT_GenStub_Execute(SC_SendHkCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_SendHkPacket()
 * ----------------------------------------------------
 */
void SC_SendHkPacket(void)
{
    UT_GenStub_Execute(SC_SendHkPacket, Basic, NULL);
}
