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
 * Defines data types used for internal structure/table indexing
 */
#ifndef SC_INDEX_TYPES_H
#define SC_INDEX_TYPES_H

/*************************************************************************
 * Includes
 *************************************************************************/

#include "common_types.h"
#include "sc_extern_typedefs.h"
#include "sc_platform_cfg.h"

/*************************************************************************
 * Types
 *************************************************************************/

/*
 * BACKGROUND INFO:
 *
 * The SC external interface (tables/commands/telemetry) uses non-zero identifiers
 * for all object references.  This is beneficial because "0" can be reserved
 * as an invalid/undefined value, and interpreted as "null" if it should appear.
 *
 * However, because all objects are stored in arrays and C arrays start at index 0,
 * there needs to be a conversion between the externally-used identifier and the
 * internal index used during the actual processing.
 */

/**
 * @brief An index type for RTS's.
 *
 * This is a 0-based value that refers to an internal
 * array index for the RTS tables.
 *
 * The valid range is [0..SC_NUMBER_OF_RTS-1]
 */
typedef uint16 SC_RtsIndex_t;

/**
 * @brief An index type for ATS's.
 *
 * This is a 0-based value that refers to an internal
 * array index for the ATS tables.
 *
 * The valid range is [0..SC_NUMBER_OF_ATS-1]
 */
typedef uint16 SC_AtsIndex_t;

/**
 * @brief An index type for ATS commands.
 *
 * This is a 0-based value that refers to an internal
 * array index for the ATS command lookup tables.
 *
 * The valid range is [0..SC_MAX_ATS_CMDS-1]
 */
typedef uint16 SC_CommandIndex_t;

/**
 * @brief A sequence number for commands.
 *
 * This value is intended to indicate the order with which commands
 * are executed within an ATS
 *
 * @note The command number in ATS entries is strictly for command
 * identification and referencing purposes.  Commands do not need to
 * appear in the table, nor be do they need to be numbered in order.
 *
 * For example, the command with SeqIndex == 3 will be the 4th command executed
 * within the ATS, after sorting by time.
 *
 * The valid range is [0..SC_MAX_ATS_CMDS-1]
 */
typedef uint16 SC_SeqIndex_t;

/*
 * ------------------------------------------------
 *  CONVERSIONS between IDs, indices, and integers
 * ------------------------------------------------
 *
 * Although these are all integers underneath, they
 * refer to different domains and thus transitioning
 * or converting between the domains should be clear.
 */

/*
 * Macros and special values associated with IDX values
 * These should work with any variant of an index type
 */

/**
 * Convert an index value to a native unsigned int
 *
 * This is mainly intended for printf()-style logging, where it should be paired
 * with the "%u" conversion specifier.
 */
#define SC_IDX_AS_UINT(arg) ((unsigned int)(arg))

/**
 * Convert a native integer to an index value
 *
 * This is mainly intended for initializing values from literals or integers
 * This is the inverse macro of SC_IDX_AS_UINT()
 */
#define SC_IDX_FROM_UINT(arg) ((uint16)(arg))
/**
 * Increment an index value
 * This works with any index type that follows the pattern
 */
#define SC_IDX_INCREMENT(arg) (++(arg))

/**
 * Decrements an index value
 * This works with any index type that follows the pattern
 */
#define SC_IDX_DECREMENT(arg) (--(arg))

/**
 * Convert an index value to a alphabetic character
 *
 * This is intended for displaying ATS identifiers as letters (A/B, etc)
 * Historically in SC there were only two ATS's (A and B) but this should
 * support up to 26.
 */
#define SC_IDX_AS_CHAR(arg) ((char)('A' + SC_IDX_AS_UINT(arg)))

/**
 * Compare two index values for equality
 */
#define SC_IDX_EQUAL(arg1, arg2) (SC_IDX_AS_UINT(arg1) == SC_IDX_AS_UINT(arg2))

/**
 * Compare if an index is less than or equal to another
 *
 * This is necessary for circumstances such as RTS group processing,
 * which requires looping through a segment/subset of indices.
 */
#define SC_IDX_LESS_OR_EQ(arg1, arg2) (SC_IDX_AS_UINT(arg1) <= SC_IDX_AS_UINT(arg2))

/**
 * Compare if an index is less than a fixed limit
 *
 * This is necessary for validating the index, that is within the
 * permissible range.  The "lim" is expected to be a fixed constant.
 */
#define SC_IDX_WITHIN_LIMIT(arg, lim) (SC_IDX_AS_UINT(arg) < (lim))

/**
 * Type-safe conversion from an ATS Number/ID to an index
 *
 * @param AtsNum   ATS Number/ID index to convert
 * @returns ATS index associated with number
 */
static inline SC_AtsIndex_t SC_AtsNumToIndex(SC_AtsNum_t AtsNum)
{
    return (SC_AtsIndex_t)SC_IDX_FROM_UINT(SC_IDNUM_AS_UINT(AtsNum) - 1);
}

/**
 * Type-safe conversion from an ATS Index to a Number/ID
 *
 * @param AtsIndex   ATS index to convert
 * @returns ATS Number associated with index
 */
static inline SC_AtsNum_t SC_AtsIndexToNum(SC_AtsIndex_t AtsIndex)
{
    return (SC_AtsNum_t)SC_IDNUM_FROM_UINT(SC_IDX_AS_UINT(AtsIndex) + 1);
}

/**
 * Type-safe conversion from an RTS Number/ID to an index
 *
 * @param RtsNum   RTS Number/ID index to convert
 * @returns RTS index associated with number
 */
static inline SC_RtsIndex_t SC_RtsNumToIndex(SC_RtsNum_t RtsNum)
{
    return (SC_RtsIndex_t)SC_IDX_FROM_UINT(SC_IDNUM_AS_UINT(RtsNum) - 1);
}

/**
 * Type-safe conversion from an RTS Index to a Number/ID
 *
 * @param RtsIndex   RTS index to convert
 * @returns RTS Number associated with index
 */
static inline SC_RtsNum_t SC_RtsIndexToNum(SC_RtsIndex_t RtsIndex)
{
    return (SC_RtsNum_t)SC_IDNUM_FROM_UINT(SC_IDX_AS_UINT(RtsIndex) + 1);
}

/**
 * Type-safe conversion from an ATS Command Number to a Command Index
 *
 * @param CommandNum   ATS Command Number/ID index to convert
 * @returns Command index associated with number
 */
static inline SC_CommandIndex_t SC_CommandNumToIndex(SC_CommandNum_t CommandNum)
{
    return (SC_CommandIndex_t)SC_IDX_FROM_UINT(SC_IDNUM_AS_UINT(CommandNum) - 1);
}

/**
 * Type-safe conversion from an ATS Command Index to a Command Number
 *
 * @param CommandIndex   ATS Command index to convert
 * @returns Command Number associated with index
 *
 */
static inline SC_CommandNum_t SC_CommandIndexToNum(SC_CommandIndex_t CommandIndex)
{
    return (SC_CommandNum_t)SC_IDNUM_FROM_UINT(SC_IDX_AS_UINT(CommandIndex) + 1);
}

/*
 * Type-specific integer conversion for each of the defined index types
 */
#define SC_COMMAND_IDX_C(i)  ((SC_CommandIndex_t)SC_IDX_FROM_UINT(i))
#define SC_RTS_IDX_C(i)      ((SC_RtsIndex_t)SC_IDX_FROM_UINT(i))
#define SC_ATS_IDX_C(i)      ((SC_AtsIndex_t)SC_IDX_FROM_UINT(i))
#define SC_SEQUENCE_IDX_C(i) ((SC_SeqIndex_t)SC_IDX_FROM_UINT(i))
#define SC_ENTRY_OFFSET_C(i) ((SC_EntryOffset_t)SC_IDX_FROM_UINT(i))

#define SC_SEQUENCE_IDX_FIRST   SC_SEQUENCE_IDX_C(0)
#define SC_SEQUENCE_IDX_INVALID SC_SEQUENCE_IDX_C(-1)
#define SC_ENTRY_OFFSET_FIRST   SC_ENTRY_OFFSET_C(0)
#define SC_ENTRY_OFFSET_INVALID SC_ENTRY_OFFSET_C(-1)

/*
 * ------------------------------------------------
 *  VALIDATION of IDs and indices
 * ------------------------------------------------
 *
 * The intent of these inline functions is to encapsulate
 * validity and range-checking on IDs and index values so
 * they can be applied uniformly across SC.
 */

/**
 * @brief Check if an ATS Index is valid
 *
 * @param AtsIndex   ATS index to check
 * @retval true if valid
 * @retval false if invalid / out of range
 */
static inline bool SC_AtsIndexIsValid(SC_AtsIndex_t AtsIndex)
{
    return SC_IDX_WITHIN_LIMIT(AtsIndex, SC_NUMBER_OF_ATS);
}

/**
 * @brief Check if an ATS Number/ID is valid
 *
 * @param AtsNum   ATS Number/ID to check
 * @retval true if valid
 * @retval false if invalid / out of range
 */
static inline bool SC_AtsNumIsValid(SC_AtsNum_t AtsNum)
{
    return SC_AtsIndexIsValid(SC_AtsNumToIndex(AtsNum));
}

/**
 * @brief Check if an RTS Index is valid
 *
 * @param RtsIndex   RTS index to check
 * @retval true if valid
 * @retval false if invalid / out of range
 */
static inline bool SC_RtsIndexIsValid(SC_RtsIndex_t RtsIndex)
{
    return SC_IDX_WITHIN_LIMIT(RtsIndex, SC_NUMBER_OF_RTS);
}

/**
 * @brief Check if an RTS Number/ID is valid
 *
 * @param RtsNum   RTS Number/ID to check
 * @retval true if valid
 * @retval false if invalid / out of range
 */
static inline bool SC_RtsNumIsValid(SC_RtsNum_t RtsNum)
{
    return SC_RtsIndexIsValid(SC_RtsNumToIndex(RtsNum));
}

/**
 * @brief Check if an ATS Command Index is valid
 *
 * @param CommandIndex   Command index to check
 * @retval true if valid
 * @retval false if invalid / out of range
 */
static inline bool SC_AtsCommandIndexIsValid(SC_CommandIndex_t CommandIndex)
{
    return SC_IDX_WITHIN_LIMIT(CommandIndex, SC_MAX_ATS_CMDS);
}

/**
 * @brief Check if an ATS Command Number/ID is valid
 *
 * @param CommandNum   Command Number/ID to check
 * @retval true if valid
 * @retval false if invalid / out of range
 */
static inline bool SC_AtsCommandNumIsValid(SC_CommandNum_t CommandNum)
{
    return SC_AtsCommandIndexIsValid(SC_CommandNumToIndex(CommandNum));
}

/**
 * @brief Check if the given RTS Index should use the event reporting subsystem
 *
 * A subset of RTS values may send events, and this is configurable
 * separately from the overall RTS limit
 *
 * @param RtsIndex   RTS index to check
 * @retval true if RTS uses events
 * @retval false if RTS does not use events
 */
static inline bool SC_RtsIndexHasEvent(SC_RtsIndex_t RtsIndex)
{
    return (SC_IDX_WITHIN_LIMIT(RtsIndex, SC_LAST_RTS_WITH_EVENTS));
}

/**
 * @brief Check if the given RTS Number/ID should use the event reporting subsystem
 *
 * A subset of RTS values may send events, and this is configurable
 * separately from the overall RTS limit
 *
 * @param RtsNum  RTS Number/ID to check
 * @retval true if RTS uses events
 * @retval false if RTS does not use events
 */
static inline bool SC_RtsNumHasEvent(SC_RtsNum_t RtsNum)
{
    return SC_RtsIndexHasEvent(SC_RtsNumToIndex(RtsNum));
}

/**
 * @brief Check if the given RTS Number/ID is less than or equal to the limit
 *
 * This is used when looping over multiple IDs, to check when the end
 * of the range is reached.  This concept exists for RTS group commands.
 *
 * @param RtsNum   RTS Number/ID to check
 * @param IdLimit RTS Number/ID that refers to upper end of limit
 * @retval true if within range
 * @retval false if out of range
 */
static inline bool SC_RtsNumWithinRange(SC_RtsNum_t RtsNum, SC_RtsNum_t IdLimit)
{
    return (SC_IDNUM_AS_UINT(RtsNum) <= SC_IDNUM_AS_UINT(IdLimit));
}

/**
 * @brief Check if the given RTS Number/ID range is valid
 *
 * This concept exists for RTS group commands, to confirm that the "first"
 * and "last" IDs specified in the range are acceptable.
 *
 * @param FirstRtsNum  First RTS Id of range (inclusive)
 * @param LastRtsNum   Last RTS Id of range (inclusive)
 * @retval true if range is valid
 * @retval false if range is not valid
 */
static inline bool SC_RtsNumValidateRange(SC_RtsNum_t FirstRtsNum, SC_RtsNum_t LastRtsNum)
{
    return SC_RtsNumIsValid(FirstRtsNum) && SC_RtsNumIsValid(LastRtsNum) &&
           SC_RtsNumWithinRange(FirstRtsNum, LastRtsNum);
}

/**
 * @brief Advance the entry offset by the given amount
 *
 * Entries in ATS/RTS buffers are always aligned to 32-bit words.  This advances the
 * offset by the specified number of bytes.  The given number of bytes will be converted
 * to words, rounding up as needed to get to the next word boundary.
 *
 * @param Pos   Starting entry offset
 * @param Bytes Amount to advance, in bytes
 * @returns Updated entry offset
 */
static inline SC_EntryOffset_t SC_EntryOffsetAdvance(SC_EntryOffset_t Pos, size_t Bytes)
{
    return (SC_EntryOffset_t) {SC_IDX_AS_UINT(Pos) + ((Bytes + sizeof(uint32) - 1) / sizeof(uint32))};
}

#endif
