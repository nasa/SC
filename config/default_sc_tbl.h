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
 *   Specification for the CFS Stored Command (SC) table structures
 *
 * @note
 *   Constants and enumerated types related to these table structures
 *   are defined in sc_tbldefs.h.
 */
#ifndef DEFAULT_SC_TBL_H
#define DEFAULT_SC_TBL_H

#include "sc_interface_cfg.h"
#include "sc_extern_typedefs.h"
#include "sc_tbldefs.h"
#include "sc_tblstruct.h"

/*************************************************************************
 * Macro Definitions
 *************************************************************************/
#define SC_ATS_HEADER_SIZE (offsetof(SC_AtsEntry_t, Msg)) /**< \brief ATS header size in bytes */
#define SC_RTS_HEADER_SIZE (offsetof(SC_RtsEntry_t, Msg)) /**< \brief RTS header size in bytes */

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

#endif
