/*************************************************************************
** File: sc_platform_cfg.h 
**
**  Copyright © 2007-2014 United States Government as represented by the
**  Administrator of the National Aeronautics and Space Administration.
**  All Other Rights Reserved.
**
**  This software was created at NASA's Goddard Space Flight Center.
**  This software is governed by the NASA Open Source Agreement and may be
**  used, distributed and modified only pursuant to the terms of that
**  agreement.
**
** Purpose:
**   This file contains the platform configuration parameters used by
**   Stored Command
**
** References:
**   Flight Software Branch C Coding Standard Version 1.2
**   CFS Development Standards Document
**
*************************************************************************/
#ifndef _sc_platform_cfg_
#define _sc_platform_cfg_

/*************************************************************************
 ** Macro Definitions
 *************************************************************************/

/** \sccfg  Max number of commands per second
**
**  \par Description:
**       Maximum number of commands that can be sent out by SC
*        in any given second.
**
**  \par Limits:
**       This parameter can't be larger than an unsigned 16 bit
**       integer (65535), but should be kepoot relatively small to
**       avoid SC hogging the CPU
**
*/
#define SC_MAX_CMDS_PER_SEC 8

/** \sccfg  Number of RTS's
**
**  \par Description:
**       The number of RTS's allowed in the system
**
**  \par Limits:
**       This parameter can't be larger than 999.This parameter will dicate the size of
**       The RTS Info Table.
*/
#define SC_NUMBER_OF_RTS 64

/** \sccfg Max buffer size for an ATS in uint16s
**
**  \par Description:
**       The max sizeof an ATS buffer in words (not bytes)
**  \par Limits:
**       This parameter can't be larger than an unsigned 16 bit
**       integer (65535).
*/
#define SC_ATS_BUFF_SIZE 8000

/** \sccfg Max buffer size for an Append ATS in uint16s
**
**  \par Description:
**       The max sizeof an Append ATS buffer in words (not bytes)
**  \par Limits:
**       This parameter cannot be larger than SC_ATS_BUFF_SIZE.
*/
#define SC_APPEND_BUFF_SIZE (SC_ATS_BUFF_SIZE / 2)

/** \sccfg Max buffer size for an RTS in uint16s
**
**  \par Description:
**       The max size of an RTS buffer in WORDS (not bytes)
**  \par Limits:
**       This parameter can't be larger than an unsigned 16 bit
**       integer (65535).
*/
#define SC_RTS_BUFF_SIZE 150

/** \sccfg Max number of commands in each ATS
**
**  \par Description:
**       The maximum number of commands that are allowed in each ATS
**
**  \par Limits:
**       This parameter can't be larger than an unsigned 16 bit
**       integer (65535).
*/
#define SC_MAX_ATS_CMDS 1000

/** \sccfg The last RTS that will be sent with an #SC_RTS_START_INF_EID event message
**
**  \par Description:
**       When all RTS's are started, the #SC_RTS_START_INF_EID event message is sent out.
**       This parameter suppreses that message for all RTS's over this number
**
**  \par Limits:
**       This parameter needs to be less than or equal to SC_NUMBER_OF_RTS
*/
#define SC_LAST_RTS_WITH_EVENTS 20

/** \sccfg Minimum Packet Size
**
**  \par Description:
**       This parameter specifies the minumum size in bytes for an ATS or RTS command.
**  \par Limits:
**       This parameter must be greater than or equal to CFE_SB_CMD_HDR_SIZE and
**       less than or equal to CFE_MISSION_SB_MAX_SB_MSG_SIZE.
*/
#define SC_PACKET_MIN_SIZE 8

/** \sccfg Maximum Packet Size
**
**  \par Description:
**       This parameter specifies the maximum size in bytes for an ATS or RTS command.
**  \par Limits:
**       This parameter must be greater than or equal to SC_PACKET_MIN_SIZE and
**       less than or equal to CFE_MISSION_SB_MAX_SB_MSG_SIZE.
*/
#define SC_PACKET_MAX_SIZE 256

/** \sccfg Command Pipe Depth
**
**  \par Description:
**       Maximum number of messages that will be allowed in the
**       SC command pipe at one time. Used during initialization
**       in the call to #CFE_SB_CreatePipe
**
**  \par Limits:
**       This parameter must be greater than zero and less than or equal to
**       CFE_SB_MAX_PIPE_DEPTH.
*/
#define SC_PIPE_DEPTH 12

/** \sccfg ATS Table Filenames
**
**  \par Description:
**       Filenames for the ATS tables loaded at startup are constructed by
**       appending a one digit table identifier plus the extension ".tbl"
**       to the base portion of the filename defined here.
**
**       The default definitions will create ATS filenames as follows:
**       "/cf/apps/sc_ats1.tbl or /cf/apps/sc_ats2.tbl"
**
**  \par Limits:
**       SC requires that this name be defined, but otherwise places no
**       limits on the definition.  If the named table file does not exist
**       at run time, or the table fails validation, then the table load
**       will fail.  Refer to #OS_MAX_PATH_LEN for filename length limits.
*/
#define SC_ATS_FILE_NAME "/cf/sc_ats"

/** \sccfg Append ATS Table Filename
**
**  \par Description:
**       This name describes the default append ATS filename loaded at
**       startup.  Often the default Append ATS file contains only a
**       single unused table entry, and is used only to initialize the
**       table state as having data that may be patched.
**
**  \par Limits:
**       SC requires that this name be defined, but otherwise places no
**       limits on the definition.  If the named table file does not exist
**       at run time, or the table fails validation, then the table load
**       will fail.  Refer to #OS_MAX_PATH_LEN for filename length limits.
*/
#define SC_APPEND_FILE_NAME "/cf/sc_append.tbl"

/** \sccfg RTS Table Filenames
**
**  \par Description:
**       Filenames for the RTS tables loaded at startup are constructed by
**       appending a one digit table identifier plus the extension ".tbl"
**       to the base portion of the filename defined here.
**
**       The default definitions will create RTS filenames as follows:
**       "/cf/apps/sc_rts001.tbl, /cf/apps/sc_rts002.tbl, etc"
**
**  \par Limits:
**       SC requires that this name be defined, but otherwise places no
**       limits on the definition.  If the named table file does not exist
**       at run time, or the table fails validation, then the table load
**       will fail.  Refer to #OS_MAX_PATH_LEN for filename length limits.
*/
#define SC_RTS_FILE_NAME "/cf/sc_rts"

/** \sccfg ATS Table Object Names
**
**  \par Description:
**       Unique table object names are required for each table registered
**       with cFE Table Services.  ATS table object names are constructed
**       by appending a one digit table identifier to the base portion of
**       the object name defined here.
**
**       The default definitions will create ATS object names as follows:
**       "ATS_TBL1 or ATS_TBL2"
**
**  \par Limits:
**       SC requires that this name be defined, but otherwise places no
**       limits on the definition.  Refer to #CFE_TBL_MAX_NAME_LENGTH for
**       specific information on limits related to table object names.
*/
#define SC_ATS_TABLE_NAME "ATS_TBL"

/** \sccfg Append ATS Table Object Names
**
**  \par Description:
**       Unique table object names are required for each table registered
**       with cFE Table Services.  This is the table object name for the
**       Append ATS table.
**
**  \par Limits:
**       SC requires that this name be defined, but otherwise places no
**       limits on the definition.  Refer to #CFE_TBL_MAX_NAME_LENGTH for
**       specific information on limits related to table object names.
*/
#define SC_APPEND_TABLE_NAME "APPEND_TBL"

/** \sccfg RTS Table Object Names
**
**  \par Description:
**       Unique table object names are required for each table registered
**       with cFE Table Services.  RTS table object names are constructed
**       by appending a three digit table identifier to the base portion
**       of the object name defined here.
**
**       The default definitions will create RTS object names as follows:
**       "RTS_TBL001, RTS_TBL002, etc"
**
**  \par Limits:
**       SC requires that this name be defined, but otherwise places no
**       limits on the definition.  Refer to #CFE_TBL_MAX_NAME_LENGTH for
**       specific information on limits related to table object names.
*/
#define SC_RTS_TABLE_NAME "RTS_TBL"

/** \sccfg Name of the RTS Infomation Table
**
**  \par Description:
**       Name of the RTS Information Table
**  \par Limits:
**       Must be less than #CFE_TBL_MAX_NAME_LENGTH
*/
#define SC_RTSINFO_TABLE_NAME "RTSINF_TBL"

/** \sccfg Name of the RTP Control block table
**
**  \par Description:
**       Name of the RTP Control Block Table
**  \par Limits:
**       Must be less than #CFE_TBL_MAX_NAME_LENGTH
*/
#define SC_RTP_CTRL_TABLE_NAME "RTPCTR_TBL"

/** \sccfg Name of the ATS Infomation Table
**
**  \par Description:
**       Name of the ATS Information Table
**  \par Limits:
**       Must be less than #CFE_TBL_MAX_NAME_LENGTH
*/
#define SC_ATSINFO_TABLE_NAME "ATSINF_TBL"

/** \sccfg Name of the Append ATS Infomation Table
**
**  \par Description:
**       Name of the Append ATS Information Table
**  \par Limits:
**       Must be less than #CFE_TBL_MAX_NAME_LENGTH
*/
#define SC_APPENDINFO_TABLE_NAME "APPINF_TBL"

/** \sccfg Name of the ATP Control block table
**
**  \par Description:
**       Name of the ATP Control Block Table
**  \par Limits:
**       Must be less than #CFE_TBL_MAX_NAME_LENGTH
*/
#define SC_ATS_CTRL_TABLE_NAME "ATPCTR_TBL"

/** \sccfg Name Prefix of ATS Cmd Status Table
 **
 **  \par Description:
 **       The prefix of the ATS Command Status table names. Note that actual table names
 **       will have a 1 digit number postfixed to it depending on the
 **       ATS number
 **
 **  \par Limits:
 **       Total length must be less than #CFE_TBL_MAX_NAME_LENGTH
 */
#define SC_ATS_CMD_STAT_TABLE_NAME "ATSCMD_TBL"

/** \sccfg Defnies default state of Continue-Ats-On-Checksum-Failure Flag
**
**  \par Description:
**       This parameter specifies the default state to continue an ATS
**       when a command in the ATS fails checksum validation
**
**  \par Limits:
**       Must be true or false
*/
#define SC_CONT_ON_FAILURE_START true

/** \sccfg Defines the TIME SC should use for its commands
**
**
**  \par Description:
**       This parameter defines what type of time SC should use for sending uot its commands
**
**  \par Limits:
**       Must be SC_USE_CFE_TIME, SC_USE_TAI, or SC_USE_UTC */
#define SC_TIME_TO_USE SC_USE_CFE_TIME

/** \sccfg Define inclusion state for RTS group commands
**
**  \par Description:
**       This parameter specifies the inclusion state for the
**       following RTS group commands: Start RTS group, Stop
**       RTS group, Enable RTS group and Disable RTS group.
**       RTS group commands affect a range of consecutive RTS
**       numbers.  When set to true, this definition results
**       in the inclusion of the group command handlers into
**       the SC source code.
**
**  \par Limits:
**       Must be defined as true or false
*/
#define SC_ENABLE_GROUP_COMMANDS true

/** \sccfg Mission specific version number for SC application
**
**  \par Description:
**       An application version number consists of four parts:
**       major version number, minor version number, revision
**       number and mission specific revision number. The mission
**       specific revision number is defined here and the other
**       parts are defined in "sc_version.h".
**
**  \par Limits:
**       Must be defined as a numeric value that is greater than
**       or equal to zero.
*/
#define SC_MISSION_REV 0

#endif /*_sc_platform_cfg_*/

/************************/
/*  End of File Comment */
/************************/
