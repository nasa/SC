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
 *  The CFS Stored Command (SC) Application command function codes
 */
#ifndef DEFAULT_SC_FCNCODE_VALUES_H
#define DEFAULT_SC_FCNCODE_VALUES_H

#define SC_CCVAL(x) SC_FunctionCode_##x

enum SC_FunctionCode
{
    SC_FunctionCode_NOOP                    = 0,
    SC_FunctionCode_RESET_COUNTERS          = 1,
    SC_FunctionCode_START_ATS               = 2,
    SC_FunctionCode_STOP_ATS                = 3,
    SC_FunctionCode_START_RTS               = 4,
    SC_FunctionCode_STOP_RTS                = 5,
    SC_FunctionCode_DISABLE_RTS             = 6,
    SC_FunctionCode_ENABLE_RTS              = 7,
    SC_FunctionCode_SWITCH_ATS              = 8,
    SC_FunctionCode_JUMP_ATS                = 9,
    SC_FunctionCode_CONTINUE_ATS_ON_FAILURE = 10,
    SC_FunctionCode_APPEND_ATS              = 11,
    SC_FunctionCode_MANAGE_TABLE            = 12,
    SC_FunctionCode_START_RTS_GRP           = 13,
    SC_FunctionCode_STOP_RTS_GRP            = 14,
    SC_FunctionCode_DISABLE_RTS_GRP         = 15,
    SC_FunctionCode_ENABLE_RTS_GRP          = 16,
};

#endif
