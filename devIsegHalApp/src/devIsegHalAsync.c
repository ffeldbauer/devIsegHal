/*******************************************************************************
 * Copyright (C) 2015 Florian Feldbauer <f.feldbauer@him.uni-mainz.de>
 *                    - Helmholtz-Institut Mainz
 *
 * This file is part of devIsegHal
 *
 * devIsegHal is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * devIseghal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * version 2.0.0; May 25, 2015
 *
*******************************************************************************/

/**
 * @file devIsegHalAsync.c
 * @author F.Feldbauer
 * @date 25 May 2015
 * @brief Implementation of async callback function 
 */

/*_____ I N C L U D E S ______________________________________________________*/

/* ANSI C includes  */

/* EPICS includes */
#include <dbLock.h>
#include <recSup.h>

/* local includes */
#include "devIsegHal.h"

/*_____ D E F I N I T I O N S ________________________________________________*/

/*_____ G L O B A L S ________________________________________________________*/

/*_____ L O C A L S __________________________________________________________*/

/*_____ F U N C T I O N S ____________________________________________________*/

/**-----------------------------------------------------------------------------
 * @brief   Callback for asynchronous handling of set parameters
 * @param   [in]  pcallback   Address of EPICS CALLBACK structure
 *
 * This callback processes the the record defined in callback user.
 * This function cannot be implemented in file devIsegHal.cpp
 * because function call "(*prset->process)( prec )" is invalid in C++:
 * rset::process is of type "long (*RECSUPFUN) ()"
 *----------------------------------------------------------------------------*/
void devIsegHalCallback( CALLBACK *pcallback ) {
  dbCommon *prec;
  rset     *prset;

  callbackGetUser( prec, pcallback );

  prec->pact = (epicsUInt8)true;
  prset = (rset*)(prec->rset);

  dbScanLock( prec );
  (*prset->process)( prec );
//  dbProcess( prec );
  dbScanUnlock( prec );
}

