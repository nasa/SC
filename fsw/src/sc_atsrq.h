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
 *   This file contains header for the functions to handle all of the ATS
 *   executive requests and internal reuqests to control
 *   the ATP and ATSs.
 */
#ifndef SC_ATSRQ_H
#define SC_ATSRQ_H

#include "cfe.h"
#include "sc_index_types.h"
#include "sc_msg.h"

/**
 * \brief Starts an ATS
 *
 *  \par Description
 *         This function starts an ATS by finding the first ATS command.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param[in] AtsIndex   The Ats to begin
 *  \param[in] TimeOffset Where to start in the ATS
 *
 *  \return Boolean execution status
 *  \retval true  ATS was started
 *  \retval false ATS was NOT started
 */
bool SC_BeginAts(SC_AtsIndex_t AtsIndex, uint16 TimeOffset);

/**
 * \brief  Start an ATS Command
 *
 *  \par Description
 *         This function starts an ATS on the ATP. This routine does
 *         not actually execute any commands, it simply sets up all
 *         of the data structures to indicate that the specified ATS
 *         is now running. This function also does all of the parameter
 *         checking.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param[in] Cmd Pointer to Software Bus buffer
 *
 *  \sa #SC_START_ATS_CC
 */
void SC_StartAtsCmd(const SC_StartAtsCmd_t *Cmd);

/**
 * \brief Stop the executing ATS Command
 *
 *  \par Description
 *            This routine stops an ATS from executing on the ATP.
 *            This routine will execute even if an ATS is not currently
 *            executing in the buffer.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param[in] Cmd Pointer to Software Bus buffer
 *
 *  \sa #SC_STOP_ATS_CC
 */
void SC_StopAtsCmd(const SC_StopAtsCmd_t *Cmd);

/**
 * \brief Stops an ATS & clears out data
 *
 *  \par Description
 *         This is a generic routine that is used to clear out the
 *            ATP information to stop an ATS.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 */
void SC_KillAts(void);

/**
 * \brief  Switch the ATS Command
 *
 *  \par Description
 *         This function initiates an ATS switch. An ATS switch cannot be
 *         immediatly started when the command is received because of the
 *         risk of sending out duplicate commands in the new buffer.
 *         (if buffer A has executed 3 of the 5 commands for second N, and
 *         the switch command is recvd at second N, the switch would
 *         happen in the same second, causing buffer B to execute all
 *         5 commands in second N , assuming that the buffers had
 *         an overlap of duplicate commands.)
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param[in] Cmd Pointer to Software Bus buffer
 *
 *  \sa #SC_SWITCH_ATS_CC
 */
void SC_SwitchAtsCmd(const SC_SwitchAtsCmd_t *Cmd);

/**
 * \brief
 *
 *  \par Description
 *         This routine is called when the ATS IN-LINE request SWITCH
 *         ATS is encountered. This routine stops the current ATS from
 *         executing and starts the 'other' one. It is assumed that there
 *         is an ATS running because this command is only valid as an
 *         IN-LINE ATS request.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \return Boolean execution status
 *  \retval true   Switch was successful
 *  \retval false  Switch was NOT successful
 */
bool SC_InlineSwitch(void);

/**
 * \brief Switches ATS's at a safe time
 *
 *  \par Description
 *       This function does the ATS switch when it is determined that
 *       the switch is "safe". When the switch request was made, the
 *       switch pend flag was set. After every scheduling of the SCP,
 *       the switch pend flag is checked. If the switch pend flag is
 *       set, this routine is called. This routine checks to see that
 *       the current time is one second past the time to start the
 *       new ATS. If it is the correct time, then the switch is performed.
 *       All of this has the effect of creating a syncronized switch of
 *       the ATS buffers, assuring that no duplicate commands are sent.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 */
void SC_ServiceSwitchPend(void);

/**
 * \brief Jump time in an ATS Command
 *
 *  \par Description
 *         This command is used to jump to a specified time in the
 *         currently running ATS. The jump command will effectively
 *         restart the ATS at the time given in the command. Because
 *         there is no restriction on the time given in the command,
 *         the ATP may try to restart the ATS at any time before or
 *         after the current time. In the case of the time tag being
 *         before the current time, ( a backwards jump ) the ATP will
 *         simply skip the commands that have been executed ( or failed
 *         execution ) and end up at the same location as before. In the
 *         case of the jump time being after the current time, the ATP
 *         will skip all commands with time tags less than the jump time
 *         and start executing the ATS at the time equal to the jump
 *         time. If there are no commands with time tags equal to the
 *         jump time, the ATP will set up the ATS to wait for the first
 *         command after the jump time. When a command is skipped while
 *         doing the jump, the command's status is marked as SKIPPED unless
 *         it has already been marked as EXECUTED, FAILED_DISTRIBUTION,
 *         or FAILED_CHECKSUM.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param[in] Cmd Pointer to Software Bus buffer
 *
 *  \sa #SC_JUMP_ATS_CC
 */
void SC_JumpAtsCmd(const SC_JumpAtsCmd_t *Cmd);

/**
 * \brief Lets an ATS continue if a command failed the checksum
 *
 *  \par Description
 *         This routine sets whether or not to let an ATS continue when
 *         one of the commands in the ATS fails a checksum validation
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param[in] Cmd Pointer to Software Bus buffer
 *
 *  \sa #SC_CONTINUE_ATS_ON_FAILURE_CC
 */
void SC_ContinueAtsOnFailureCmd(const SC_ContinueAtsOnFailureCmd_t *Cmd);

/**
 * \brief  Append to an ATS Command
 *
 *  \par Description
 *         This function adds the contents of the Append ATS table to
 *         the selected ATS.  The ATS is then re-sorted for command
 *         execution order.  This command may target an ATS that is
 *         currently active (executing).  This command will not change
 *         the ATS execution state.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param[in] Cmd Pointer to Software Bus buffer
 *
 *  \sa #SC_APPEND_ATS_CC
 */
void SC_AppendAtsCmd(const SC_AppendAtsCmd_t *Cmd);

#endif
