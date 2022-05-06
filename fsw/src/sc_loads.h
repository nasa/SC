/*************************************************************************
** File: sc_loads.h 
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
**   This file contains functions to handle validation of TBL tables,
**   as well as setting up Stored Command's internal data structures for
**   those tables
**
** References:
**   Flight Software Branch C Coding Standard Version 1.2
**   CFS Development Standards Document
**
*************************************************************************/
#ifndef _sc_loads_
#define _sc_loads_

#include "cfe.h"

/************************************************************************/
/** \brief Parses an RTS to see if it is valid
 **
 **  \par Description
 **         This routine is called to validate an RTS buffer. It parses through
 **           the RTS to make sure all of the commands look in reasonable shape.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]    Buffer          A pointer to the area to validate
 **
 **  \returns
 **  \retstmt Returns true if the RTS was valid \endcode
 **  \retstmt Returns false if the RTS was NOT valid \endcode
 **  \endreturns
 **
 *************************************************************************/
bool SC_ParseRts(uint32 Buffer[]);

/************************************************************************/
/** \brief Buids the Time index buffer for the ATS
 **
 **  \par Description
 **            This routine builds the ATS Time Index Table after an ATS buffer
 **            has been loaded and the ATS Command Index Table has been built.
 **            This routine will take the commands that are pointed to by the
 **            pointers in the command index table and sort the commands by
 **            time order.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]    AtsIndex        ATS array index
 **
 **
 *************************************************************************/

void SC_BuildTimeIndexTable(uint16 AtsIndex);

/************************************************************************/
/** \brief Inserts an item in a sorted list
 **
 **  \par Description
 **            This function will insert a new element into the list of
 **            ATS commands sorted by execution time.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]    AtsIndex        ATS array index selection
 **
 **  \param [in]    NewCmdIndex     ATS command index for new list element
 **
 **  \param [in]    ListLength      Number of elements currently in list
 **
 **
 *************************************************************************/
void SC_Insert(uint16 AtsIndex, uint32 NewCmdIndex, uint32 ListLength);

/************************************************************************/
/** \brief Initializes ATS tables before a load starts
 **
 **  \par Description
 **            This function simply clears out the ats tables in preparation
 **            for a load.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]    AtsIndex        ATS array index
 **
 *************************************************************************/
void SC_InitAtsTables(uint16 AtsIndex);

/************************************************************************/
/** \brief Validation function for ATS or Append ATS table data
 **
 **  \par Description
 **              This routine is called to validate the contents of an ATS
 **            or Apppend ATS table.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS         \retdesc \copydoc CFE_SUCCESS   \endcode
 **  \retcode #SC_ERROR            \retdesc \copydoc SC_ERROR   \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_VerifyAtsTable(uint32 *Buffer, int32 BufferWords);

/************************************************************************/
/** \brief Validation function for a single ATS or Append ATS table entry
 **
 **  \par Description
 **              This routine is called to validate the contents of a
 **            single ATS or Append ATS table entry.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retstmt Returns 0 if no more entries in the table  \endcode
 **  \retstmt Returns -1 if the current entry is invalid  \endcode
 **  \retstmt Returns positive integer equal to table entry length (in words) \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_VerifyAtsEntry(uint32 *Buffer, int32 EntryIndex, int32 BufferWords);

/************************************************************************/
/** \brief Loads an ATS into the data structures in SC
 **
 **  \par Description
 **         This routine is called when the SC app gets a new ATS table.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]    AtsIndex            ATS table array index (base zero)
 **
 **
 *************************************************************************/
void SC_LoadAts(uint16 AtsIndex);

/************************************************************************/
/** \brief Validation function for an ATS
 **
 **  \par Description
 **              This routine is called from the cFE Table Services and passed
 **            as a parameter in the cFE Table Registration call.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS         \retdesc \copydoc CFE_SUCCESS   \endcode
 **  \retcode #SC_ERROR            \retdesc \copydoc SC_ERROR   \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_ValidateAts(void *TableData);

/************************************************************************/
/** \brief Validation function for the Append ATS Table
 **
 **  \par Description
 **         This routine is called from the cFE Table Services as part of
 **         the table load/validate/commit process.  The function pointer
 **         is passed as a parameter in the cFE Table Registration call.
 **
 **  \par Assumptions, External Events, and Notes:
 **         None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS         \retdesc \copydoc CFE_SUCCESS   \endcode
 **  \retcode #SC_ERROR            \retdesc \copydoc SC_ERROR   \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_ValidateAppend(void *TableData);

/************************************************************************/
/** \brief Updates Append ATS Info table per new contents of Append ATS table
 **
 **  \par Description
 **         This routine is called when the SC app receives notification
 **         from cFE Table Services that the Append ATS table contents
 **         have been updated.
 **
 **  \par Assumptions, External Events, and Notes:
 **         None
 **
 *************************************************************************/
void SC_UpdateAppend(void);

/************************************************************************/
/** \brief Appends contents of Append ATS table to indicated ATS table
 **
 **  \par Description
 **         This routine is called from the Append ATS command handler to
 **         append the contents of the Append ATS table to the end of the
 **         indicated ATS table.
 **
 **  \par Assumptions, External Events, and Notes:
 **         None
 **
 **  \param [in]    AtsIndex            ATS table array index (base zero)
 **
 *************************************************************************/
void SC_ProcessAppend(uint16 AtsIndex);

/************************************************************************/
/** \brief Loads an RTS into the data structures in SC
 **
 **  \par Description
 **         This routine is called when the SC app gets a new RTS table.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]    RtsIndex            RTS table array index (base zero)
 **
 **
 *************************************************************************/
void SC_LoadRts(uint16 RtsIndex);

/************************************************************************/
/** \brief Validation function for an RTS
 **
 **  \par Description
 **              This routine is called from the cFE Table Services and passed
 **            as a parameter in the cFE Table Registration call.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS         \retdesc \copydoc CFE_SUCCESS   \endcode
 **  \retcode #SC_ERROR            \retdesc \copydoc SC_ERROR   \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_ValidateRts(void *TableData);

#endif /*_sc_loads_*/

/************************/
/*  End of File Comment */
/************************/
