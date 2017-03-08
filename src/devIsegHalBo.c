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
 * @file devIsegHalBo.c
 * @author F.Feldbauer
 * @date 25 May 2015
 * @brief Device Support implementation for bo records
 */

/*_____ I N C L U D E S ______________________________________________________*/

/* ANSI C includes  */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* EPICS includes */
#include <boRecord.h>
#include <alarm.h>
#include <dbAccess.h>
#include <errlog.h>
#include <epicsExport.h>
#include <epicsTypes.h>
#include <iocLog.h>
#include <iocsh.h>
#include <recGbl.h>

/* local includes */
#include "devIsegHal.h"

/*_____ D E F I N I T I O N S ________________________________________________*/
static long devIsegHalInitRecord_bo( boRecord *prec );
static long devIsegHalWrite_bo( dbCommon *prec, char* value ); 

/*_____ G L O B A L S ________________________________________________________*/
devIsegHal_dset_t devIsegHalBo = {
  7,
  NULL,
  devIsegHalInit,
  devIsegHalInitRecord_bo,
  NULL,
  devIsegHalWrite,
  NULL,
  devIsegHalWrite_bo
};
epicsExportAddress( dset, devIsegHalBo );

/*_____ L O C A L S __________________________________________________________*/

/*_____ F U N C T I O N S ____________________________________________________*/

/**-----------------------------------------------------------------------------
 * @brief   Initialization of bo records
 * @param   [in]  prec   Address of the record calling this function
 * @return  In case of error return -1, otherwise return 0
 *----------------------------------------------------------------------------*/
static long devIsegHalInitRecord_bo( boRecord *prec ){
  prec->pact = (epicsUInt8)true; /* disable record */

  devIsegHal_rec_t conf = { &prec->out, "WR", "BOOL", true };
  long status = devIsegHalInitRecord( (dbCommon*)prec, &conf );
  if( status != 0 ) return ERROR;

  prec->pact = (epicsUInt8)false; /* enable record */

  return OK;
}

/**-----------------------------------------------------------------------------
 * @brief   Convert value to cstring for bo records
 * @param   [in]  prec   Address of the record calling this function
 * @param   [in]  value  Address of cstring containing value
 * @return  -1 in case of error, otherwise 0
 *
 * Upon normal process of the record, the current contents of the VAL field
 * is converted into a cstring. If PACT is set to true (process via callback)
 * the cstring is instead parsed and its value written to VAL field.
 *----------------------------------------------------------------------------*/
static long devIsegHalWrite_bo( dbCommon *prec, char* value ) {
  boRecord *pbo = (boRecord *)prec;

  if( pbo->pact ) {
    epicsUInt32 buffer = 0;
    if( sscanf( value, "%u", &buffer ) != 1 ) {
      return ERROR;
    }
    pbo->val   = ( buffer ? 1 : 0 );
    pbo->rval  = ( buffer ? 1 : 0 );
    return OK;
  }

  value[0] = ( pbo->rval ? '1' : '0' );
  value[1] = 0; /* just to be sure */
  return OK;
}

