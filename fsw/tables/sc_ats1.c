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
 *   CFS Stored Command (SC) sample ATS table 1
 *
 * @note
 * Note 1: The following source code demonstrates how to create a sample
 *         Stored Command ATS table.  The preferred method for creating
 *         flight versions of ATS tables is to use custom ground system
 *         tools that output the binary table files, skipping this step
 *         altogether.
 *
 * Note 2: This source file creates a sample ATS table that contains the
 *         following commands that are scheduled as follows:
 *
 *         SC NOOP command, execution time = SC_TEST_TIME + 30
 *         SC Enable RTS #1 command, execution time = SC_TEST_TIME + 35
 *         SC Start RTS #1 command, execution time = SC_TEST_TIME + 40
 *         SC Reset Counters command, execution time = SC_TEST_TIME + 100
 *
 * Note 3: Before starting the sample ATS, set time = SC_TEST_TIME.  The
 *         user will then have 30 seconds to start the ATS before the
 *         first command in the sample ATS is scheduled to execute.
 *
 * Note 4: The byte following the command code in each command packet
 *         secondary header must contain an 8 bit checksum.  Refer to
 *         the SC Users Guide for information on how to calculate this
 *         checksum.
 *
 * Note 5: If the command length (in bytes) is odd, a pad byte must be added
 *         to the ATS command structure (opt data portion) to ensure the next
 *         command starts on a word (uint16) boundary.
 *
 * Note 6: There is a crucial safety measure that is required of all ATS tables.
 *         The ATP relies on a sentinel word of zeroes at the end of an ATS table
 *         to signal the end of the ATS table (end of data marker).
 */

#include "cfe.h"
#include "cfe_endian.h"
#include "cfe_tbl_filedef.h"

#include "sc_platform_cfg.h" /* defines table buffer size */
#include "sc_msgdefs.h"      /* defines SC command code values */
#include "sc_msgids.h"       /* defines SC packet msg ID's */

/*
** Arbitrary spacecraft time for start of sample ATS
*/
#define TEST_TIME 1000000

/*
** Execution time for each sample command
*/
#define CMD1_TIME (TEST_TIME + 30)
#define CMD2_TIME (TEST_TIME + 35)
#define CMD3_TIME (TEST_TIME + 40)
#define CMD4_TIME (TEST_TIME + 100)

/*
** Create execution time as Most Signficant (MS) and Least Signficant (LS) 16 bit values
*/
#define CMD1_TIME_MS ((CMD1_TIME >> 16) & 0xFFFF)
#define CMD2_TIME_MS ((CMD2_TIME >> 16) & 0xFFFF)
#define CMD3_TIME_MS ((CMD3_TIME >> 16) & 0xFFFF)
#define CMD4_TIME_MS ((CMD4_TIME >> 16) & 0xFFFF)

#define CMD1_TIME_LS (CMD1_TIME & 0xFFFF)
#define CMD2_TIME_LS (CMD2_TIME & 0xFFFF)
#define CMD3_TIME_LS (CMD3_TIME & 0xFFFF)
#define CMD4_TIME_LS (CMD4_TIME & 0xFFFF)

/*
** Calculate checksum for each sample command
*/
#define CMD1_XSUM 0x008F
#define CMD2_XSUM 0x008D
#define CMD3_XSUM 0x008E
#define CMD4_XSUM 0x008E

/*
** Optional command data values
*/
#define CMD2_ARG 1
#define CMD3_ARG 1

/*
** Command packet segment flags and sequence counter
** - 2 bits of segment flags (0xC000 = start and end of packet)
** - 14 bits of sequence count (unused for command packets)
*/
#define PKT_FLAGS 0xC000

/*
** Length of cmd pkt data (in bytes minus one) that follows primary header (thus, 0xFFFF = 64k)
*/
#define CMD1_LENGTH 1
#define CMD2_LENGTH 5
#define CMD3_LENGTH 5
#define CMD4_LENGTH 1

/*
** Sample ATS_TBL1 Table Header
*/
static CFE_TBL_FileDef_t CFE_TBL_FileDef __attribute__((__used__)) = {
    "ATS_Table1", "SC.ATS_TBL1", "SC Sample ATS_TBL1", "sc_ats1.tbl", (SC_ATS_BUFF_SIZE * sizeof(uint16))};

/*
** Sample ATS_TBL1 Table Data
*/
uint16 ATS_Table1[SC_ATS_BUFF_SIZE] = {
    /* cmd num, <---- cmd exe time ---->   <---------------------------- cmd pkt primary header
       ---------------------------->  <----- cmd pkt 2nd header ---->   <-- opt data ---> */
    0,
    1,
    CMD1_TIME_MS,
    CMD1_TIME_LS,
    CFE_MAKE_BIG16(SC_CMD_MID),
    CFE_MAKE_BIG16(PKT_FLAGS),
    CFE_MAKE_BIG16(CMD1_LENGTH),
    CFE_MAKE_BIG16((SC_NOOP_CC << 8) | CMD1_XSUM),
    0,
    2,
    CMD2_TIME_MS,
    CMD2_TIME_LS,
    CFE_MAKE_BIG16(SC_CMD_MID),
    CFE_MAKE_BIG16(PKT_FLAGS),
    CFE_MAKE_BIG16(CMD2_LENGTH),
    CFE_MAKE_BIG16((SC_ENABLE_RTS_CC << 8) | CMD2_XSUM),
    CMD2_ARG,
    0x0000,
    0,
    3,
    CMD3_TIME_MS,
    CMD3_TIME_LS,
    CFE_MAKE_BIG16(SC_CMD_MID),
    CFE_MAKE_BIG16(PKT_FLAGS),
    CFE_MAKE_BIG16(CMD3_LENGTH),
    CFE_MAKE_BIG16((SC_START_RTS_CC << 8) | CMD3_XSUM),
    CMD3_ARG,
    0x0000,
    0,
    4,
    CMD4_TIME_MS,
    CMD4_TIME_LS,
    CFE_MAKE_BIG16(SC_CMD_MID),
    CFE_MAKE_BIG16(PKT_FLAGS),
    CFE_MAKE_BIG16(CMD4_LENGTH),
    CFE_MAKE_BIG16((SC_RESET_COUNTERS_CC << 8) | CMD4_XSUM),
    0,
    0,
    0,
    0,
    0,
    0,
    0};

/************************/
/*  End of File Comment */
/************************/
