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
 *   CFS Stored Command (SC) Application Public Definitions
 *
 * This provides default values for configurable items that affect
 * the interface(s) of this module.  This includes the CMD/TLM message
 * interface, tables definitions, and any other data products that
 * serve to exchange information with other entities.
 *
 * @note This file may be overridden/superceded by mission-provided defintions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef SC_INTERFACE_CFG_H
#define SC_INTERFACE_CFG_H

#include "sc_interface_cfg_values.h"

/**
 * \brief Minimum Packet Size
 *
 *  \par Description:
 *       This parameter specifies the minumum size in bytes for an ATS or RTS command.
 *  \par Limits:
 *       This parameter must be greater than or equal to CFE_SB_CMD_HDR_SIZE and
 *       less than or equal to CFE_MISSION_SB_MAX_SB_MSG_SIZE.
 */
#define SC_PACKET_MIN_SIZE                   SC_INTERFACE_CFGVAL(PACKET_MIN_SIZE)
#define DEFAULT_SC_INTERFACE_PACKET_MIN_SIZE 8

/**
 * \brief Maximum Packet Size
 *
 *  \par Description:
 *       This parameter specifies the maximum size in bytes for an ATS or RTS command.
 *  \par Limits:
 *       This parameter must be greater than or equal to SC_PACKET_MIN_SIZE and
 *       less than or equal to CFE_MISSION_SB_MAX_SB_MSG_SIZE.
 */
#define SC_PACKET_MAX_SIZE                   SC_INTERFACE_CFGVAL(PACKET_MAX_SIZE)
#define DEFAULT_SC_INTERFACE_PACKET_MAX_SIZE 256

#define SC_NUMBER_OF_ATS                   SC_INTERFACE_CFGVAL(NUMBER_OF_ATS)
#define DEFAULT_SC_INTERFACE_NUMBER_OF_ATS 2 /**< \brief the number of Absolute Time Sequences */

/**
 * \brief  Number of RTS's
 *
 *  \par Description:
 *       The number of RTS's allowed in the system
 *
 *  \par Limits:
 *       This parameter can't be larger than 999.This parameter will dicate the size of
 *       The RTS Info Table.
 */
#define SC_NUMBER_OF_RTS                   SC_INTERFACE_CFGVAL(NUMBER_OF_RTS)
#define DEFAULT_SC_INTERFACE_NUMBER_OF_RTS 4

/**
 * \brief Max number of commands in each ATS
 *
 *  \par Description:
 *       The maximum number of commands that are allowed in each ATS
 *
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 16 bit
 *       integer (65535).
 */
#define SC_MAX_ATS_CMDS                   SC_INTERFACE_CFGVAL(MAX_ATS_CMDS)
#define DEFAULT_SC_INTERFACE_MAX_ATS_CMDS 1000

/**
 * \brief Max buffer size for an ATS in uint16s
 *
 *  \par Description:
 *       The max sizeof an ATS buffer in words (not bytes)
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 16 bit
 *       integer (65535).
 */
#define SC_ATS_BUFF_SIZE                   SC_INTERFACE_CFGVAL(ATS_BUFF_SIZE)
#define DEFAULT_SC_INTERFACE_ATS_BUFF_SIZE 8000

/**
 * \brief Max buffer size for an Append ATS in uint16s
 *
 *  \par Description:
 *       The max sizeof an Append ATS buffer in words (not bytes)
 *  \par Limits:
 *       This parameter cannot be larger than SC_ATS_BUFF_SIZE.
 */
#define SC_APPEND_BUFF_SIZE                   SC_INTERFACE_CFGVAL(APPEND_BUFF_SIZE)
#define DEFAULT_SC_INTERFACE_APPEND_BUFF_SIZE 4000 /* Half of ATS_BUFF_SIZE */

/**
 * \brief Max buffer size for an RTS in uint16s
 *
 *  \par Description:
 *       The max size of an RTS buffer in WORDS (not bytes)
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 16 bit
 *       integer (65535).
 */
#define SC_RTS_BUFF_SIZE                   SC_INTERFACE_CFGVAL(RTS_BUFF_SIZE)
#define DEFAULT_SC_INTERFACE_RTS_BUFF_SIZE 150

#endif
