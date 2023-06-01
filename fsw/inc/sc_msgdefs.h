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
 *   Specification for the CFS Stored Command (SC) command and telemetry
 *   message constant definitions.
 *
 * @note
 *   DO NOT PUT ANY TYPEDEFS OR STRUCTURE DEFINITIONS IN THIS FILE!
 *   ADD THEM TO sc_msg.h IF NEEDED!
 */
#ifndef SC_MSGDEFS_H
#define SC_MSGDEFS_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/**
 * \name Which processor runs next
 * \{
 */
#define SC_ATP  0    /**< \brief ATP process next */
#define SC_RTP  1    /**< \brief RTP process next */
#define SC_NONE 0xFF /**< \brief No pending process */
/**\}*/

#define SC_MAX_TIME 0xFFFFFFFF /**< \brief Maximum time in SC */

/**
 * \name ATS/RTS Cmd Status macros
 * \{
 */
#define SC_EMPTY           0 /**< \brief the object is not loaded */
#define SC_LOADED          1 /**< \brief the object is loaded */
#define SC_IDLE            2 /**< \brief the object is not executing */
#define SC_EXECUTED        3 /**< \brief the object has completed executing */
#define SC_SKIPPED         4 /**< \brief the object (ats command) was skipped */
#define SC_EXECUTING       5 /**< \brief the object is currently executing */
#define SC_FAILED_CHECKSUM 6 /**< \brief the object failed a checksum test */
#define SC_FAILED_DISTRIB  7 /**< \brief the object could not be sent on the SWB */
#define SC_STARTING        8 /**< \brief used when an inline switch is executed */
/**\}*/

/**
 * \name Defines for each ATS
 * \{
 */
#define SC_NO_ATS 0 /**<\ brief No ATS */
#define SC_ATSA   1 /**< \brief ATS A */
#define SC_ATSB   2 /**< \brief ATS B */
/**\}*/

/**
 * \name constants for config parameters for which TIME to use
 * \{
 */
#define SC_USE_CFE_TIME 0 /**< \brief Use cFE configured time */
#define SC_USE_TAI      1 /**< \brief Use TAI Time */
#define SC_USE_UTC      2 /**< \brief USE UTC Time */
/**\}*/

#define SC_INVALID_RTS_NUMBER 0 /**< \brief Invalid RTS number */

#define SC_NUMBER_OF_ATS 2 /**< \brief the number of Absolute Time Sequences */

/**
 * \name SC Continue Flags
 * \{
 */
#define SC_CONTINUE_TRUE  1 /**< \brief Continue on failure */
#define SC_CONTINUE_FALSE 0 /**< \brief Do not continue on failure */
/**\}*/

/**
 * \defgroup cfssccmdcodes CFS Stored Command Command Codes
 * \{
 */

/**
 * \brief Noop
 *
 *  \par Description
 *       Implements the Noop command that insures the SC app is alive
 *
 *  \par Command Structure
 *       #SC_NoArgsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - The #SC_NOOP_INF_EID informational event message will be
 *         generated when the command is received
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - Error specific event message #SC_LEN_ERR_EID
 *
 *  \par Criticality
 *       None
 *
 *  \sa #SC_RESET_COUNTERS_CC
 */
#define SC_NOOP_CC 0

/**
 * \brief Reset Counters
 *
 *  \par Description
 *       Resets the SC housekeeping counters
 *
 *  \par Command Structure
 *       #SC_NoArgsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will be cleared
 *       - The #SC_STARTATS_CMD_INF_EID informational event message will be
 *         generated when the command is received
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - Error specific event message #SC_LEN_ERR_EID
 *
 *  \par Criticality
 *       None
 *
 *  \sa #SC_NOOP_CC
 */
#define SC_RESET_COUNTERS_CC 1

/**
 * \brief Start an ATS
 *
 *  \par Description
 *       Starts the specified ATS
 *
 *  \par Command Structure
 *       #SC_StartAtsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - ATP is not idle
 *       - ATS specified is not loaded
 *       - Invalid ATS ID
 *       - All command were skipped
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - Error specific event message
 *
 *  \par Criticality
 *       None
 *
 *  \sa #SC_STOP_ATS_CC
 */
#define SC_START_ATS_CC 2
/**
 * \brief Stop an ATS
 *
 *  \par Description
 *       Stops the specified ATS
 *
 *  \par Command Structure
 *       #SC_NoArgsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - the #SC_STOPATS_CMD_INF_EID event message will be generated
 *
 **
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *
 *  \par Criticality
 *       None
 *
 *  \sa #SC_START_ATS_CC
 */
#define SC_STOP_ATS_CC 3

/**
 * \brief Start an RTS
 *
 *  \par Description
 *       Starts the specified RTS
 *
 *  \par Command Structure
 *       #SC_RtsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - The #SC_STARTRTS_CMD_DBG_EID will be sent
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - Invalid command field in first RTS command
 *       - RTS not loaded
 *       - RTS already running
 *       - RTS is disabled
 *       - Invalid RTS ID
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - #SC_HkTlm_t.RtsActiveErrCtr will increment
 *       - Error specific event message
 *
 *  \par Criticality
 *       None
 *
 *  \sa #SC_STOP_RTS_CC
 */
#define SC_START_RTS_CC 4

/**
 * \brief Stop an RTS
 *
 *  \par Description
 *       Stops the specified RTS
 *
 *  \par Command Structure
 *       #SC_RtsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - The #SC_STOPRTS_CMD_INF_EID will be sent
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - RTS ID is invalid
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - Error specific event message
 *
 *  \par Criticality
 *       None
 *
 *  \sa #SC_START_RTS_CC
 */
#define SC_STOP_RTS_CC 5

/**
 * \brief DISABLE an RTS
 *
 *  \par Description
 *       Disables the specified RTS
 *
 *  \par Command Structure
 *       #SC_RtsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - The #SC_DISABLE_RTS_DEB_EID will be sent
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - RTS ID is invalid
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - Error specific event message
 *
 *  \par Criticality
 *       None
 *
 *  \sa #SC_ENABLE_RTS_CC
 */
#define SC_DISABLE_RTS_CC 6

/**
 * \brief Enable an RTS
 *
 *  \par Description
 *       Enables the specified RTS
 *
 *  \par Command Structure
 *       #SC_RtsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - The #SC_ENABLE_RTS_DEB_EID will be sent
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - RTS ID is invalid
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - Error specific event message
 *
 *  \par Criticality
 *       None
 *
 *  \sa #SC_DISABLE_RTS_CC
 */
#define SC_ENABLE_RTS_CC 7

/**
 * \brief Switch the running ATS
 *
 *  \par Description
 *       Switches the running ATS and the ATS no running
 *
 *  \par Command Structure
 *       #SC_NoArgsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - The #SC_SWITCH_ATS_CMD_INF_EID will be sent
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - Desitination ATS is not loaded
 *       - There is no currently running ATS to switch from
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - Error specific event message
 *
 *  \par Criticality
 *       None
 */
#define SC_SWITCH_ATS_CC 8

/**
 * \brief Jump the time in the running ATS
 *
 *  \par Description
 *       Moves the 'current time' pointer in the ATS to another time
 *
 *  \par Command Structure
 *       #SC_JumpAtsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - The #SC_JUMP_ATS_INF_EID will be sent
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - All ATS Cmds were skipped in the jump, ATS is shut off
 *       - No ATS is active
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - Error specific event message
 *
 *  \par Criticality
 *       None
 */
#define SC_JUMP_ATS_CC 9

/**
 * \brief Set the Continue-On-Checksum-Failure flag
 *
 *  \par Description
 *       Sets the flag which specifies whether or not to continue
 *        processing an ATS if one of the commands in the ATS fails
 *        checksum validation before being sent out.
 *
 *  \par Command Structure
 *       #SC_SetContinueAtsOnFailureCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - The #SC_CONT_CMD_DEB_EID will be sent
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - Invalid State specified
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - Error specific event message
 *
 *  \par Criticality
 *       None
 */
#define SC_CONTINUE_ATS_ON_FAILURE_CC 10

/**
 * \brief Append to an ATS table
 *
 *  \par Description
 *       Adds contents of the Append table to the specified ATS table
 *
 *  \par Command Structure
 *       #SC_AppendAtsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - ATS specified is not loaded
 *       - Invalid ATS ID
 *       - Append table contents too large to fit in ATS free space
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - Error specific event message
 *
 *  \par Criticality
 *       None
 */
#define SC_APPEND_ATS_CC 11

/**
 * \brief Request from cFE Table Services to manage a table
 *
 *  \par Description
 *       This command signals a need for the host application (SC)
 *       to allow cFE Table Services to manage the specified table.
 *       For loadable tables, this command indicates that a table
 *       update is available.  For dump only tables, this command
 *       indicates that cFE Table Services wants to dump the table
 *       data.  In either case, the host application must call the
 *       table manage API function so that the pending function
 *       can be executed within the context of the host.
 *
 *       Note: There is no reason for this command to be sent from
 *       any source other than cFE Table Services.
 *
 *  \par Command Structure
 *       #CFE_TBL_NotifyCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified via:
 *       - cFE Table Services housekeeping telemetry
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Invalid table ID
 *       - Unexpected result during manage of loadable table
 *
 *  \par Evidence of failure for this command may be verified via:
 *       - cFE Table Services housekeeping telemetry
 *       - Error specific SC event message
 *
 *  \par Criticality
 *       None
 */
#define SC_MANAGE_TABLE_CC 12

#if (SC_ENABLE_GROUP_COMMANDS == true)
/**
 * \brief START a group of RTS
 *
 *  \par Description
 *       The load state for an RTS may be LOADED or NOT LOADED.
 *       The enable state for an RTS may be ENABLED or DISABLED.
 *       The run state for an RTS may be STARTED or STOPPED.
 *       This command STARTS each RTS in the specified group that is
 *       currently LOADED, ENABLED and STOPPED.
 *
 *  \par Command Structure
 *       #SC_RtsGrpCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - #SC_STARTRTSGRP_CMD_INF_EID event will indicate the number of RTS
 *         in the group that were actually STARTED by the command.
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - Invalid group definition, first RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *       - Invalid group definition, last RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *       - Invalid group definition, last RTS ID must be greater than or equal to first RTS ID
 *       - If the group definition is valid the command will report success, regardless of
 *         whether any RTS in the group is actually STARTED by the command.
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - The #SC_LEN_ERR_EID event will indicate invalid command packet length.
 *       - The #SC_STARTRTSGRP_CMD_ERR_EID event will indicate invalid group definition.
 *
 *  \par Criticality
 *       None
 *
 *  \sa #SC_STOP_RTS_GRP_CC
 */
#define SC_START_RTS_GRP_CC 13

/**
 * \brief STOP a group of RTS
 *
 *  \par Description
 *       The load state for an RTS may be LOADED or NOT LOADED.
 *       The enable state for an RTS may be ENABLED or DISABLED.
 *       The run state for an RTS may be STARTED or STOPPED.
 *       This command STOPS each RTS in the specified group that is currently STARTED.
 *
 *  \par Command Structure
 *       #SC_RtsGrpCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - The #SC_STOPRTSGRP_CMD_INF_EID event will indicate the number of RTS
 *         in the group that were actually STOPPED by the command
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - Invalid group definition, first RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *       - Invalid group definition, last RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *       - Invalid group definition, last RTS ID must be greater than or equal to first RTS ID
 *       - If the group definition is valid the command will report success, regardless of
 *         whether any RTS in the group is actually STOPPED by the command.
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - The #SC_LEN_ERR_EID event will indicate invalid command packet length.
 *       - The #SC_STOPRTSGRP_CMD_ERR_EID event will indicate invalid group definition.
 *
 *  \par Criticality
 *       None
 *
 *  \sa #SC_START_RTS_GRP_CC
 */
#define SC_STOP_RTS_GRP_CC 14

/**
 * \brief DISABLE a group of RTS
 *
 *  \par Description
 *       The enable state for an RTS may be ENABLED or DISABLED.
 *       This command sets the enable state for the specified group of RTS to DISABLED.
 *
 *  \par Command Structure
 *       #SC_RtsGrpCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - The #SC_DISRTSGRP_CMD_INF_EID event will indicate the number of RTS
 *         in the group that were changed from ENABLED to DISABLED by the command
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - Invalid group definition, first RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *       - Invalid group definition, last RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *       - Invalid group definition, last RTS ID must be greater than or equal to first RTS ID
 *       - If the group definition is valid the command will report success, regardless of
 *         whether the group contained an RTS that was not already DISABLED.
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - The #SC_LEN_ERR_EID event will indicate invalid command packet length.
 *       - The #SC_DISRTSGRP_CMD_ERR_EID event will indicate invalid group definition.
 *
 *  \par Criticality
 *       None
 *
 *  \sa #SC_ENABLE_RTS_GRP_CC
 */
#define SC_DISABLE_RTS_GRP_CC 15

/**
 * \brief ENABLE a group of RTS
 *
 *  \par Description
 *       The enable state for an RTS may be ENABLED or DISABLED.
 *       This command sets the enable state for the specified group of RTS to ENABLED.
 *
 *  \par Command Structure
 *       #SC_RtsGrpCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #SC_HkTlm_t.CmdCtr will increment
 *       - The #SC_ENARTSGRP_CMD_INF_EID event will indicate success and display the
 *         number of RTS that were changed from DISABLED to ENABLED by the command.
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - Invalid group definition, first RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *       - Invalid group definition, last RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *       - Invalid group definition, last RTS ID must be greater than or equal to first RTS ID
 *       - If the group definition is valid the command will report success, regardless of
 *         whether the group contained an RTS that was not already ENABLED.
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #SC_HkTlm_t.CmdErrCtr will increment
 *       - The #SC_LEN_ERR_EID event will indicate invalid command packet length.
 *       - The #SC_ENARTSGRP_CMD_ERR_EID event will indicate invalid group definition.
 *
 *  \par Criticality
 *       None
 *
 *  \sa #SC_DISABLE_RTS_GRP_CC
 */
#define SC_ENABLE_RTS_GRP_CC 16

#endif

/**\}*/

#ifndef SC_OMIT_DEPRECATED
#define SC_START_RTSGRP_CC   SC_START_RTS_GRP_CC
#define SC_STOP_RTSGRP_CC    SC_STOP_RTS_GRP_CC
#define SC_DISABLE_RTSGRP_CC SC_DISABLE_RTS_GRP_CC
#define SC_ENABLE_RTSGRP_CC  SC_ENABLE_RTS_GRP_CC
#endif

#endif
