/*************************************************************************
 ** File: sc_app.h 
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
 **     This file contains the Stored Command main event loop header
 **
 ** References:
 **   Flight Software Branch C Coding Standard Version 1.2
 **   CFS Development Standards Document
 **
 *************************************************************************/

#ifndef _sc_app_
#define _sc_app_

/*************************************************************************
** Includes
*************************************************************************/

#include "cfe.h"
#include "sc_platform_cfg.h"
#include "sc_tbldefs.h"
#include "sc_msgdefs.h"
#include "sc_msg.h"

/**
** \brief Wakeup for SC
**
** \par Description
**      Wakes up SC every 1 second for routine maintenance whether a
**      message was received or not.
*/
#define SC_SB_TIMEOUT 1000


/************************************************************************/
/** \brief Main loop for SC
 **
 **  \par Description
 **       This function is the entry point and main loop for the Stored
 **       Commands (SC) application.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 *************************************************************************/
void SC_AppMain(void);

/************************************************************************/
/** \brief Initialize application
 **
 **  \par Description
 **       This function initializes the SC application. The return value
 **       is either CFE_SUCCESS or the error code from the failed cFE
 **       function call. Note that all errors generate an identifying
 **       event message.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS \retdesc \copydoc CFE_SUCCESS \endcode
 **  \retstmt Return status from CFE initialization function \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_AppInit(void);

/************************************************************************/
/** \brief Initialize application tables
 **
 **  \par Description
 **       This function initializes the SC application tables. The
 **       return value is either CFE_SUCCESS or the error code from the
 **       failed cFE function call. Note that all errors generate an
 **       identifying event message.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS \retdesc \copydoc CFE_SUCCESS \endcode
 **  \retstmt Return status from CFE initialization function \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_InitTables(void);

/************************************************************************/
/** \brief Register tables with cFE Table Services
 **
 **  \par Description
 **       This function registers all SC tables with cFE Table Services.
 **       The return value is either CFE_SUCCESS or the error code from
 **       the failed cFE function call. Note that all errors generate an
 **       identifying event message.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS \retdesc \copydoc CFE_SUCCESS \endcode
 **  \retstmt Return status from CFE initialization function \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_RegisterAllTables(void);

/************************************************************************/
/** \brief Get dump only table buffer pointers
 **
 **  \par Description
 **       This function acquires buffer pointers to the dump only tables.
 **       The return value is either CFE_SUCCESS or the error code from
 **       the failed cFE function call. Note that all errors generate an
 **       identifying event message.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS \retdesc \copydoc CFE_SUCCESS \endcode
 **  \retstmt Return status from CFE initialization function \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_GetDumpTablePointers(void);

/************************************************************************/
/** \brief Get loadable table buffer pointers
 **
 **  \par Description
 **       This function acquires buffer pointers to the loadable tables.
 **       The return value is either CFE_SUCCESS or the error code from
 **       the failed cFE function call. Note that all errors generate an
 **       identifying event message.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS \retdesc \copydoc CFE_SUCCESS \endcode
 **  \retstmt Return status from CFE initialization function \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_GetLoadTablePointers(void);

/************************************************************************/
/** \brief Load default RTS tables
 **
 **  \par Description
 **       This function loads the default RTS tables. The return value
 **       is either CFE_SUCCESS or the error code from the failed cFE
 **       function call. Note that all errors generate an identifying
 **       event message.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS \retdesc \copydoc CFE_SUCCESS \endcode
 **  \retstmt Return status from CFE initialization function \endcode
 **  \endreturns
 **
 *************************************************************************/
void SC_LoadDefaultTables(void);

/************************************************************************/
/** \brief Register to receive cFE Table Services manage request commands
 **
 **  \par Description
 **       This function provides cFE Table Services with the information
 **       necessary to send a notification command when one of the SC dump
 **       only tables has a dump pending, or when one of the SC loadable
 **       tables has a load pending.  Upon receipt of the command, the
 **       command handler will call the cFE Table Services API function
 **       to manage the table.  This sequence of events ensures that dump
 **       tables are not being updated by SC at the same moment that the
 **       dump occurs, and likewise, that loadable tables are not being
 **       referenced by SC at the moment that the update occurs.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]         (none)
 **
 **  \sa #SC_TableManageCmd
 **
 *************************************************************************/
void SC_RegisterManageCmds(void);

/************************************************************************
** Macro Definitions
*************************************************************************/

/**
** \name SC number of bytes in a word */
/** \{ */
#define SC_BYTES_IN_WORD 4 /**< \brief Words are used to define table lengths   */
/** \} */

/**
** \name Sizes of the headers for the RTC's and ATC's */
/** \{ */
#define SC_ATS_HEADER_SIZE (sizeof(SC_AtsEntryHeader_t))
#define SC_RTS_HEADER_SIZE (sizeof(SC_RtsEntryHeader_t))
/** \} */

/**
** \name Buffer sizes in 32-bit words */
/** \{ */
#define SC_ATS_BUFF_SIZE32    (SC_ATS_BUFF_SIZE / 2)
#define SC_RTS_BUFF_SIZE32    (SC_RTS_BUFF_SIZE / 2)
#define SC_APPEND_BUFF_SIZE32 (SC_APPEND_BUFF_SIZE / 2)
/** \} */

/**
** \name SC error return value */
/** \{ */
#define SC_ERROR -1
/** \} */

/**
** \name  Cmd pipe information*/
/** \{ */
#define SC_CMD_PIPE_NAME "SC_CMD_PIPE"
/** \} */

#define SC_ATS_HDR_WORDS       ((SC_ATS_HEADER_SIZE + SC_PACKET_MIN_SIZE) / SC_BYTES_IN_WORD)
#define SC_ATS_HDR_NOPKT_WORDS (SC_ATS_HEADER_SIZE / SC_BYTES_IN_WORD)

#define SC_RTS_HDR_WORDS       ((SC_RTS_HEADER_SIZE + SC_PACKET_MIN_SIZE) / SC_BYTES_IN_WORD)
#define SC_RTS_HDR_NOPKT_WORDS (SC_RTS_HEADER_SIZE / SC_BYTES_IN_WORD)

#define SC_DUP_TEST_UNUSED -1

/* used for ATS table initialization */
#define SC_INVALID_CMD_NUMBER 0xFFFF

/* used to round byte len up to word len */
#define SC_ROUND_UP_BYTES 3

/* count of bytes in an ATS append table entry */
#define SC_BYTES_IN_ATS_APPEND_ENTRY 2

/* convert an ATS command number to a table index */
#define SC_ATS_CMD_NUM_TO_INDEX(num) ((num)-1)

/* convert an ATS command table index to an ID number */
#define SC_ATS_CMD_INDEX_TO_NUM(index) ((index) + 1)

/* convert an ATS ID number to a table index */
#define SC_ATS_NUM_TO_INDEX(num) ((num)-1)

/* convert an ATS table index to an ID number */
#define SC_ATS_INDEX_TO_NUM(index) ((index) + 1)

/* convert an ATS ID to a table index */
#define SC_ATS_ID_TO_INDEX(id) ((id)-1)

/* convert an RTS number to a table index */
#define SC_RTS_NUM_TO_INDEX(num) ((num)-1)

/* convert an RTS table index to a number */
#define SC_RTS_INDEX_TO_NUM(index) ((index) + 1)

/* convert an RTS ID to a table index */
#define SC_RTS_ID_TO_INDEX(id) ((id)-1)

/* convert an RTS table index to an ID number */
#define SC_RTS_INDEX_TO_ID(index) ((index)+1)

/*********************************************************************************************/

/**
**  \brief ATS Table Entry Header Type
*/
typedef struct
{
    /* Header needs to be 32 bit multiple */
    uint16 Pad;

    /* Command identifier, range = 1 to SC_MAX_ATS_CMDS */
    uint16 CmdNumber;

    /* 32 bit absolute time, stored as two 16 bit values */
    uint16 TimeTag_MS; /* Most significant 16 bits */
    uint16 TimeTag_LS; /* Least significant 16 bits */

    /*
    ** Note: the command packet data is variable length,
    **       the command packet header (not shown here),
    **       comes directly after Time tag 2.
    */

} SC_AtsEntryHeader_t;

typedef union
{
    SC_AtsEntryHeader_t Header;

    uint16 *Buffer;
} SC_AtsEntryHeaderBuf_t;

/**
**  \brief  RTS Command Header Type
*/
typedef struct
{

    /* Relative time tag*/
    SC_RelTimeTag_t TimeTag;

    /*
    ** Note: the command packet data is variable length,
    **       the command packet header (not shown here),
    **       comes directly after Time tag.
    */

} SC_RtsEntryHeader_t;

/*
 * This determines the number of 32-bit words required to contain the
 * SC_RtsEntryHeader_t structure.  This is used to size a temporary buffer
 * used when validating the RTS table.
 */
#define SC_RTS_HDR_32BIT_WORDS SC_RTS_HDR_WORDS

typedef union
{
    SC_RtsEntryHeader_t Header;

    uint16 *Buffer;
} SC_RtsEntryHeaderBuf_t;

/**
**  \brief SC Operational Data Structure
**  This structure contains addresses and handles for loadable and dump-only tables
**  along with storage for the housekeeping packet.
*/
typedef struct
{
    CFE_SB_PipeId_t CmdPipe; /**< \brief Command pipe ID                     */

    CFE_TBL_Handle_t AtsTblHandle[SC_NUMBER_OF_ATS]; /**< \brief Table handles for all ATS tables    */
    uint32 *         AtsTblAddr[SC_NUMBER_OF_ATS];   /**< \brief Table Addresses for all ATS tables  */

    CFE_TBL_Handle_t AppendTblHandle; /**< \brief Table handle for Append ATS table   */
    uint32 *         AppendTblAddr;   /**< \brief Table Address for Append ATS table  */

    CFE_TBL_Handle_t RtsTblHandle[SC_NUMBER_OF_RTS]; /**< \brief Table handles for all RTS tables    */
    uint32 *         RtsTblAddr[SC_NUMBER_OF_RTS];   /**< \brief Table addresses for all RTS tables  */

    CFE_TBL_Handle_t   AtsInfoHandle;  /**< \brief Table handle the for ATS Info Table */
    SC_AtsInfoTable_t *AtsInfoTblAddr; /**< \brief Table address for the ATS Info Table*/

    CFE_TBL_Handle_t   AppendInfoHandle;  /**< \brief Table handle for Append Info Table  */
    SC_AtsInfoTable_t *AppendInfoTblAddr; /**< \brief Table address for Append Info Table */

    CFE_TBL_Handle_t   RtsInfoHandle;  /**< \brief Table handle for RTS Info Table     */
    SC_RtsInfoEntry_t *RtsInfoTblAddr; /**< \brief Table address for RTS INfo Table    */

    CFE_TBL_Handle_t      RtsCtrlBlckHandle; /**< \brief Table handle for the RTP ctrl block */
    SC_RtpControlBlock_t *RtsCtrlBlckAddr;   /**< \brief Table address for the RTP ctrl block*/

    CFE_TBL_Handle_t      AtsCtrlBlckHandle; /**< \brief Table handle for the ATP ctrl block */
    SC_AtpControlBlock_t *AtsCtrlBlckAddr;   /**< \brief Table address for the ATP ctrl block*/

    CFE_TBL_Handle_t AtsCmdStatusHandle[SC_NUMBER_OF_ATS];  /**< \brief ATS Cmd Status table handle     */
    uint32 *         AtsCmdStatusTblAddr[SC_NUMBER_OF_ATS]; /**< \brief ATS Cmd Status table address    */

    int32 AtsDupTestArray[SC_MAX_ATS_CMDS]; /**< \brief ATS test for duplicate cmd numbers  */

    uint16 NumCmdsSec;   /**< \brief the num of cmds that have gone out
                               in a one second period                    */
    SC_HkTlm_t HkPacket; /**< \brief SC Housekeeping structure           */

} SC_OperData_t;

/**
**  \brief SC Application Data Structure
**  This structure is used by the application to process time ordered commands.
*/
typedef struct
{
    uint16 AtsTimeIndexBuffer[SC_NUMBER_OF_ATS][SC_MAX_ATS_CMDS];
    /**< \brief  This table is used to keep a time ordered listing
         of ATS commands. The index used is the ATS command number. The first
         in this table holds the command number of the command that will execute
         first, the second entry has the number of the 2nd cmd, etc.. */

    int32 AtsCmdIndexBuffer[SC_NUMBER_OF_ATS][SC_MAX_ATS_CMDS];
    /**< \brief  This table is used to keep a list of ATS table command offsets.
         These offsets correspond to the addresses of ATS commands located in the ATS table.
         The index used is the ATS command index with values from 0 to SC_MAX_ATS_CMDS-1 */

    uint8           NextProcNumber;  /**< \brief the next command processor number */
    SC_AbsTimeTag_t NextCmdTime[2];  /**< \brief The overall next command time  0 - ATP, 1- RTP*/
    SC_AbsTimeTag_t CurrentTime;     /**< \brief this is the current time for SC */
    uint16          Unused;          /**< \brief Unused */
    uint16          AutoStartRTS;    /**< \brief Start selected auto-exec RTS after init */
    uint16          AppendWordCount; /**< \brief Size of cmd entries in current Append ATS table */
} SC_AppData_t;

/************************************************************************
** Exported Data
*************************************************************************/
extern SC_AppData_t  SC_AppData;
extern SC_OperData_t SC_OperData;

#endif /* _sc_app_ */

/************************/
/*  End of File Comment */
/************************/
