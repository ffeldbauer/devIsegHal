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
 * @file devIsegHalGlobalSwitch.c
 * @author T. Triffterer
 * @date 18 August 2016
 * @brief Device Support for bo records to switch on/off all channels via broadcast 
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
static long devIsegHalGlobalSwitchInitRecord_bo( boRecord *prec );
static long devIsegHalGlobalSwitchWrite_bo( dbCommon *prec, char* value ); 

/*_____ G L O B A L S ________________________________________________________*/
devIsegHal_dset_t devIsegHalGlobalSwitchBo = {
  7,
  NULL,
  devIsegHalInit,
  devIsegHalGlobalSwitchInitRecord_bo,
  NULL,
  devIsegHalGlobalSwitchWrite,
  NULL,
  devIsegHalGlobalSwitchWrite_bo
};
epicsExportAddress( dset, devIsegHalGlobalSwitchBo );

/*_____ L O C A L S __________________________________________________________*/

/*_____ F U N C T I O N S ____________________________________________________*/

/**-----------------------------------------------------------------------------
 * @brief   Initialization of bo records
 * @param   [in]  prec   Address of the record calling this function
 * @return  In case of error return -1, otherwise return 0
 *----------------------------------------------------------------------------*/
static long devIsegHalGlobalSwitchInitRecord_bo( boRecord *prec ){
  prec->pact = (epicsUInt8)true; /* disable record */

  devIsegHal_rec_t conf = { &prec->out, "", "", false };
  long status = devIsegHalGlobalSwitchInit( (dbCommon*)prec, &conf );
  if( status != 0 ) return ERROR;

  prec->pact = (epicsUInt8)false; /* enable record */

  return OK;
}

/**-----------------------------------------------------------------------------
 * @brief       Convert value to cstring for bo records
 * @param [in]  prec   Address of the record calling this function
 * @param [out] value  Address of cstring containing value
 * @return      -1 in case of error, otherwise 0
 *
 * Upon normal process of the record, the current contents of the VAL field
 * is converted into a cstring. If PACT is set to true (process via callback)
 * the cstring is instead parsed and its value written to VAL field.
 *----------------------------------------------------------------------------*/
static long devIsegHalGlobalSwitchWrite_bo( dbCommon *prec, char* value ) {
  boRecord *pbo = (boRecord *)prec;
  char type = value[0];

  unsigned val = 0;
  if( 'O' == type )      val = ( pbo->val << 3 );
  else if( 'E' == type ) val = ( pbo->val << 5 );
  else return ERROR;

  if( sprintf( value, "004#e800600100%02x", val ) < 0 ) {
    return ERROR;
  }
  return OK;
}

