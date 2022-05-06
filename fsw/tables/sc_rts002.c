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
 *   CFS Stored Command (SC) sample RTS table 2
 *
 * @note
 * Note 1: The following source code demonstrates how to create a sample
 *         Stored Command RTS table.  The preferred method for creating
 *         flight versions of RTS tables is to use custom ground system
 *         tools that output the binary table files, skipping this step
 *         altogether.
 *
 * Note 2: This source file creates a sample RTS table that contains only
 *         the following commands that are scheduled as follows:
 *
 *         SC NOOP command, execution time relative to start of RTS = 0
 *         SC NOOP command, execution time relative to prev cmd = 5
 *         SC NOOP command, execution time relative to prev cmd = 5
 *
 * Note 3: The byte following the command code in each command packet
 *         secondary header must contain an 8 bit checksum.  Refer to
 *         the SC Users Guide for information on how to calculate this
 *         checksum.
 *
 * Note 4: If the command length (in bytes) is odd, a pad byte must be added
 *         to the RTS command structure (opt data portion) to ensure the next
 *         command starts on a word (uint16) boundary.
 */

#include "cfe.h"
#include "cfe_tbl_filedef.h"
#include "cfe_endian.h"

#include "sc_platform_cfg.h" /* defines table buffer size */
#include "sc_msgdefs.h"      /* defines SC command code values */
#include "sc_msgids.h"       /* defines SC packet msg ID's */

/*
** Execution time for each sample command
*/
#define CMD1_TIME 0
#define CMD2_TIME 5
#define CMD3_TIME 7

/*
** Create execution time as two 16 bit values
*/
#define CMD1_TIME_A ((uint16)((uint32)CMD1_TIME >> 16))
#define CMD2_TIME_A ((uint16)((uint32)CMD2_TIME >> 16))
#define CMD3_TIME_A ((uint16)((uint32)CMD3_TIME >> 16))

#define CMD1_TIME_B ((uint16)((uint32)CMD1_TIME))
#define CMD2_TIME_B ((uint16)((uint32)CMD2_TIME))
#define CMD3_TIME_B ((uint16)((uint32)CMD3_TIME))

/*
** Calculate checksum for each sample command
*/
#define CMD1_XSUM 0x008F
#define CMD2_XSUM 0x008F
#define CMD3_XSUM 0x008F

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
#define CMD2_LENGTH 1
#define CMD3_LENGTH 1

/*
** Sample cFE Table Header
*/
static CFE_TBL_FileDef_t CFE_TBL_FileDef __attribute__((__used__)) = {
    "RTS_Table002", "SC.RTS_TBL002", "SC Sample RTS_TBL002", "sc_rts002.tbl", (SC_RTS_BUFF_SIZE * sizeof(uint16))};

/*
** Sample RTS Table Data
*/
uint16 RTS_Table002[SC_RTS_BUFF_SIZE] = {
    /*  cmd time,  <---------------------------- cmd pkt primary header ---------------------------->  <----- cmd pkt
       2nd header ---->   <-- opt data ---> */
    CMD1_TIME_A,
    CMD1_TIME_B,
    CFE_MAKE_BIG16(SC_CMD_MID),
    CFE_MAKE_BIG16(PKT_FLAGS),
    CFE_MAKE_BIG16(CMD1_LENGTH),
    CFE_MAKE_BIG16((SC_NOOP_CC << 8) | CMD1_XSUM),
    CMD2_TIME_A,
    CMD2_TIME_B,
    CFE_MAKE_BIG16(SC_CMD_MID),
    CFE_MAKE_BIG16(PKT_FLAGS),
    CFE_MAKE_BIG16(CMD2_LENGTH),
    CFE_MAKE_BIG16((SC_NOOP_CC << 8) | CMD2_XSUM),
    CMD3_TIME_A,
    CMD3_TIME_B,
    CFE_MAKE_BIG16(SC_CMD_MID),
    CFE_MAKE_BIG16(PKT_FLAGS),
    CFE_MAKE_BIG16(CMD3_LENGTH),
    CFE_MAKE_BIG16((SC_NOOP_CC << 8) | CMD3_XSUM),
};

/************************/
/*  End of File Comment */
/************************/
