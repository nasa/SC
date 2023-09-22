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
 * Generated stub function for SC_NoOpCmd()
 * ----------------------------------------------------
 */
void SC_NoOpCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_AddParam(SC_NoOpCmd, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(SC_NoOpCmd, Basic, NULL);
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
 * Generated stub function for SC_ProcessCommand()
 * ----------------------------------------------------
 */
void SC_ProcessCommand(const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_AddParam(SC_ProcessCommand, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(SC_ProcessCommand, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_ProcessRequest()
 * ----------------------------------------------------
 */
void SC_ProcessRequest(const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_AddParam(SC_ProcessRequest, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(SC_ProcessRequest, Basic, NULL);
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
void SC_ResetCountersCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_AddParam(SC_ResetCountersCmd, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(SC_ResetCountersCmd, Basic, NULL);
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

/*
 * ----------------------------------------------------
 * Generated stub function for SC_TableManageCmd()
 * ----------------------------------------------------
 */
void SC_TableManageCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_AddParam(SC_TableManageCmd, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(SC_TableManageCmd, Basic, NULL);
}
