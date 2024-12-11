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
 *   CFS Stored Command (SC) Application Private Config Definitions
 *
 * This provides default values for configurable items that are internal
 * to this module and do NOT affect the interface(s) of this module.  Changes
 * to items in this file only affect the local module and will be transparent
 * to external entities that are using the public interface(s).
 *
 * @note This file may be overridden/superceded by mission-provided defintions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef SC_INTERNAL_CFG_H
#define SC_INTERNAL_CFG_H

/**
 * \defgroup cfsscplatformcfg CFS Stored Command Platform Configuration
 * \{
 */

/**
 * \brief Whether to update headers on commands when sending on software bus
 *
 *  \par Description:
 *
 *       This controls whether to update the command header on the outgoing messages
 *       produced by the SC app.
 *
 *        - If the messages in the ATS/RTS tables are expected to be completely
 *          initialized, including the sequence number and any required checksum
 *          field, then this should be configured as "false" such that these fields
 *          will not be modified during message transmission.  The UpdateHeader
 *          parameter to CFE_SB_TransmitMsg() will be passed as false, and the message
 *          will be broadcast on the software bus verbatim.  This replicates historical
 *          behavior of SC.
 *
 *        - If the messages in the ATS/RTS tables need a real-time header update as
 *          part of the sending process, this should be configured as "true".  Any
 *          checksum embedded within the ATS/RTS will be ignored.  The UpdateHeader
 *          parameter to CFE_SB_TransmitMsg() will be passed as true, such that the
 *          command will be sequenced and timestamped with current values.  A correct
 *          checksum will be computed based on the updated header values, and the
 *          resulting message is broadcast on the software bus.
 *
 * \sa
 *      CFE_SB_TransmitMsg() - UpdateHeader parameter
 *
 *  \par Limits:
 *       Must be true or false
 */
#define SC_PLATFORM_ENABLE_HEADER_UPDATE false

/**
 * \brief  Max number of commands per wakeup
 *
 *  \par Description:
 *       Maximum number of commands that can be sent out by SC
 *       in any given wakeup cycle.
 *
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 16 bit
 *       integer (65535), but should be kept relatively small to
 *       avoid SC hogging the CPU
 *
 */
#define SC_MAX_CMDS_PER_WAKEUP 8

/**
 * \brief Max buffer size for an ATS in uint16s
 *
 *  \par Description:
 *       The max sizeof an ATS buffer in words (not bytes)
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 16 bit
 *       integer (65535).
 */
#define SC_ATS_BUFF_SIZE 8000

/**
 * \brief Max buffer size for an Append ATS in uint16s
 *
 *  \par Description:
 *       The max sizeof an Append ATS buffer in words (not bytes)
 *  \par Limits:
 *       This parameter cannot be larger than SC_ATS_BUFF_SIZE.
 */
#define SC_APPEND_BUFF_SIZE (SC_ATS_BUFF_SIZE / 2)

/**
 * \brief Max buffer size for an RTS in uint16s
 *
 *  \par Description:
 *       The max size of an RTS buffer in WORDS (not bytes)
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 16 bit
 *       integer (65535).
 */
#define SC_RTS_BUFF_SIZE 150

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
#define SC_MAX_ATS_CMDS 1000

/**
 * \brief The last RTS that will be sent with an #SC_RTS_START_INF_EID event message
 *
 *  \par Description:
 *       When all RTS's are started, the #SC_RTS_START_INF_EID event message is sent out.
 *       This parameter suppreses that message for all RTS's over this number
 *
 *  \par Limits:
 *       This parameter needs to be less than or equal to SC_NUMBER_OF_RTS
 */
#define SC_LAST_RTS_WITH_EVENTS 20

/**
 * \brief Command Pipe Depth
 *
 *  \par Description:
 *       Maximum number of messages that will be allowed in the
 *       SC command pipe at one time. Used during initialization
 *       in the call to #CFE_SB_CreatePipe
 *
 *  \par Limits:
 *       This parameter must be greater than zero and less than or equal to
 *       OS_QUEUE_MAX_DEPTH.
 */
#define SC_PIPE_DEPTH 12

/**
 * \brief ATS Table Filenames
 *
 *  \par Description:
 *       Filenames for the ATS tables loaded at startup are constructed by
 *       appending a one digit table identifier plus the extension ".tbl"
 *       to the base portion of the filename defined here.
 *
 *       The default definitions will create ATS filenames as follows:
 *       "/cf/apps/sc_ats1.tbl or /cf/apps/sc_ats2.tbl"
 *
 *  \par Limits:
 *       SC requires that this name be defined, but otherwise places no
 *       limits on the definition.  If the named table file does not exist
 *       at run time, or the table fails validation, then the table load
 *       will fail.  Refer to #OS_MAX_PATH_LEN for filename length limits.
 */
#define SC_ATS_FILE_NAME "/cf/sc_ats"

/**
 * \brief Append ATS Table Filename
 *
 *  \par Description:
 *       This name describes the default append ATS filename loaded at
 *       startup.  Often the default Append ATS file contains only a
 *       single unused table entry, and is used only to initialize the
 *       table state as having data that may be patched.
 *
 *  \par Limits:
 *       SC requires that this name be defined, but otherwise places no
 *       limits on the definition.  If the named table file does not exist
 *       at run time, or the table fails validation, then the table load
 *       will fail.  Refer to #OS_MAX_PATH_LEN for filename length limits.
 */
#define SC_APPEND_FILE_NAME "/cf/sc_append.tbl"

/**
 * \brief RTS Table Filenames
 *
 *  \par Description:
 *       Filenames for the RTS tables loaded at startup are constructed by
 *       appending a one digit table identifier plus the extension ".tbl"
 *       to the base portion of the filename defined here.
 *
 *       The default definitions will create RTS filenames as follows:
 *       "/cf/apps/sc_rts001.tbl, /cf/apps/sc_rts002.tbl, etc"
 *
 *  \par Limits:
 *       SC requires that this name be defined, but otherwise places no
 *       limits on the definition.  If the named table file does not exist
 *       at run time, or the table fails validation, then the table load
 *       will fail.  Refer to #OS_MAX_PATH_LEN for filename length limits.
 */
#define SC_RTS_FILE_NAME "/cf/sc_rts"

/**
 * \brief ATS Table Object Names
 *
 *  \par Description:
 *       Unique table object names are required for each table registered
 *       with cFE Table Services.  ATS table object names are constructed
 *       by appending a one digit table identifier to the base portion of
 *       the object name defined here.
 *
 *       The default definitions will create ATS object names as follows:
 *       "ATS_TBL1 or ATS_TBL2"
 *
 *  \par Limits:
 *       SC requires that this name be defined, but otherwise places no
 *       limits on the definition.  Refer to #CFE_MISSION_TBL_MAX_NAME_LENGTH for
 *       specific information on limits related to table object names.
 */
#define SC_ATS_TABLE_NAME "ATS_TBL"

/**
 * \brief Append ATS Table Object Names
 *
 *  \par Description:
 *       Unique table object names are required for each table registered
 *       with cFE Table Services.  This is the table object name for the
 *       Append ATS table.
 *
 *  \par Limits:
 *       SC requires that this name be defined, but otherwise places no
 *       limits on the definition.  Refer to #CFE_MISSION_TBL_MAX_NAME_LENGTH for
 *       specific information on limits related to table object names.
 */
#define SC_APPEND_TABLE_NAME "APPEND_TBL"

/**
 * \brief RTS Table Object Names
 *
 *  \par Description:
 *       Unique table object names are required for each table registered
 *       with cFE Table Services.  RTS table object names are constructed
 *       by appending a three digit table identifier to the base portion
 *       of the object name defined here.
 *
 *       The default definitions will create RTS object names as follows:
 *       "RTS_TBL001, RTS_TBL002, etc"
 *
 *  \par Limits:
 *       SC requires that this name be defined, but otherwise places no
 *       limits on the definition.  Refer to #CFE_MISSION_TBL_MAX_NAME_LENGTH for
 *       specific information on limits related to table object names.
 */
#define SC_RTS_TABLE_NAME "RTS_TBL"

/**
 * \brief Name of the RTS Infomation Table
 *
 *  \par Description:
 *       Name of the RTS Information Table
 *  \par Limits:
 *       Must be less than #CFE_MISSION_TBL_MAX_NAME_LENGTH
 */
#define SC_RTSINFO_TABLE_NAME "RTSINF_TBL"

/**
 * \brief Name of the RTP Control block table
 *
 *  \par Description:
 *       Name of the RTP Control Block Table
 *  \par Limits:
 *       Must be less than #CFE_MISSION_TBL_MAX_NAME_LENGTH
 */
#define SC_RTP_CTRL_TABLE_NAME "RTPCTR_TBL"

/**
 * \brief Name of the ATS Infomation Table
 *
 *  \par Description:
 *       Name of the ATS Information Table
 *  \par Limits:
 *       Must be less than #CFE_MISSION_TBL_MAX_NAME_LENGTH
 */
#define SC_ATSINFO_TABLE_NAME "ATSINF_TBL"

/**
 * \brief Name of the Append ATS Infomation Table
 *
 *  \par Description:
 *       Name of the Append ATS Information Table
 *  \par Limits:
 *       Must be less than #CFE_MISSION_TBL_MAX_NAME_LENGTH
 */
#define SC_APPENDINFO_TABLE_NAME "APPINF_TBL"

/**
 * \brief Name of the ATP Control block table
 *
 *  \par Description:
 *       Name of the ATP Control Block Table
 *  \par Limits:
 *       Must be less than #CFE_MISSION_TBL_MAX_NAME_LENGTH
 */
#define SC_ATS_CTRL_TABLE_NAME "ATPCTR_TBL"

/**
 * \brief Name Prefix of ATS Cmd Status Table
 *
 *  \par Description:
 *       The prefix of the ATS Command Status table names. Note that actual table names
 *       will have a 1 digit number postfixed to it depending on the
 *       ATS number
 *
 *  \par Limits:
 *       Total length must be less than #CFE_MISSION_TBL_MAX_NAME_LENGTH
 */
#define SC_ATS_CMD_STAT_TABLE_NAME "ATSCMD_TBL"

/**
 * \brief Defines default state of Continue-Ats-On-Checksum-Failure Flag
 *
 *  \par Description:
 *       This parameter specifies the default state to continue an ATS
 *       when a command in the ATS fails checksum validation
 *
 *  \par Limits:
 *       Must be SC_AtsCont_TRUE or SC_AtsCont_FALSE
 */
#define SC_CONT_ON_FAILURE_START SC_AtsCont_TRUE

/**
 * \brief Defines the TIME SC should use for its commands
 *
 *  \par Description:
 *       This parameter defines what type of time SC should use for sending out its commands
 *
 *  \par Limits:
 *       Must be SC_TimeRef_USE_CFE_TIME, SC_TimeRef_USE_TAI, or SC_TimeRef_USE_UTC */
#define SC_TIME_TO_USE SC_TimeRef_USE_CFE_TIME

/**
 * \brief Autostart RTS ID after power on
 *
 * \par Description:
 *      The specified RTS will be automatically invoked after a power-on
 *      May be configured as 0 to disable
 *
 * \par Limits:
 *       Must be a valid RTS ID or 0
 */
#define RTS_ID_AUTO_POWER_ON 1

/**
 * \brief Autostart RTS ID after processor reset
 *
 * \par Description:
 *      The specified RTS will be automatically invoked after a processor reset
 *      May be configured as 0 to disable
 *
 * \par Limits:
 *       Must be a valid RTS ID or 0
 */
#define RTS_ID_AUTO_PROCESSOR 2

/**
 * \brief Mission specific version number for SC application
 *
 *  \par Description:
 *       An application version number consists of four parts:
 *       major version number, minor version number, revision
 *       number and mission specific revision number. The mission
 *       specific revision number is defined here and the other
 *       parts are defined in "sc_version.h".
 *
 *  \par Limits:
 *       Must be defined as a numeric value that is greater than
 *       or equal to zero.
 */
#define SC_MISSION_REV 0

/**\}*/

#endif
