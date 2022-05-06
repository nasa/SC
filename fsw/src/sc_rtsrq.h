/************************************************************************
 * NASA Docket No. GSC-18,924-1, and identified as “Core Flight
 * System (cFS) Stored Command Application version 3.1.0”
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
 *   This file contains the headers to handle all of the RTS
 *   executive requests and internal reuqests to control
 *   the RTP and RTSs.
 */
#ifndef SC_RTSRQ_H
#define SC_RTSRQ_H

#include "cfe.h"

/**
 * \brief Start an RTS Command
 *
 *  \par Description
 *             This routine starts the execution of an RTS.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]         CmdPacket      Pointer to Software Bus buffer
 *
 *  \sa #SC_START_RTS_CC
 */
void SC_StartRtsCmd(const CFE_SB_Buffer_t *CmdPacket);

#if (SC_ENABLE_GROUP_COMMANDS == true)

/**
 * \brief Start a group of RTS Command
 *
 *  \par Description
 *             This routine starts the execution of a group of RTS.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]         CmdPacket      Pointer to Software Bus buffer
 *
 *  \sa #SC_START_RTSGRP_CC
 */
void SC_StartRtsGrpCmd(const CFE_SB_Buffer_t *CmdPacket);
#endif

/**
 * \brief  Stop an RTS from executing Command
 *
 *  \par Description
 *             This routine stops the execution of an RTS.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]         CmdPacket      Pointer to Software Bus buffer
 *
 *  \sa #SC_STOP_RTS_CC
 */
void SC_StopRtsCmd(const CFE_SB_Buffer_t *CmdPacket);

#if (SC_ENABLE_GROUP_COMMANDS == true)

/**
 * \brief  Stop a group of RTS from executing Command
 *
 *  \par Description
 *             This routine stops the execution of a group of RTS.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]         CmdPacket      Pointer to Software Bus buffer
 *
 *  \sa #SC_STOP_RTS_CC
 */
void SC_StopRtsGrpCmd(const CFE_SB_Buffer_t *CmdPacket);
#endif

/**
 * \brief Disable an RTS Command
 *
 *  \par Description
 *             This routine disables an enabled RTS.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]         CmdPacket      Pointer to Software Bus buffer
 *
 *  \sa #SC_DISABLE_RTS_CC
 */
void SC_DisableRtsCmd(const CFE_SB_Buffer_t *CmdPacket);

#if (SC_ENABLE_GROUP_COMMANDS == true)

/**
 * \brief Disable a group of RTS Command
 *
 *  \par Description
 *             This routine disables a group of enabled RTS.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]         CmdPacket      Pointer to Software Bus buffer
 *
 *  \sa #SC_DISABLE_RTS_CC
 */
void SC_DisableRtsGrpCmd(const CFE_SB_Buffer_t *CmdPacket);
#endif

/**
 * \brief Enable an RTS Command
 *
 *  \par Description
 *             This routine enables a disabled RTS.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]         CmdPacket      Pointer to Software Bus buffer
 *
 *  \sa #SC_ENABLE_RTS_CC
 */
void SC_EnableRtsCmd(const CFE_SB_Buffer_t *CmdPacket);

#if (SC_ENABLE_GROUP_COMMANDS == true)

/**
 * \brief Enable a group of RTS Command
 *
 *  \par Description
 *             This routine enables a group of disabled RTS.
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]         CmdPacket      Pointer to Software Bus buffer
 *
 *  \sa #SC_ENABLE_RTSGRP_CC
 */
void SC_EnableRtsGrpCmd(const CFE_SB_Buffer_t *CmdPacket);
#endif

/**
 * \brief Stops an RTS & clears out data
 *
 *  \par Description
 *      This is a generic routine to stop an RTS
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]         RtsIndex       RTS index to kill (base zero)
 */
void SC_KillRts(uint16 RtsIndex);

/**
 * \brief Automatically starts an RTS
 *
 *  \par Description
 *        This function sends a command back to the SC app to
 *        start the RTS designated as the auto-start RTS (usually 1)
 *
 *  \par Assumptions, External Events, and Notes:
 *        None
 *
 *  \param [in]         RtsNumber      RTS number to start (base one)
 */
void SC_AutoStartRts(uint16 RtsNumber);

#endif
