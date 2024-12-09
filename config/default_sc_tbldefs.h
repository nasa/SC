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
 *   Specification for the CFS Stored Command (SC) table related
 *   constant and structure definitions.
 *
 */
#ifndef SC_TBLDEFS_H
#define SC_TBLDEFS_H

#include "common_types.h"
#include "sc_extern_typedefs.h"
#include "cfe_msg_hdr.h"

/*************************************************************************
 * Macro Definitions
 *************************************************************************/
#define SC_ATS_HEADER_SIZE (sizeof(SC_AtsEntryHeader_t)) /**< \brief ATS header size in bytes */
#define SC_RTS_HEADER_SIZE (sizeof(SC_RtsEntryHeader_t)) /**< \brief RTS header size in bytes */

/**
 * \defgroup cfscstblids ID definitions for cFE Table Services manage table request command
 * \{
 */
#define SC_TBL_ID_ATS_0     (1)                                  /**< \brief ATS 0 Table ID */
#define SC_TBL_ID_APPEND    (SC_TBL_ID_ATS_0 + SC_NUMBER_OF_ATS) /**< \brief Append Table ID */
#define SC_TBL_ID_RTS_0     (SC_TBL_ID_APPEND + 1)               /**< \brief RTS 0 Table ID */
#define SC_TBL_ID_RTS_INFO  (SC_TBL_ID_RTS_0 + SC_NUMBER_OF_RTS) /**< \brief RTS Info Table ID */
#define SC_TBL_ID_RTP_CTRL  (SC_TBL_ID_RTS_INFO + 1)             /**< \brief RTS Control Table ID */
#define SC_TBL_ID_ATS_INFO  (SC_TBL_ID_RTP_CTRL + 1)             /**< \brief ATS Info Table ID */
#define SC_TBL_ID_ATP_CTRL  (SC_TBL_ID_ATS_INFO + 1)             /**< \brief ATS Control Table ID */
#define SC_TBL_ID_ATS_CMD_0 (SC_TBL_ID_ATP_CTRL + 1)             /**< \brief ATS 0 Command Table ID */
/**\}*/

/************************************************************************
 * Type Definitions
 ************************************************************************/

/**
 *  \brief Absolute Value time tag for ATC's
 */
typedef uint32 SC_AbsTimeTag_t;

/**
 *  \brief Relative wakeup count
 */
typedef uint32 SC_RelWakeupCount_t;

/**
 *  \brief ATS Table Entry Header Type
 */
typedef struct
{
    uint16 Pad; /**< \brief Structure padding */

    SC_CommandNum_t CmdNumber; /**< \brief command number, range = 1 to SC_MAX_ATS_CMDS */

    uint16 TimeTag_MS; /**< \brief Time tag most significant 16 bits */
    uint16 TimeTag_LS; /**< \brief Time tag least significant 16 bits */

    /*
     * Note: the command packet data is variable length,
     *       the command packet header (not shown here),
     *       comes directly after the time tag
     */
} SC_AtsEntryHeader_t;

/**
 * \brief ATS header and message header
 */
typedef struct
{
    SC_AtsEntryHeader_t     Header; /**< \brief ATS header */
    CFE_MSG_CommandHeader_t Msg;    /**< \brief Command Message to be sent */
} SC_AtsEntry_t;

/**
 * \brief RTS Command Header Type
 */
typedef struct
{
    SC_RelWakeupCount_t WakeupCount; /**< \brief Relative wakeup count */

    /*
     * Note: the command packet data is variable length,
     *       the command packet header (not shown here),
     *       comes directly after WakeupCount.
     */
} SC_RtsEntryHeader_t;

/**
 * \brief RTS header and message header
 */
typedef struct
{
    SC_RtsEntryHeader_t     Header; /**< \brief RTS header */
    CFE_MSG_CommandHeader_t Msg;    /**< \brief Command Message to be sent */
} SC_RtsEntry_t;

#endif
