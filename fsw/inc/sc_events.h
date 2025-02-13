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
 *   his file contains the definitions of all of the events sent by
 *   the Stored Command Processor
 */
#ifndef SC_EVENTS_H
#define SC_EVENTS_H

/**
 * \defgroup cfsscevents CFS Stored Command Event IDs
 * \{
 */

/**
 * \brief SC Application Fatal Termination Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when the App exits
 *  due to a fatal error condition
 */
#define SC_APP_EXIT_ERR_EID 1

/**
 * \brief SC Command Message Length Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a command is received, but it is not of the expected
 *  length
 */
#define SC_CMD_LEN_ERR_EID 2

/**
 * \brief SC Create Pipe Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when #CFE_SB_CreatePipe returns an
 *  error
 */
#define SC_CR_PIPE_ERR_EID 3

/**
 * \brief SC Housekeeping Request Subscribe Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when #CFE_SB_Subscribe to the Housekeeping
 *  Request packet fails
 */
#define SC_INIT_SB_SUBSCRIBE_HK_ERR_EID 4

/**
 * \brief SC Wakeup Cycle Message Subscribe Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when #CFE_SB_Subscribe to the wakeup
 *  request packet fails
 */
#define SC_INIT_SB_SUBSCRIBE_ERR_EID 5

/**
 * \brief HS Command Message Subscribe Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when #CFE_SB_Subscribe to the SC Command
 *  Request packet fails
 */
#define SC_INIT_SB_SUBSCRIBE_CMD_ERR_EID 6

/**
 * \brief SC Initialization Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when the App has
 *  completed initialization.
 */
#define SC_INIT_INF_EID 9

/**
 * \brief SC RTS Table Registration Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a call to #CFE_TBL_Register for an RTS table failed
 */
#define SC_REGISTER_RTS_TBL_ERR_EID 10

/**
 * \brief SC ATS Table Registration Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a call to #CFE_TBL_Register for an ATS table failed
 */
#define SC_REGISTER_ATS_TBL_ERR_EID 11

/**
 * \brief SC RTS Info Table Registration Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when trying to register the RTS Info Table
 *  dump only table fails
 */
#define SC_REGISTER_RTS_INFO_TABLE_ERR_EID 16

/**
 * \brief SC RTS Control Block Table Registration Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when trying to register the RTS control block
 *  dump only table fails
 */
#define SC_REGISTER_RTS_CTRL_BLK_TABLE_ERR_EID 17

/**
 * \brief SC ATS Info Table Registration Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when trying to register the ATS Info Table
 *  dump only table fails
 */
#define SC_REGISTER_ATS_INFO_TABLE_ERR_EID 18

/**
 * \brief SC ATS Control Block Table Registration Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when trying to register the ATS control block
 *  dump only table fails
 */
#define SC_REGISTER_ATS_CTRL_BLK_TABLE_ERR_EID 19

/**
 * \brief SC ATS Command Status Table Registration Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when one of the ATS command status tables fails table
 *  registration
 */
#define SC_REGISTER_ATS_CMD_STATUS_TABLE_ERR_EID 20

/**
 * \brief SC RTS Table Load Failure Count Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message indicates the number of RTS tables that failed
 *  to load at startup. This can be either from failure to open the RTS file
 *  or from a failure to load the RTS table.
 */
#define SC_RTS_LOAD_FAIL_COUNT_INFO_EID 21

/**
 * \brief SC ATS Execution Started Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when an ATS is started successfully
 */
#define SC_STARTATS_CMD_INF_EID 23

/**
 * \brief SC ATS Start Rejected Table Not Loaded Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_START_ATS_CC cmd failed because
 *  the specified ATS was not loaded.
 */
#define SC_STARTATS_CMD_NOT_LDED_ERR_EID 24

/**
 * \brief SC ATS Start Rejected ATP Not Idle Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_START_ATS_CC command was issued
 *  but there is already an ATS running
 */
#define SC_STARTATS_CMD_NOT_IDLE_ERR_EID 25

/**
 * \brief SC ATS Start Rejected ATS ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when the ATS Id specified in the
 *  #SC_START_ATS_CC command was invalid
 */
#define SC_STARTATS_CMD_INVLD_ID_ERR_EID 26

/**
 * \brief SC ATS Stopped Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when a #SC_STOP_ATS_CC command successfully
 *  stopped an ATS
 */
#define SC_STOPATS_CMD_INF_EID 27

/**
 * \brief SC ATS Stopped No ATS Running Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when a #SC_STOP_ATS_CC command was issued
 *  but there was no ATS running
 */
#define SC_STOPATS_NO_ATS_INF_EID 28

/**
 * \brief SC ATS All Commands Skipped Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an ATS is begun, and all times for
 *  the commands in the ATS exist in the past.
 */
#define SC_ATS_SKP_ALL_ERR_EID 29

/**
 * \brief SC ATS Some Commands Skipped Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *  This event message is issued when an ATS is started, and some of the
 *  times for commands in the ATS exist in the past.
 */
#define SC_ATS_ERR_SKP_DBG_EID 30

/**
 * \brief SC ATS Switch Pending Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when a #SC_SWITCH_ATS_CC command is issued and
 *  the switch is scheduled
 */
#define SC_SWITCH_ATS_CMD_INF_EID 31

/**
 * \brief SC ATS Switch Command Rejected Destination ATS Not Loaded Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_SWITCH_ATS_CC command is issued, but the
 *  ATS to switch to is not loaded
 */
#define SC_SWITCH_ATS_CMD_NOT_LDED_ERR_EID 32

/**
 * \brief SC ATS Switch Command Rejected ATP Is Idle Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_SWITCH_ATS_CC command is issued, but there
 *  is not an ATS running to switch from
 */
#define SC_SWITCH_ATS_CMD_IDLE_ERR_EID 33

/**
 * \brief SC ATS Switch Completed Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when an ATS switch is scheduled and has been switched
 */
#define SC_ATS_SERVICE_SWTCH_INF_EID 34

/**
 * \brief SC ATS Switch Scheduled Destination ATS Empty Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an ATS switch is scheduled, but there are no commands
 *  (ie not loaded) in the destination ATS
 */
#define SC_SERVICE_SWITCH_ATS_CMD_LDED_ERR_EID 35

/**
 * \brief SC ATS Switch Scheduled ATP Is Idle Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an ATS switch is scheduled, but there is no ATS running
 *  This error will only occur is something gets corrupted
 */
#define SC_ATS_SERVICE_SWITCH_IDLE_ERR_EID 36

/**
 * \brief SC ATS Inline Switch Completed Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when an ATS is scheduled in-line and the switch is successful
 */
#define SC_ATS_INLINE_SWTCH_INF_EID 37

/**
 * \brief SC ATS Inline Switch Destination ATS Not Loaded Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an ATS switch is scheduled, but there are no commands
 *  (ie not loaded) in the destination ATS
 */
#define SC_ATS_INLINE_SWTCH_NOT_LDED_ERR_EID 38

/**
 * \brief SC ATS Jump Command All Commands Skipped Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_JUMP_ATS_CC command was issued, and the time to
 *  jump to was passed all of the commands in the ATS
 */
#define SC_JUMPATS_CMD_STOPPED_ERR_EID 39

/**
 * \brief SC ATS Jump Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when a #SC_JUMP_ATS_CC command was executed successfully
 */
#define SC_JUMP_ATS_INF_EID 40

/**
 * \brief SC ATS Jump Command No Active ATS Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_JUMP_ATS_CC command was received, but there
 *  is no ATS currently running
 */
#define SC_JUMPATS_CMD_NOT_ACT_ERR_EID 41

/**
 * \brief SC Continue ATS On Failure Commanded State Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_CONTINUE_ATS_ON_FAILURE_CC command was received, but the state
 *  in the command was invalid
 */
#define SC_CONT_CMD_ERR_EID 42

/**
 * \brief SC Continue ATS On Failure Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when the #SC_CONTINUE_ATS_ON_FAILURE_CC command was received and
 *  the state was changed successfully
 */
#define SC_CONT_CMD_INF_EID 43

/**
 * \brief SC ATS Command Skipped Checksum Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a command from an ATS is about to be sent out
 *  but it fails checksum validation
 */
#define SC_ATS_CHKSUM_ERR_EID 44

/**
 * \brief SC ATS Aborted Command Checksum Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an ATS command that was about to be sent out
 *  failed checksum validation, and the Continue-ATS-on_checksum-Failure flag was
 *  set to 'FALSE'
 */
#define SC_ATS_ABT_ERR_EID 45

/**
 * \brief SC ATS Transmit Message Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an ATS command is about to be sent out,
 *  and the #CFE_SB_TransmitMsg call failed to send it
 */
#define SC_ATS_DIST_ERR_EID 46

/**
 * \brief SC ATS Command Skipped Due To Number Mismatch Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an ATS command is about to be sent out,
 *  but it's command number is not what was expected
 */
#define SC_ATS_MSMTCH_ERR_EID 47

/**
 * \brief SC ATS Command Skipped Due To Command Status Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an ATS command is about to be send out,
 *  but the command isn't marked as '#SC_Status_LOADED'
 */
#define SC_ATS_SKP_ERR_EID 48

/**
 * \brief SC RTS Transmit Message Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an RTS command was about to be sent out,
 *  and #CFE_SB_TransmitMsg couldn't send the message
 */
#define SC_RTS_DIST_ERR_EID 49

/**
 * \brief SC RTS Stopped Due To Command Checksum Failure Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an RTS command was about to be sent out,
 *  but the command failed checksum validation
 */
#define SC_RTS_CHKSUM_ERR_EID 50

/**
 * \brief SC Reset Counters Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when the #SC_RESET_COUNTERS_CC command was
 * received
 */
#define SC_RESET_INF_EID 51

/**
 * \brief SC No-op Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a #SC_NOOP_CC command has been received
 */
#define SC_NOOP_INF_EID 52

/**
 * \brief SC RTS Table Command Message ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when and RTS table is loaded, but there is an invalid
 *  message Id in the command
 */
#define SC_RTS_INVLD_MID_ERR_EID 59

/**
 * \brief SC RTS Table Command Message Length Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when and RTS table is loaded, but there is an invalid
 *  length in the command
 */
#define SC_RTS_LEN_ERR_EID 60

/**
 * \brief SC RTS Table Command Runs Off Buffer Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when and RTS table is loaded, but the command
 *  runs off the end of the buffer
 */
#define SC_RTS_LEN_BUFFER_ERR_EID 61

/**
 * \brief SC RTS Table Data Detected That Would Exceed Buffer Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when and RTS table is loaded, but there
 *  is non-zero data at the end when the size left is less than #SC_RTS_HDR_WORDS
 */
#define SC_RTS_LEN_TOO_LONG_ERR_EID 62

/**
 * \brief SC Command Pipe Message ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid message Id is received in the
 *  command pipe
 */
#define SC_MID_ERR_EID 63

/**
 * \brief SC Command Code Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid command code was received in
 *  the command pipe
 */
#define SC_CC_ERR_EID 64

/**
 * \brief SC RTS Info Table Get Address Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when getting the address of the Rts Info table
 *  failed
 */
#define SC_GET_ADDRESS_RTS_INFO_ERR_EID 65

/**
 * \brief SC RTS Control Block Table Get Address Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when getting the address of the Rts Control Block table
 *  failed
 */
#define SC_GET_ADDRESS_RTS_CTRL_BLCK_ERR_EID 66

/**
 * \brief SC ATS Info Table Get Address Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when getting the address of the Ats Info table
 *  failed
 */
#define SC_GET_ADDRESS_ATS_INFO_ERR_EID 67

/**
 * \brief SC ATS Control Block Table Get Address Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when getting the address of the Ats Control Block table
 *  failed
 */
#define SC_GET_ADDRESS_ATS_CTRL_BLCK_ERR_EID 68

/**
 * \brief SC ATS Command Status Table Get Address Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when getting the address of an Ats Command Status table
 *  failed
 */
#define SC_GET_ADDRESS_ATS_CMD_STAT_ERR_EID 69

/**
 * \brief SC RTS Table Get Address Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when getting the address of an RTS table
 *  failed
 */
#define SC_GET_ADDRESS_RTS_ERR_EID 70

/**
 * \brief SC ATS Table Get Address Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when getting the address of an ATS table
 *  failed
 */
#define SC_GET_ADDRESS_ATS_ERR_EID 71

/**
 * \brief SC RTS Start Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *  This event message is issued when a #SC_START_RTS_CC cmd is received and is successful
 */
#define SC_STARTRTS_CMD_DBG_EID 72

/**
 * \brief SC RTS Started Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when an RTS is started successfully
 */
#define SC_RTS_START_INF_EID 73

/**
 * \brief SC Start RTS Rejected Due To First Command Length Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an RTS is started but the first command has an invalid length
 */
#define SC_STARTRTS_CMD_INVLD_LEN_ERR_EID 74

/**
 * \brief SC Start RTS Rejected Due To RTS Not Loaded Or In Use Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an RTS is tried to be started, but the RTS is not
 *  marked as #SC_Status_LOADED
 */
#define SC_STARTRTS_CMD_NOT_LDED_ERR_EID 75

/**
 * \brief SC Start RTS Rejected RTS Disabled Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_START_RTS_CC command was received, but the
 *  RTS is disabled
 */
#define SC_STARTRTS_CMD_DISABLED_ERR_EID 76

/**
 * \brief SC Start RTS Rejected RTS ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_START_RTS_CC command was received, but the
 *  RTS Id was invalid
 */
#define SC_STARTRTS_CMD_INVALID_ERR_EID 77

/**
 * \brief SC Stop RTS Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when an #SC_STOP_RTS_CC command is received and exexuted successfully
 */
#define SC_STOPRTS_CMD_INF_EID 78

/**
 * \brief SC Stop RTS Rejected RTS ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_STOP_RTS_CC command was rejected because the
 *  RTS Id given was invalid
 */
#define SC_STOPRTS_CMD_ERR_EID 79

/**
 * \brief SC Disable RTS Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when a #SC_DISABLE_RTS_CC command was received,
 * and executed successfully
 */
#define SC_DISABLE_RTS_INF_EID 80

/**
 * \brief SC Disable RTS Rejected RTS ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_DISABLE_RTS_CC command was received,
 *  but the RTS Id given was invalid
 */
#define SC_DISRTS_CMD_ERR_EID 81

/**
 * \brief SC Enable RTS Command ID Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when a #SC_ENABLE_RTS_CC command was received,
 * and executed successfully
 */
#define SC_ENABLE_RTS_INF_EID 82

/**
 * \brief SC Enable RTS Rejected RTS ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_DISABLE_RTS_CC command was received,
 *  but the RTS Id given was invalid
 */
#define SC_ENARTS_CMD_ERR_EID 83

/**
 * \brief SC RTS Aborted Due To Command Exceeding Buffer Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when trying to get the next RTS command to execute,
 *  and that command runs passed the end of the RTS table buffer
 */
#define SC_RTS_LNGTH_ERR_EID 84

/**
 * \brief SC RTS Aborted Due To Command Length Field Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when trying to get the next RTS command to execute,
 *  and that command has an illegal length value in the command
 */
#define SC_RTS_CMD_LNGTH_ERR_EID 85

/**
 * \brief SC RTS Execution Completed Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when an RTS completes execution
 */
#define SC_RTS_COMPL_INF_EID 86

/**
 * \brief SC ATS Execution Completed Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event message is issued when an ATS completes execution
 */
#define SC_ATS_COMPL_INF_EID 87

/**
 * \brief SC Jump Command Skipped ATS Commands Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *  This event message is issued a Jump Command is issued and
 *  Some of the ATS commands were marked as skipped
 */
#define SC_JUMP_ATS_SKIPPED_DBG_EID 88

/**
 * \brief SC Append ATS Table Get Address Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message indicates a failure to get the table data address from
 *  cFE Table Services
 */
#define SC_GET_ADDRESS_APPEND_ERR_EID 92

/**
 * \brief SC Append ATS Table Registration Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message indicates a failure to register the table with cFE Table Services
 */
#define SC_REGISTER_APPEND_TBL_ERR_EID 93

/**
 * \brief SC Append ATS Table Updated Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *  This event message signals that the Append ATS table has been updated.
 */
#define SC_UPDATE_APPEND_EID 97

/**
 * \brief SC Append ATS Table Appended Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *  This event signals the successful completion of an Append ATS command.
 */
#define SC_APPEND_CMD_INF_EID 98

/**
 * \brief SC Append ATS Rejected ATS ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_APPEND_ATS_CC command has failed
 *  because the specified ATS command argument was not a valid ATS ID.
 */
#define SC_APPEND_CMD_ARG_ERR_EID 99

/**
 * \brief SC Append ATS Rejected Target ATS Table Empty Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_APPEND_ATS_CC command has failed
 *  because the specified target ATS table is empty.  The Append ATS command
 *  requires that both the source Append table and the target ATS table have
 *  valid contents.
 */
#define SC_APPEND_CMD_TGT_ERR_EID 100

/**
 * \brief SC Append ATS Rejected Append Table Empty Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_APPEND_ATS_CC command has failed
 *  because the source Append table is empty.  The Append ATS command requires
 *  that both the source Append table and the target ATS table have valid
 *  contents.
 */
#define SC_APPEND_CMD_SRC_ERR_EID 101

/**
 * \brief SC Append ATS Rejected Insufficent Space Left In ATS Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_APPEND_ATS_CC command has failed
 *  because there is not room in the specified target ATS table to add the
 *  contents of the Append table.
 */
#define SC_APPEND_CMD_FIT_ERR_EID 102

/**
 * \brief SC ATS Or Append Table Verification Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *  This event message signals the successful verification of an ATS or an
 *     Append ATS table.
 */
#define SC_VERIFY_ATS_EID 103

/**
 * \brief SC ATS Or Append Table Verification Table Entry Command Number Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message indicates an error during verification of an ATS
 *  or an Append ATS table due to an invalid table entry. The cause is
 *  a table entry with an invalid command number.
 */
#define SC_VERIFY_ATS_NUM_ERR_EID 104

/**
 * \brief SC ATS Or Append Table Data Detected That Would Exceed Buffer Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message indicates an error during verification of an ATS
 *  or an Append ATS table due to an invalid table entry. The cause is
 *  a table entry that has a valid command number but the entry begins
 *  at an offset too near the end of the table buffer to provide room
 *  for even the smallest possible command packet.
 *
 *  This error can be corrected by setting the first data word which
 *  follows the last valid table entry to zero. Note that the first
 *  word in an ATS table entry is the entry command number.
 */
#define SC_VERIFY_ATS_END_ERR_EID 105

/**
 * \brief SC ATS Or Append Table Verification Command Length Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message indicates an error during verification of an ATS
 *  or an Append ATS table due to an invalid table entry. The cause is
 *  a table entry with an invalid command packet length.
 */
#define SC_VERIFY_ATS_PKT_ERR_EID 106

/**
 * \brief SC ATS Or Append Table Verification Command Length Exceeds Buffer Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message indicates an error during verification of an ATS
 *  or an Append ATS table due to an invalid table entry. The cause is
 *  a table entry with an otherwise valid command packet length that
 *  would extend past the end of the table buffer.
 */
#define SC_VERIFY_ATS_BUF_ERR_EID 107

/**
 * \brief SC ATS Or Append Table Verification Duplicate Command Number Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message indicates an error during verification of an ATS
 *  or an Append ATS table due to an invalid table entry. The cause is
 *  a table entry with an otherwise valid command number that is already
 *  in use by an earlier table entry.
 */
#define SC_VERIFY_ATS_DUP_ERR_EID 109

/**
 * \brief SC ATS Or Append Table Verification Table Empty Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message indicates an error during verification of an ATS
 *  or an Append ATS table due to the table having no entries.  This
 *  error can only occur if the first entry in the table has a command
 *  number equal to zero - the end of data marker.
 */
#define SC_VERIFY_ATS_MPT_ERR_EID 110

/**
 * \brief SC Table Manage Request Table ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued upon receipt of a table manage
 *  request command that has an invalid table ID argument.
 */
#define SC_TABLE_MANAGE_ID_ERR_EID 111

/**
 * \brief SC RTS Table Manage Processing Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  The expectation is that this command is sent by cFE Table
 *  Services only when the indicated RTS table has been updated.
 *  Thus, after allowing cFE Table Services an opportunity to
 *  manage the table, the call to re-acquire the table data
 *  pointer is expected to return an indication that the table
 *  data has been updated.  This event message is issued upon
 *  receipt of any other function result.
 */
#define SC_TABLE_MANAGE_RTS_ERR_EID 112

/**
 * \brief SC ATS Table Manage Processing Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  The expectation is that this command is sent by cFE Table
 *  Services only when the indicated ATS table has been updated.
 *  Thus, after allowing cFE Table Services an opportunity to
 *  manage the table, the call to re-acquire the table data
 *  pointer is expected to return an indication that the table
 *  data has been updated.  This event message is issued upon
 *  receipt of any other function result.
 */
#define SC_TABLE_MANAGE_ATS_ERR_EID 113

/**
 * \brief SC ATS Append Table Manage Processing Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  The expectation is that this command is sent by cFE Table
 *  Services only when the ATS Append table has been updated.
 *  Thus, after allowing cFE Table Services an opportunity to
 *  manage the table, the call to re-acquire the table data
 *  pointer is expected to return an indication that the table
 *  data has been updated.  This event message is issued upon
 *  receipt of any other function result.
 */
#define SC_TABLE_MANAGE_APPEND_ERR_EID 114

/**
 * \brief SC Start RTS Group Command Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *  This event message is issued following the successful execution of
 *  a #SC_START_RTS_GRP_CC command.
 */
#define SC_STARTRTSGRP_CMD_INF_EID 115

/**
 * \brief SC Start RTS Group RTS ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_START_RTS_GRP_CC command was
 *  rejected because the RTS group definition was invalid:
 *  - First RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *  - Last RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *  - Last RTS ID must be greater than or equal to First RTS ID
 */
#define SC_STARTRTSGRP_CMD_ERR_EID 116

/**
 * \brief SC Stop RTS Group Command Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *  This event message is issued following the successful execution of
 *  a #SC_STOP_RTS_GRP_CC command.
 */
#define SC_STOPRTSGRP_CMD_INF_EID 117

/**
 * \brief SC Stop RTS Group RTS ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_STOP_RTS_GRP_CC command was
 *  rejected because the RTS group definition was invalid:
 *  - First RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *  - Last RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *  - Last RTS ID must be greater than or equal to First RTS ID
 */
#define SC_STOPRTSGRP_CMD_ERR_EID 118

/**
 * \brief SC Disable RTS Group Command Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *  This event message is issued following the successful execution of
 *  a #SC_DISABLE_RTS_GRP_CC command.
 */
#define SC_DISRTSGRP_CMD_INF_EID 119

/**
 * \brief SC Disable RTS Group RTS ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_DISABLE_RTS_GRP_CC command was
 *  rejected because the RTS group definition was invalid:
 *  - First RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *  - Last RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *  - Last RTS ID must be greater than or equal to First RTS ID
 */
#define SC_DISRTSGRP_CMD_ERR_EID 120

/**
 * \brief SC Enable RTS Group Command Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *  This event message is issued following the successful execution of
 *  a #SC_ENABLE_RTS_GRP_CC command.
 */
#define SC_ENARTSGRP_CMD_INF_EID 121

/**
 * \brief SC Enable RTS Group RTS ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_ENABLE_RTS_GRP_CC command was
 *  rejected because the RTS group definition was invalid:
 *  - First RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *  - Last RTS ID must be 1 through #SC_NUMBER_OF_RTS
 *  - Last RTS ID must be greater than or equal to First RTS ID
 */
#define SC_ENARTSGRP_CMD_ERR_EID 122

/**
 * \brief SC Jump Command Entry Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *  This event message is issued when a #SC_JUMP_ATS_CC command was issued, and the time to
 *  jump to was less than or equal to a list entry.
 */
#define SC_JUMPATS_CMD_LIST_INF_EID 123

/**
 * \brief SC RTS Table Load Failed Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *  This event message is issued when an RTS table fails to load at startup.
 *
 *  The event text includes the RTS index and the result of the
 *  #CFE_TBL_Load call.
 */
#define SC_RTS_LOAD_FAIL_DBG_EID 124

/**
 * \brief SC Start RTS Group RTS Not Loaded Or In Use Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_START_RTS_GRP_CC command was received, but an
 *  RTS is marked as #SC_Status_LOADED
 */
#define SC_STARTRTSGRP_CMD_NOT_LDED_ERR_EID 126

/**
 * \brief SC Start RTS Group RTS Disabled Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when a #SC_START_RTS_GRP_CC command was received, but an
 *  RTS is disabled
 */
#define SC_STARTRTSGRP_CMD_DISABLED_ERR_EID 127

/**
 * \brief SC Begin ATS ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid ATS index is received
 *  in the SC_BeginAts function
 */
#define SC_BEGINATS_INVLD_INDEX_ERR_EID 128

/**
 * \brief SC RTS Table Manage RTS Index Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid RTS index is received
 *  in the SC_ManageRtsTable function
 */
#define SC_TABLE_MANAGE_RTS_INV_INDEX_ERR_EID 129

/**
 * \brief SC ATS Table Manage ATS Index Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid ATS index is received
 *  in the SC_ManageAtsTable function
 */
#define SC_TABLE_MANAGE_ATS_INV_INDEX_ERR_EID 130

/**
 * \brief SC ATS Table Load ATS Index Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid ATS index is received
 *  in the SC_LoadAts function
 */
#define SC_LOADATS_INV_INDEX_ERR_EID 131

/**
 * \brief SC ATS Build Time Index ATS Index Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid ATS index is received
 *  in the SC_BuildTimeIndexTable function
 */
#define SC_BUILD_TIME_IDXTBL_ERR_EID 132

/**
 * \brief SC ATS Insert ATS Index Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid ATS index is received
 *  in the SC_Insert function
 */
#define SC_INSERTATS_INV_INDEX_ERR_EID 133

/**
 * \brief SC ATS Table Initialization Error ATS Index Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid ATS index is received
 *  in the SC_InitAtsTables function
 */
#define SC_INIT_ATSTBL_INV_INDEX_ERR_EID 134

/**
 * \brief SC RTS Table Initialization Error RTS Index Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid RTS index is received
 *  in the SC_LoadRts function
 */
#define SC_LOADRTS_INV_INDEX_ERR_EID 135

/**
 * \brief SC ATS Process Append ATS Index Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid ATS index is received
 *  in the SC_ProcessAppend function
 */
#define SC_PROCESS_APPEND_INV_INDEX_ERR_EID 136

/**
 * \brief SC RTS Kill RTS Index Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid RTS index is received
 *  in the SC_KillRts function
 */
#define SC_KILLRTS_INV_INDEX_ERR_EID 137

/**
 * \brief SC RTS Autostart RTS ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *  This event message is issued when an invalid RTS ID is received
 *  in the SC_AutoStartRts function
 */
#define SC_AUTOSTART_RTS_INV_ID_ERR_EID 138

/**\}*/

#endif
