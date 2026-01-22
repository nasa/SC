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
 * Definition of CFS Stored Command (SC) types that are shared between
 * tables, messages, or otherwise need to be known publicly.
 */
#ifndef SC_EXTERN_TYPEDEFS_H
#define SC_EXTERN_TYPEDEFS_H

#include "common_types.h"

/**
 * ATS/RTS Cmd Status Enumeratoion
 */
enum SC_Status
{
    SC_Status_EMPTY,           /**< \brief the object is not loaded */
    SC_Status_LOADED,          /**< \brief the object is loaded */
    SC_Status_IDLE,            /**< \brief the object is not executing */
    SC_Status_EXECUTED,        /**< \brief the object has completed executing */
    SC_Status_SKIPPED,         /**< \brief the object (ats command) was skipped */
    SC_Status_EXECUTING,       /**< \brief the object is currently executing */
    SC_Status_FAILED_CHECKSUM, /**< \brief the object failed a checksum test */
    SC_Status_FAILED_DISTRIB,  /**< \brief the object could not be sent on the SWB */
    SC_Status_STARTING         /**< \brief used when an inline switch is executed */
};

typedef uint8 SC_Status_Enum_t;

#ifndef SC_OMIT_DEPRECATED
/**
 * \name Old-style ATS/RTS Cmd Status macros
 * \{
 */
#define SC_EMPTY           SC_Status_EMPTY
#define SC_LOADED          SC_Status_LOADED
#define SC_IDLE            SC_Status_IDLE
#define SC_EXECUTED        SC_Status_EXECUTED
#define SC_SKIPPED         SC_Status_SKIPPED
#define SC_EXECUTING       SC_Status_EXECUTING
#define SC_FAILED_CHECKSUM SC_Status_FAILED_CHECKSUM
#define SC_FAILED_DISTRIB  SC_Status_FAILED_DISTRIB
#define SC_STARTING        SC_Status_STARTING
/**\}*/
#endif

/**
 * @brief An identifier for RTS's
 *
 * This is a 1-based numeric value that refers to a specific RTS.
 * The value of 0 is reserved and is considered invalid/null.
 *
 * The valid range is [1..SC_NUMBER_OF_RTS] (inclusive)
 *
 * @note Some code and documentation may also refer to this as an RTS Number.
 * This is synonymous with an RTS ID.
 */
typedef uint16 SC_RtsNum_t;

/**
 * @brief An identifier for ATS's
 *
 * This is a 1-based numeric value that refers to a specific ATS.
 * The value of 0 is reserved and is considered invalid/null.
 *
 * The valid range is [1..SC_NUMBER_OF_ATS] (inclusive)
 *
 * @note Some code and documentation may also refer to this as an ATS Number.
 * This is synonymous with an ATS ID.
 *
 * Unlike RTS, in many circumstances an alphabetic identifier is also used
 * to identify an ATS (e.g. ATS A, ATS B, etc).  This is a simple mapping where
 * A refers to ATS ID 1, B refers to ATS ID 2, etc.
 */
typedef uint16 SC_AtsNum_t;

/**
 * A command number for ATS's
 *
 * This is a 1-based numeric value that refers to a specific
 * command within an ATS.  Each entry within an ATS has one of
 * these identifiers on each command in it.
 *
 * @note RTS sequences do not use this identifier, as these
 * commands only have a relative offset from the previous command.
 *
 * The value of 0 is reserved and is considered invalid/null.
 *
 * The valid range is [1..SC_MAX_ATS_CMDS] (inclusive)
 *
 * IMPORTANT: This number only serves to uniquely identify a
 * specific command within an ATS.  It is _not_ necessarily the
 * same as a sequence number within the ATS, as commands may be
 * defined in the table any order (that is, they may have absolute
 * time stamps that are not in sequence).
 */
typedef uint16 SC_CommandNum_t;

/**
 * @brief Represents an offset into an ATS or RTS buffer
 *
 * This is a 0-based numeric value that refers to a 32-bit word position
 * within the ATS or RTS buffer.  This can be used to look up the
 * specific command at that position.
 *
 * The valid range is [0..(SC_ATS_BUFF_SIZE/4)-1] for ATS
 * or [0..(SC_RTS_BUFF_SIZE/4)-1] for RTS
 *
 * @note ATS/RTS Buffers are indexed using 32-bit words.
 * To get a byte offset, this value needs to be multiplied by 4.
 */
typedef uint16 SC_EntryOffset_t;

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
