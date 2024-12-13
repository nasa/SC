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
 *   This file contains functions to handle getting the next time
 *   or wakeup count of commands for the ATP and RTP, as well as
 *   updating the time for Stored Command.
 */

#ifndef SC_STATE_H
#define SC_STATE_H

#include "cfe.h"

/**
 * \brief Gets the next wakeup count for an RTS command to run
 *
 *  \par Description
 *         This function searches the RTS info table to find
 *         the next RTS that needs to run based on the wakeup
 *         count that the RTS needs to run and it's priority.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 */
void SC_GetNextRtsTime(void);

/**
 * \brief Gets the next RTS command to run
 *
 *  \par Description
 *         This routine is called when #SC_ProcessRtpCommand
 *         executes an RTS command and needs to get the next command in
 *         the buffer. This routine will get the next RTS command from the
 *         currently executing RTS on the active RTP. If this routine
 *         finds a fatal error with fetching the next RTS command or cannot
 *         find a next RTS command, then the sequence and RTP is stopped.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 */
void SC_GetNextRtsCommand(void);

/**
 * \brief Gets the next ATS command to run
 *
 *  \par Description
 *         This routine gets the next ATS command from the currently
 *         executing ATS buffer. If there is no next ATS command then
 *         this routine will stop the currently running ATS.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 */
void SC_GetNextAtsCommand(void);

#endif
