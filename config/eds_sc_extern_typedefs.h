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
 *  The CFS Data Storage (DS) Application table related
 *   constant definitions.
 */
#ifndef EDS_SC_EXTERN_TYPEDEFS_H
#define EDS_SC_EXTERN_TYPEDEFS_H

#include "sc_eds_typedefs.h"

typedef SC_AtsNum_Atom_t      SC_AtsNum_t;
typedef SC_RtsNum_Atom_t      SC_RtsNum_t;
typedef SC_CommandNum_Atom_t  SC_CommandNum_t;
typedef SC_EntryOffset_Atom_t SC_EntryOffset_t;

/**
 * Convert from an ID or Number value (e.g. RTS/ATS identifier) to a native unsigned int
 *
 * This is mainly intended for printf()-style logging, where it should be paired
 * with the "%u" conversion specifier.
 */
#define SC_IDNUM_AS_UINT(arg) ((unsigned int)(arg))

/**
 * Convert from a native integer value (e.g. a literal) to an ID or Number value
 *
 * This is mainly intended for initializing values from literals or integers
 * This is the inverse macro of SC_IDNUM_AS_UINT()
 */
#define SC_IDNUM_FROM_UINT(arg) ((uint16)(arg))

/* _INITIALIZER macros to be used in static (e.g. table) definitions that need to resolve at compile time */
#define SC_RTS_NUM_INITIALIZER(i)     SC_IDNUM_FROM_UINT(i)
#define SC_ATS_NUM_INITIALIZER(i)     SC_IDNUM_FROM_UINT(i)
#define SC_COMMAND_NUM_INITIALIZER(i) SC_IDNUM_FROM_UINT(i)

#define SC_IDNUM_EQUAL(arg1, arg2) (SC_IDNUM_AS_UINT(arg1) == SC_IDNUM_AS_UINT(arg2))
#define SC_IDNUM_IS_NULL(arg)      (SC_IDNUM_AS_UINT(arg) == 0)

/* _C macros to be used in other places that need to resolve at runtime time - these are type safe */
#define SC_RTS_NUM_C(i)     ((SC_RtsNum_t)SC_IDNUM_FROM_UINT(i))
#define SC_ATS_NUM_C(i)     ((SC_AtsNum_t)SC_IDNUM_FROM_UINT(i))
#define SC_COMMAND_NUM_C(i) ((SC_CommandNum_t)SC_IDNUM_FROM_UINT(i))

/* _NULL macros refer to a value that is always reserved */
#define SC_RTS_NUM_NULL     SC_RTS_NUM_C(0)
#define SC_ATS_NUM_NULL     SC_ATS_NUM_C(0)
#define SC_COMMAND_NUM_NULL SC_COMMAND_NUM_C(0)

#endif
