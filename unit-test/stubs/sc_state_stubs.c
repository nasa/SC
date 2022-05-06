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

#include "sc_state.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

void SC_GetNextRtsTime(void)
{
    UT_DEFAULT_IMPL(SC_GetNextRtsTime);
}

void SC_UpdateNextTime(void)
{
    UT_DEFAULT_IMPL(SC_UpdateNextTime);
}

void SC_GetNextRtsCommand(void)
{
    UT_DEFAULT_IMPL(SC_GetNextRtsCommand);
}

void SC_GetNextAtsCommand(void)
{
    UT_DEFAULT_IMPL(SC_GetNextAtsCommand);
}
