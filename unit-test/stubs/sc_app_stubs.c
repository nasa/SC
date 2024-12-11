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
 * Auto-Generated stub implementations for functions defined in sc_app header
 */

#include "sc_app.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for SC_AppInit()
 * ----------------------------------------------------
 */
CFE_Status_t SC_AppInit(void)
{
    UT_GenStub_SetupReturnBuffer(SC_AppInit, CFE_Status_t);

    UT_GenStub_Execute(SC_AppInit, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_AppInit, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_AppMain()
 * ----------------------------------------------------
 */
void SC_AppMain(void)
{
    UT_GenStub_Execute(SC_AppMain, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_GetDumpTablePointers()
 * ----------------------------------------------------
 */
CFE_Status_t SC_GetDumpTablePointers(void)
{
    UT_GenStub_SetupReturnBuffer(SC_GetDumpTablePointers, CFE_Status_t);

    UT_GenStub_Execute(SC_GetDumpTablePointers, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_GetDumpTablePointers, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_GetLoadTablePointers()
 * ----------------------------------------------------
 */
CFE_Status_t SC_GetLoadTablePointers(void)
{
    UT_GenStub_SetupReturnBuffer(SC_GetLoadTablePointers, CFE_Status_t);

    UT_GenStub_Execute(SC_GetLoadTablePointers, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_GetLoadTablePointers, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_InitTables()
 * ----------------------------------------------------
 */
CFE_Status_t SC_InitTables(void)
{
    UT_GenStub_SetupReturnBuffer(SC_InitTables, CFE_Status_t);

    UT_GenStub_Execute(SC_InitTables, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_InitTables, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_LoadDefaultTables()
 * ----------------------------------------------------
 */
void SC_LoadDefaultTables(void)
{
    UT_GenStub_Execute(SC_LoadDefaultTables, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_RegisterAllTables()
 * ----------------------------------------------------
 */
CFE_Status_t SC_RegisterAllTables(void)
{
    UT_GenStub_SetupReturnBuffer(SC_RegisterAllTables, CFE_Status_t);

    UT_GenStub_Execute(SC_RegisterAllTables, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_RegisterAllTables, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_RegisterDumpOnlyTables()
 * ----------------------------------------------------
 */
CFE_Status_t SC_RegisterDumpOnlyTables(void)
{
    UT_GenStub_SetupReturnBuffer(SC_RegisterDumpOnlyTables, CFE_Status_t);

    UT_GenStub_Execute(SC_RegisterDumpOnlyTables, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_RegisterDumpOnlyTables, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_RegisterLoadableTables()
 * ----------------------------------------------------
 */
CFE_Status_t SC_RegisterLoadableTables(void)
{
    UT_GenStub_SetupReturnBuffer(SC_RegisterLoadableTables, CFE_Status_t);

    UT_GenStub_Execute(SC_RegisterLoadableTables, Basic, NULL);

    return UT_GenStub_GetReturnValue(SC_RegisterLoadableTables, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for SC_RegisterManageCmds()
 * ----------------------------------------------------
 */
void SC_RegisterManageCmds(void)
{
    UT_GenStub_Execute(SC_RegisterManageCmds, Basic, NULL);
}
