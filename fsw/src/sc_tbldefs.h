/*************************************************************************
** File: sc_tbldefs.h 
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
**   Specification for the CFS Stored Command (SC) table structures
**
*************************************************************************/
#ifndef _sc_tbldefs_
#define _sc_tbldefs_

/*************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "sc_platform_cfg.h"

/**
**  \brief ID definitions for cFE Table Services manage table request command
*/
#define SC_TBL_ID_ATS_0     (1)
#define SC_TBL_ID_APPEND    (SC_TBL_ID_ATS_0 + SC_NUMBER_OF_ATS)
#define SC_TBL_ID_RTS_0     (SC_TBL_ID_APPEND + 1)
#define SC_TBL_ID_RTS_INFO  (SC_TBL_ID_RTS_0 + SC_NUMBER_OF_RTS)
#define SC_TBL_ID_RTP_CTRL  (SC_TBL_ID_RTS_INFO + 1)
#define SC_TBL_ID_ATS_INFO  (SC_TBL_ID_RTP_CTRL + 1)
#define SC_TBL_ID_APP_INFO  (SC_TBL_ID_ATS_INFO + 1)
#define SC_TBL_ID_ATP_CTRL  (SC_TBL_ID_APP_INFO + 1)
#define SC_TBL_ID_ATS_CMD_0 (SC_TBL_ID_ATP_CTRL + 1)

/************************************************************************
** Type Definitions
*************************************************************************/

/**
**  \brief Absolute Value time tag for ATC's
*/
typedef uint32 SC_AbsTimeTag_t;

/**
**  \brief Relative time tag for RTC's
*/
typedef uint32 SC_RelTimeTag_t;

/**
**  \brief ATS Info Table Type - One of these records are kept for each ATS
*/
typedef struct
{

    uint16 AtsUseCtr;        /* How many times it has been used */
    uint16 NumberOfCommands; /* number of commands in the ATS */
    uint32 AtsSize;          /* size of the ATS */

} SC_AtsInfoTable_t;

/**
**  \brief ATP Control Block Type
*/
typedef struct
{

    uint8  AtpState;       /* execution state of the ATP */
    uint8  AtsNumber;      /* current ATS running if any */
    uint32 CmdNumber;      /* current cmd number to run if any */
    uint16 TimeIndexPtr;   /* time index pointer for current cmd */
    uint16 SwitchPendFlag; /* indicates that a buffer switch is waiting */

} SC_AtpControlBlock_t;

/**
**  \brief RTP Control Block Type
   Note: now there is only really one RTP
   This structure contains overall info for the next relative time
   processor.
*/
typedef struct
{

    uint16 NumRtsActive; /* number of RTSs currently active */
    uint16 RtsNumber;    /* next RTS number */

} SC_RtpControlBlock_t;

/**
**  \brief RTS info table entry type -One of these records is kept for each RTS
*/
typedef struct
{

    uint8           RtsStatus;       /* status of the RTS */
    bool            DisabledFlag;    /* disabled/enabled flag */
    uint8           CmdCtr;          /* Cmds executed in current rts */
    uint8           CmdErrCtr;       /* errs in current RTS */
    SC_AbsTimeTag_t NextCommandTime; /* next command time for RTS */
    uint16          NextCommandPtr;  /* where next rts cmd is */
    uint16          UseCtr;          /* how many times RTS is run */

} SC_RtsInfoEntry_t;

#endif /*_sc_tbldefs_*/

/************************/
/*  End of File Comment */
/************************/
