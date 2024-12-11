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
 *   Contains CFS Stored Command macros that run preprocessor checks
 *   on mission configurable parameters
 */
#ifndef SC_VERIFY_H
#define SC_VERIFY_H

/*************************************************************************
 * Includes
 *************************************************************************/

#include "cfe.h"
#include "sc_platform_cfg.h"

/*************************************************************************
 * Macro Definitions
 *************************************************************************/

#ifndef SC_MAX_CMDS_PER_WAKEUP
#error SC_MAX_CMDS_PER_WAKEUP must be defined!
#elif (SC_MAX_CMDS_PER_WAKEUP > 65535)
#error SC_MAX_CMDS_PER_WAKEUP cannot be greater than 65535!
#elif (SC_MAX_CMDS_PER_WAKEUP < 1)
#error SC_MAX_CMDS_PER_WAKEUP cannot be less than 1!
#endif

#ifndef SC_NUMBER_OF_RTS
#error SC_NUMBER_OF_RTS must be defined!
#elif (SC_NUMBER_OF_RTS > CFE_PLATFORM_TBL_MAX_NUM_TABLES)
#error SC_NUMBER_OF_RTS cannot be greater than CFE_PLATFORM_TBL_MAX_NUM_TABLES!
#elif (SC_NUMBER_OF_RTS > 65535)
#error SC_NUMBER_OF_RTS cannot be greater than 65535
#elif (SC_NUMBER_OF_RTS < 1)
#error SC_NUMBER_OF_RTS cannot be less than 1!
#endif

/*
 *  SC also has the following "dump only" tables..
 *
 *    RTS info table
 *    RTS control block table
 *    ATS info table
 *    ATS control block table
 *    ATS command status table
 *
 *  CFE_TBL_MAX_NUM_TABLES limits the sum of all tables from all apps.
 */
#if ((SC_NUMBER_OF_RTS + SC_NUMBER_OF_ATS + 5) > CFE_PLATFORM_TBL_MAX_NUM_TABLES)
#error Sum of all SC tables cannot be greater than CFE_PLATFORM_TBL_MAX_NUM_TABLES!
#endif

#ifndef SC_ATS_BUFF_SIZE
#error SC_ATS_BUFF_SIZE must be defined!
#elif (SC_ATS_BUFF_SIZE > 65535)
#error SC_ATS_BUFF_SIZE cannot be greater than 65535!
#elif (SC_ATS_BUFF_SIZE < SC_PACKET_MIN_SIZE)
#error SC_ATS_BUFF_SIZE must be at least big enough to hold one command (SC_PACKET_MAX_SIZE)!
/* buf size = words, tbl size = bytes */
#elif ((SC_ATS_BUFF_SIZE * 2) > CFE_PLATFORM_TBL_MAX_DBL_TABLE_SIZE)
#error SC_ATS_BUFF_SIZE cannot be greater than CFE_PLATFORM_TBL_MAX_DBL_TABLE_SIZE!
#endif

#ifndef SC_APPEND_BUFF_SIZE
#error SC_APPEND_BUFF_SIZE must be defined!
#elif (SC_APPEND_BUFF_SIZE > SC_ATS_BUFF_SIZE)
#error SC_APPEND_BUFF_SIZE cannot be greater than SC_ATS_BUFF_SIZE!
#endif

#ifndef SC_RTS_BUFF_SIZE
#error SC_RTS_BUFF_SIZE must be defined!
#elif (SC_RTS_BUFF_SIZE > 65535)
#error SC_RTS_BUFF_SIZE cannot be greater than 65535!
#elif (SC_RTS_BUFF_SIZE < SC_PACKET_MIN_SIZE)
#error SC_RTS_BUFF_SIZE must be at least big enough to hold one command (SC_PACKET_MIN_SIZE)!
/* buf size = words, tbl size = bytes */
#elif ((SC_RTS_BUFF_SIZE * 2) > CFE_PLATFORM_TBL_MAX_SNGL_TABLE_SIZE)
#error SC_RTS_BUFF_SIZE cannot be greater than CFE_PLATFORM_TBL_MAX_SNGL_TABLE_SIZE!
#endif

#ifndef SC_MAX_ATS_CMDS
#error SC_MAX_ATS_CMDS must be defined!
#elif (SC_MAX_ATS_CMDS > 65535)
#error SC_MAX_ATS_CMDS cannot be greater than 65535!
#elif (SC_MAX_ATS_CMDS < 1)
#error SC_MAX_ATS_CMDS cannot be less than 1!
#endif

#ifndef SC_LAST_RTS_WITH_EVENTS
#error SC_LAST_RTS_WITH_EVENTS must be defined!
#elif (SC_LAST_RTS_WITH_EVENTS > SC_NUMBER_OF_RTS)
#error SC_LAST_RTS_WITH_EVENTS cannot be greater than SC_NUMBER_OF_RTS!
#elif (SC_LAST_RTS_WITH_EVENTS < 1)
#error SC_LAST_RTS_WITH_EVENTS cannot be less than 1!
#endif

#ifndef SC_PACKET_MIN_SIZE
#error SC_PACKET_MIN_SIZE must be defined!
#elif (SC_PACKET_MIN_SIZE > CFE_MISSION_SB_MAX_SB_MSG_SIZE)
#error SC_PACKET_MIN_SIZE cannot be greater than CFE_MISSION_SB_MAX_SB_MSG_SIZE!
#elif (SC_PACKET_MIN_SIZE < 8)
#error SC_PACKET_MIN_SIZE cannot be less than CFE_SB_CMD_HDR_SIZE!
#elif ((SC_PACKET_MIN_SIZE % 4) != 0)
#error SC_PACKET_MIN_SIZE is not 32-bit aligned!
#endif

#ifndef SC_PACKET_MAX_SIZE
#error SC_PACKET_MAX_SIZE must be defined!
#elif (SC_PACKET_MAX_SIZE > CFE_MISSION_SB_MAX_SB_MSG_SIZE)
#error SC_PACKET_MAX_SIZE cannot be greater than CFE_MISSION_SB_MAX_SB_MSG_SIZE!
#elif (SC_PACKET_MAX_SIZE < SC_PACKET_MIN_SIZE)
#error SC_PACKET_MAX_SIZE cannot be less than SC_PACKET_MIN_SIZE!
#elif ((SC_PACKET_MAX_SIZE % 4) != 0)
#error SC_PACKET_MAX_SIZE is not 32-bit aligned!
#endif

#ifndef SC_PIPE_DEPTH
#error SC_PIPE_DEPTH must be defined!
#elif (SC_PIPE_DEPTH < 1)
#error SC_PIPE_DEPTH cannot be less than 1!
#endif

#ifndef SC_ATS_TABLE_NAME
#error SC_ATS_TABLE_NAME must be defined!
#endif

#ifndef SC_APPEND_TABLE_NAME
#error SC_APPEND_TABLE_NAME must be defined!
#endif

#ifndef SC_RTS_TABLE_NAME
#error SC_RTS_TABLE_NAME must be defined!
#endif

#ifndef SC_ATS_FILE_NAME
#error SC_ATS_FILE_NAME must be defined!
#endif

#ifndef SC_APPEND_FILE_NAME
#error SC_APPEND_FILE_NAME must be defined!
#endif

#ifndef SC_RTS_FILE_NAME
#error SC_RTS_FILE_NAME must be defined!
#endif

#ifndef SC_RTSINFO_TABLE_NAME
#error SC_RTSINFO_TABLE_NAME must be defined!
#endif

#ifndef SC_RTP_CTRL_TABLE_NAME
#error SC_RTP_CTRL_TABLE_NAME must be defined!
#endif

#ifndef SC_ATSINFO_TABLE_NAME
#error SC_ATSINFO_TABLE_NAME must be defined!
#endif

#ifndef SC_APPENDINFO_TABLE_NAME
#error SC_APPENDINFO_TABLE_NAME must be defined!
#endif

#ifndef SC_ATS_CTRL_TABLE_NAME
#error SC_ATS_CTRL_TABLE_NAME must be defined!
#endif

#ifndef SC_ATS_CMD_STAT_TABLE_NAME
#error SC_ATS_CMD_STAT_TABLE_NAME must be defined!
#endif

#ifndef SC_CONT_ON_FAILURE_START
#error SC_CONT_ON_FAILURE_START must be defined!
#elif ((SC_CONT_ON_FAILURE_START != SC_AtsCont_TRUE) && (SC_CONT_ON_FAILURE_START != SC_AtsCont_FALSE))
#error SC_CONT_ON_FAILURE_START must be either SC_AtsCont_TRUE or SC_AtsCont_FALSE!
#endif

#ifndef SC_TIME_TO_USE
#error SC_TIME_TO_USE must be defined!
#elif (SC_TIME_TO_USE != SC_TimeRef_USE_CFE_TIME)
#if (SC_TIME_TO_USE != SC_TimeRef_USE_TAI)
#if (SC_TIME_TO_USE != SC_TimeRef_USE_UTC)
#error SC_TIME_TO_USE must be either SC_TimeRef_USE_CFE_TIME, SC_TimeRef_USE_TAI or SC_TimeRef_USE_UTC!
#endif
#endif
#endif

#ifndef SC_MISSION_REV
#error SC_MISSION_REV must be defined!
#elif (SC_MISSION_REV < 0)
#error SC_MISSION_REV must be greater than or equal to zero!
#endif

#endif
