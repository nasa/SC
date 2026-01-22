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
 *   CFS Stored Command (SC) Application Message IDs
 */
#ifndef DEFAULT_SC_MSGIDS_H
#define DEFAULT_SC_MSGIDS_H

#include "sc_msgid_values.h"

/**
 * \defgroup cfssccmdmid CFS Stored Command Command Message IDs
 * \{
 */

#define SC_CMD_MID     CFE_PLATFORM_SC_CMD_MIDVAL(CMD)
#define SC_SEND_HK_MID CFE_PLATFORM_SC_CMD_MIDVAL(SEND_HK)
#define SC_WAKEUP_MID  CFE_PLATFORM_SC_CMD_MIDVAL(WAKEUP)

/**\}*/

/**
 * \defgroup cfssctlmmid CFS Stored Command Telemetry Message IDs
 * \{
 */

#define SC_HK_TLM_MID CFE_PLATFORM_SC_TLM_MIDVAL(HK_TLM) /**< \brief SC Hk Telemetry Message ID ****/

/**\}*/

/* Compatibility identifiers - in case existing SCH table(s) use the old wakeup MID define */
#ifndef SC_OMIT_DEPRECATED
#define SC_1HZ_WAKEUP_MID SC_WAKEUP_MID
#endif

#endif
