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
 *   CFS Stored Command (SC) Application Message IDs
 */
#ifndef SC_MSGIDS_H
#define SC_MSGIDS_H

/**
 * \defgroup cfssccmdmid CFS Stored Command Command Message IDs
 * \{
 */

#define SC_CMD_MID          (0x18A9) /**< \brief Msg ID for cmds to SC   */
#define SC_SEND_HK_MID      (0x18AA) /**< \brief Msg ID to request SC HK */
#define SC_WAKEUP_MID       (0x18AB) /**< \brief Msg ID to receive the wakeup command */

/**\}*/

/**
 * \defgroup cfssctlmmid CFS Stored Command Telemetry Message IDs
 * \{
 */

#define SC_HK_TLM_MID (0x08AA) /**< \brief Msg ID to send telemtry down on */

/**\}*/

/* Compatibility identifiers - in case existing SCH table(s) use the old wakeup MID define */
#ifndef SC_OMIT_DEPRECATED
#define SC_1HZ_WAKEUP_MID SC_WAKEUP_MID
#define SC_ONEHZ_WAKEUP_MID SC_WAKEUP_MID
#endif

#endif
