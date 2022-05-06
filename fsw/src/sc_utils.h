/*************************************************************************
** File: sc_utils.h 
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
**   This file contains the utilty functions for Stored Command
**
** References:
**   Flight Software Branch C Coding Standard Version 1.2
**   CFS Development Standards Document
**
*************************************************************************/
#ifndef _sc_utils_
#define _sc_utils_

#include "cfe.h"
#include "sc_app.h"

/************************************************************************/
/** \brief Gets the current time from CFE
 **
 **  \par Description
 **       Queries the CFE TIME services and retieves the Current time
 **
 **
 **  \par Assumptions, External Events, and Notes:
 **        This routine stores the time in #SC_AppData
 **
 **
 *************************************************************************/
void SC_GetCurrentTime(void);

/************************************************************************/
/** \brief Gets the absolute time from an ATS entry
 **
 **  \par Description
 **       This function returns the absolute time tag contained within
 **       the ATS entry passed into the function
 **
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]        Entry         A #SC_AtsEntryHeader_t* pointer to
 **                                   the ATS entry
 **
 **  \returns
 **  \retstmt Returns the absolute time tag  \endcode
 **  \endreturns
 **
 *************************************************************************/
SC_AbsTimeTag_t SC_GetAtsEntryTime(SC_AtsEntryHeader_t *Entry);

/************************************************************************/
/** \brief Computes an absolute time from relative time
 **
 **  \par Description
 **       This function computes an absolute time from 'now' and the
 **       relative time passed into the function
 **
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]        RelTime         The relative time to compute from
 **
 **  \returns
 **  \retstmt Returns the computed absolute time   \endcode
 **  \endreturns
 **
 *************************************************************************/
SC_AbsTimeTag_t SC_ComputeAbsTime(uint16 RelTime);

/************************************************************************/
/** \brief Compares absolute time
 **
 **  \par Description
 **
 **       This function compares two absolutes time.
 **
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]    AbsTime1            The first time to compare
 **
 **  \param [in]    AbsTime2            The second time to compare
 **
 **  \returns
 **  \retstmt Returns true  if AbsTime1 is greater than AbsTime2 \endcode
 **  \retstmt Returns false  if AbsTime1 is less than AbsTime2 \endcode
 **  \endreturns
 **
 *************************************************************************/
bool SC_CompareAbsTime(SC_AbsTimeTag_t AbsTime1, SC_AbsTimeTag_t AbsTime2);

/************************************************************************/
/** \brief Verify command message length
 **
 **  \par Description
 **       This routine will check if the actual length of a software bus
 **       command message matches the expected length and send an
 **       error event message if a mismatch occurs
 **
 **  \par Assumptions, External Events, and Notes:
 **       None
 **
 **  \param [in]   Msg              A #CFE_SB_Buffer_t* pointer that
 **                                 references the software bus message
 **
 **  \param [in]   ExpectedLength   The expected length of the message
 **                                 based upon the command code
 **
 **  \returns
 **  \retstmt Returns true if the length is as expected      \endcode
 **  \retstmt Returns false if the length is not as expected \endcode
 **  \endreturns
 **
 **  \sa #SC_LEN_ERR_EID
 **
 *************************************************************************/
bool SC_VerifyCmdLength(const CFE_MSG_Message_t *Msg, size_t ExpectedLength);

/************************************************************************/
/** \brief Toggles the ATS index
 **
 **  \par Description
 **       This function toggles the ATS index between 0 and 1.  This
 **       function does not modify global data, but rather returns a
 **       new ATS index that can be used and saved by the calling
 **       function.
 **
 **  \par Assumptions, External Events, and Notes:
 **       None
 **
 **  \returns
 **  \retstmt Returns 0 if the current ATS index is 1   \endcode
 **  \retstmt Returns 1 if the current ATS index is 0   \endcode
 **  \endreturns
 **
 **  \sa #SC_LEN_ERR_EID
 **
 *************************************************************************/
uint16 SC_ToggleAtsIndex(void);

#endif /*_sc_utils_*/

/************************/
/*  End of File Comment */
/************************/
