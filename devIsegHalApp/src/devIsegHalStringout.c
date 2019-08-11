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
 * @file devIsegHalStringout.c
 * @author F.Feldbauer
 * @date 25 May 2015
 * @brief Device Support implementation for stringout records
 */

/*_____ I N C L U D E S ______________________________________________________*/

/* ANSI C includes  */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* EPICS includes */
#include <alarm.h>
#include <dbAccess.h>
#include <errlog.h>
#include <epicsExport.h>
#include <epicsTypes.h>
#include <iocLog.h>
#include <iocsh.h>
#include <recGbl.h>
#include <stringoutRecord.h>

/* local includes */
#include "devIsegHal.h"

/*_____ D E F I N I T I O N S ________________________________________________*/
static long devIsegHalInitRecord_so( stringoutRecord *prec );
static long devIsegHalWrite_so( dbCommon *prec, char* value ); 

/*_____ G L O B A L S ________________________________________________________*/
devIsegHal_dset_t devIsegHalSo = {
  7,
  NULL,
  devIsegHalInit,
  devIsegHalInitRecord_so,
  NULL,
  devIsegHalWrite,
  NULL,
  devIsegHalWrite_so
};
epicsExportAddress( dset, devIsegHalSo );

/*_____ L O C A L S __________________________________________________________*/

/*_____ F U N C T I O N S ____________________________________________________*/

/**-----------------------------------------------------------------------------
 * @brief       Initialization of stringout records
 * @param [in]  prec   Address of the record calling this function
 * @return      In case of error return -1, otherwise return 0
 *----------------------------------------------------------------------------*/
static long devIsegHalInitRecord_so( stringoutRecord *prec ){
  prec->pact = (epicsUInt8)true; /* disable record */

  devIsegHal_rec_t conf = { &prec->out, "W", "STR", false };
  long status = devIsegHalInitRecord( (dbCommon*)prec, &conf );
  if( status != 0 ) return ERROR;

  prec->udf  = FALSE;
  prec->pact = (epicsUInt8)false; /* enable record */

  return OK;
}

/**-----------------------------------------------------------------------------
 * @brief       Convert value to cstring for stringin records
 * @param [in]  prec   Address of the record calling this function
 * @param [out] value  Address of cstring containing value
 * @return      -1 in case of error, otherwise 0
 *----------------------------------------------------------------------------*/
static long devIsegHalWrite_so( dbCommon *prec, char* value ) {
  stringoutRecord *pso = (stringoutRecord *)prec;
  strncpy( value, pso->val, MAX_STRING_SIZE );
  return OK;
}

