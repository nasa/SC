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
 * Provides default definitions for HK table structures
 *
 * @note This file may be overridden/superceded by mission-provided defintions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef DEFAULT_SC_TBLSTRUCT_H
#define DEFAULT_SC_TBLSTRUCT_H

/*************************************************************************
 * Includes
 *************************************************************************/
#include "common_types.h"
#include "sc_tbldefs.h"
#include "sc_mission_cfg.h"
#include "cfe_mission_cfg.h"

/**
 *  \brief ATP Control Block Type
 */
typedef struct
{
    SC_Status_Enum_t AtpState;       /**< \brief execution state of the ATP */
    SC_AtsNum_t      CurrAtsNum;     /**< \brief current ATS running if any */
    uint16           Padding;        /**< \brief Structure padding to align to 32-bit boundaries */
    SC_CommandNum_t  CmdNumber;      /**< \brief current cmd number to run if any */
    SC_SeqIndex_t    TimeIndexPtr;   /**< \brief time index pointer for current cmd */
    uint16           SwitchPendFlag; /**< \brief indicates that a buffer switch is waiting */
} SC_AtpControlBlock_t;

/**
 *  \brief RTP Control Block Type
 *
 *  \note Now there is only really one RTP
 *  This structure contains overall info for the next relative time
 *  processor.
 */
typedef struct
{
    uint16      NumRtsActive; /**< \brief number of RTSs currently active */
    SC_RtsNum_t CurrRtsNum;   /**< \brief next RTS number */
} SC_RtpControlBlock_t;

#endif
