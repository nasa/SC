/*************************************************************************
** File: sc_rtsrq.h 
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
**     This file contains the headers to handle all of the RTS
**     executive requests and internal reuqests to control
**     the RTP and RTSs.
**
** References:
**   Flight Software Branch C Coding Standard Version 1.2
**   CFS Development Standards Document
**
*************************************************************************/

#ifndef _sc_rtsreq_
#define _sc_rtsreq_

#include "cfe.h"

/************************************************************************/
/** \brief Start an RTS Command
 **
 **  \par Description
 **             This routine starts the execution of an RTS.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]         CmdPacket      a #CFE_SB_Buffer_t* pointer that
 **                                     references a software bus message
 **
 **  \sa #SC_START_RTS_CC
 **
 *************************************************************************/
void SC_StartRtsCmd(const CFE_SB_Buffer_t *CmdPacket);

#if (SC_ENABLE_GROUP_COMMANDS == true)
/************************************************************************/
/** \brief Start a group of RTS Command
 **
 **  \par Description
 **             This routine starts the execution of a group of RTS.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]         CmdPacket      a #CFE_SB_Buffer_t* pointer that
 **                                     references a software bus message
 **
 **  \sa #SC_START_RTSGRP_CC
 **
 *************************************************************************/
void SC_StartRtsGrpCmd(const CFE_SB_Buffer_t *CmdPacket);
#endif

/************************************************************************/
/** \brief  Stop an RTS from executing Command
 **
 **  \par Description
 **             This routine stops the execution of an RTS.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]         CmdPacket      a #CFE_SB_Buffer_t* pointer that
 **                                     references a software bus message
 **
 **  \sa #SC_STOP_RTS_CC
 **
 ************************************************************************/
void SC_StopRtsCmd(const CFE_SB_Buffer_t *CmdPacket);

#if (SC_ENABLE_GROUP_COMMANDS == true)
/************************************************************************/
/** \brief  Stop a group of RTS from executing Command
 **
 **  \par Description
 **             This routine stops the execution of a group of RTS.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]         CmdPacket      a #CFE_SB_Buffer_t* pointer that
 **                                     references a software bus message
 **
 **  \sa #SC_STOP_RTS_CC
 **
 ************************************************************************/
void SC_StopRtsGrpCmd(const CFE_SB_Buffer_t *CmdPacket);
#endif

/************************************************************************/
/** \brief Disable an RTS Command
 **
 **  \par Description
 **             This routine disables an enabled RTS.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]         CmdPacket      a #CFE_SB_Buffer_t* pointer that
 **                                     references a software bus message
 **
 **  \sa #SC_DISABLE_RTS_CC
 **
 *************************************************************************/
void SC_DisableRtsCmd(const CFE_SB_Buffer_t *CmdPacket);

#if (SC_ENABLE_GROUP_COMMANDS == true)
/************************************************************************/
/** \brief Disable a group of RTS Command
 **
 **  \par Description
 **             This routine disables a group of enabled RTS.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]         CmdPacket      a #CFE_SB_Buffer_t* pointer that
 **                                     references a software bus message
 **
 **  \sa #SC_DISABLE_RTS_CC
 **
 *************************************************************************/
void SC_DisableRtsGrpCmd(const CFE_SB_Buffer_t *CmdPacket);
#endif

/************************************************************************/
/** \brief Enable an RTS Command
 **
 **  \par Description
 **             This routine enables a disabled RTS.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]         CmdPacket      a #CFE_SB_Buffer_t* pointer that
 **                                     references a software bus message
 **
 **  \sa #SC_ENABLE_RTS_CC
 **
 *************************************************************************/
void SC_EnableRtsCmd(const CFE_SB_Buffer_t *CmdPacket);

#if (SC_ENABLE_GROUP_COMMANDS == true)
/************************************************************************/
/** \brief Enable a group of RTS Command
 **
 **  \par Description
 **             This routine enables a group of disabled RTS.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]         CmdPacket      a #CFE_SB_Buffer_t* pointer that
 **                                     references a software bus message
 **
 **  \sa #SC_ENABLE_RTSGRP_CC
 **
 *************************************************************************/
void SC_EnableRtsGrpCmd(const CFE_SB_Buffer_t *CmdPacket);
#endif

/************************************************************************/
/** \brief Stops an RTS & clears out data
 **
 **  \par Description
 **      This is a generic routine to stop an RTS
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]         RtsIndex       RTS index to kill (base zero)
 **
 *************************************************************************/
void SC_KillRts(uint16 RtsIndex);

/************************************************************************/
/** \brief Automatically starts an RTS
 **
 **  \par Description
 **        This function sends a command back to the SC app to
 **        start the RTS designated as the auto-start RTS (usually 1)
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]         RtsNumber      RTS number to start (base one)
 **
 *************************************************************************/
void SC_AutoStartRts(uint16 RtsNumber);

#endif /* _sc_rtsreq_ */

/************************/
/*  End of File Comment */
/************************/
