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
 *   This file contains functions to handle dispatching of commands
 */
#ifndef SC_DISPATCH_H
#define SC_DISPATCH_H

#include "cfe_sb.h"
#include "cfe_msg.h"

/**
 * \brief Verify command message length
 *
 *  \par Description
 *       This routine will check if the actual length of a software bus
 *       command message matches the expected length and send an
 *       error event message if a mismatch occurs
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   Msg              Pointer to message
 *  \param [in]   ExpectedLength   The expected length of the message
 *                                 based upon the command code
 *
 *  \return Boolean length verification result
 *  \retval true  Length matches expected
 *  \retval false Length does not match expected
 *
 *  \sa #SC_CMD_LEN_ERR_EID
 */
bool SC_VerifyCmdLength(const CFE_MSG_Message_t *Msg, size_t ExpectedLength);

/**
 * \brief Routes commands to be processed
 *
 *  \par Description
 *           This routine determines the source of a request to
 *           the Stored Command processor and routes it to one of the lower
 *           level request processing routines
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 */
void SC_ProcessRequest(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Processes commands
 *
 *  \par Description
 *       Process commands. Commands can be from external sources or from SC
 *       itself.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 */
void SC_ProcessCommand(const CFE_SB_Buffer_t *BufPtr);

#endif
