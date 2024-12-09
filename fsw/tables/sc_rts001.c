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
 *   CFS Stored Command (SC) sample RTS table 1
 *
 * The following source code demonstrates how to create a sample
 * Stored Command RTS table using the software defined command structures.
 * It's also possible to create this table via alternative tools
 * (ground system) and or system agnostic data definitions (XTCE/EDS/JSON).
 *
 * This source file creates a sample RTS table that contains only
 * the following commands that are scheduled as follows:
 *
 * SC NOOP command, execution wakeup count relative to start of RTS = 0
 * SC Enable RTS #2 command, execution wakeup count relative to prev cmd = 5
 * SC Start RTS #2 command, execution wakeup count relative to prev cmd = 5
 */

#include "cfe.h"
#include "cfe_tbl_filedef.h"

#include "sc_tbldefs.h"      /* defines SC table headers */
#include "sc_platform_cfg.h" /* defines table buffer size */
#include "sc_msgdefs.h"      /* defines SC command code values */
#include "sc_msgids.h"       /* defines SC packet msg ID's */
#include "sc_msg.h"          /* defines SC message structures */

/* Checksum for each sample command */
#ifndef SC_NOOP_CKSUM
#define SC_NOOP_CKSUM (0x8F)
#endif
#ifndef SC_ENABLE_RTS2_CKSUM
#define SC_ENABLE_RTS2_CKSUM (0x8E)
#endif
#ifndef SC_START_RTS2_CKSUM
#define SC_START_RTS2_CKSUM (0x8D)
#endif

/* Custom table structure, modify as needed to add desired commands */
typedef struct
{
    SC_RtsEntryHeader_t hdr1;
    SC_NoopCmd_t        cmd1;
    SC_RtsEntryHeader_t hdr2;
    SC_EnableRtsCmd_t   cmd2;
    SC_RtsEntryHeader_t hdr3;
    SC_StartRtsCmd_t    cmd3;
} SC_RtsStruct001_t;

/* Define the union to size the table correctly */
typedef union
{
    SC_RtsStruct001_t rts;
    uint16            buf[SC_RTS_BUFF_SIZE];
} SC_RtsTable001_t;

/* Helper macro to get size of structure elements */
#define SC_MEMBER_SIZE(member) (sizeof(((SC_RtsStruct001_t *)0)->member))

/* Used designated initializers to be verbose, modify as needed/desired */
SC_RtsTable001_t SC_Rts001 = {
    /* 1 */
    .rts.hdr1.WakeupCount = 0,
    .rts.cmd1.CommandHeader = CFE_MSG_CMD_HDR_INIT(SC_CMD_MID, SC_MEMBER_SIZE(cmd1), SC_NOOP_CC, SC_NOOP_CKSUM),

    /* 2 */
    .rts.hdr2.WakeupCount = 5,
    .rts.cmd2.CommandHeader =
        CFE_MSG_CMD_HDR_INIT(SC_CMD_MID, SC_MEMBER_SIZE(cmd2), SC_ENABLE_RTS_CC, SC_ENABLE_RTS2_CKSUM),
    .rts.cmd2.Payload.RtsNum = SC_RTS_NUM_INITIALIZER(2),

    /* 3 */
    .rts.hdr3.WakeupCount = 5,
    .rts.cmd3.CommandHeader =
        CFE_MSG_CMD_HDR_INIT(SC_CMD_MID, SC_MEMBER_SIZE(cmd3), SC_START_RTS_CC, SC_START_RTS2_CKSUM),
    .rts.cmd3.Payload.RtsNum = SC_RTS_NUM_INITIALIZER(2)};

/* Macro for table structure */
CFE_TBL_FILEDEF(SC_Rts001, SC.RTS_TBL001, SC Example RTS_TBL001, sc_rts001.tbl)
