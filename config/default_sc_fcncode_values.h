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
 *  The CFS Data Storage (DS) Application Message IDs header file
 */
#ifndef DEFAULT_SC_FCNCODE_VALUES_H
#define DEFAULT_SC_FCNCODE_VALUES_H

#define SC_CCVAL(x) SC_FunctionCode_##x

enum SC_FunctionCode
{
    SC_FunctionCode_NOOP             = 0,
    SC_FunctionCode_RESET_COUNTERS   = 1,
    SC_FunctionCode_SET_APP_STATE    = 2,
    SC_FunctionCode_SET_FILTER_FILE  = 3,
    SC_FunctionCode_SET_FILTER_TYPE  = 4,
    SC_FunctionCode_SET_FILTER_PARMS = 5,
    SC_FunctionCode_SET_DEST_TYPE    = 6,
    SC_FunctionCode_SET_DEST_STATE   = 7,
    SC_FunctionCode_SET_DEST_PATH    = 8,
    SC_FunctionCode_SET_DEST_BASE    = 9,
    SC_FunctionCode_SET_DEST_EXT     = 10,
    SC_FunctionCode_SET_DEST_SIZE    = 11,
    SC_FunctionCode_SET_DEST_AGE     = 12,
    SC_FunctionCode_SET_DEST_COUNT   = 13,
    SC_FunctionCode_CLOSE_FILE       = 14,
    SC_FunctionCode_GET_FILE_INFO    = 15,
    SC_FunctionCode_ADD_MID          = 16,
    SC_FunctionCode_REMOVE_MID       = 17,
    SC_FunctionCode_CLOSE_ALL        = 18,
};

#endif
