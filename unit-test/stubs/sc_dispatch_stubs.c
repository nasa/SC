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
 * Auto-Generated stub implementations for functions defined in sc_dispatch
 * header
 */

#include "sc_dispatch.h"
#include "utgenstub.h"

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
 * Generated stub function for SC_VerifyCmdLength()
 * ----------------------------------------------------
 */
bool SC_VerifyCmdLength(const CFE_MSG_Message_t *Msg, size_t ExpectedLength)
{
    UT_GenStub_SetupReturnBuffer(SC_VerifyCmdLength, bool);

    UT_GenStub_AddParam(SC_VerifyCmdLength, const CFE_MSG_Message_t *, Msg);
    UT_GenStub_AddParam(SC_VerifyCmdLength, size_t, ExpectedLength);

    UT_GenStub_Execute(SC_VerifyCmdLength, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_VerifyCmdLength, bool);
}
