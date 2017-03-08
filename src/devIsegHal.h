/*******************************************************************************
 * Copyright (C) 2015 Florian Feldbauer <f.feldbauer@him.uni-mainz.de>
 *                    - Helmholtz-Institut Mainz
 *                    iseg Spezialelektronik GmbH
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

#ifndef devIsegHal_H
#define devIsegHal_H

/*_____ I N C L U D E S ______________________________________________________*/

/* ANSI C/C++ includes  */
#include <stdbool.h>

/* isegHAL includes */
#include <isegclientapi.h>

/* EPICS includes */
#include <callback.h>
#include <dbCommon.h>
#include <dbScan.h>
#include <devSup.h>
#include <epicsTime.h>
#include <shareLib.h>

/*_____ D E F I N I T I O N S ________________________________________________*/

/* Define return values for device support functions */
#define OK                    0
#define DO_NOT_CONVERT        2
#define ERROR                 -1

#define CHECK_LAST_REFRESHED

typedef long (*DEVSUPINT)(dbCommon*, char* ); /**< internal device support function */

/**
 * @brief Device Support Entry Table
 *
 * Pointers to the device support routines called by the records
 */
typedef struct {
  long number;            /**< number of device support routines */
  DEVSUPFUN report;       /**< print report (unused) */
  DEVSUPFUN init;         /**< init device support */
  DEVSUPFUN init_record;  /**< init record instance */
  DEVSUPFUN ioint_info;   /**< get io interrupt info */
  DEVSUPFUN read_write;   /**< read/write value */
  DEVSUPFUN special_conv; /**< convertion for ai/ao records */
	DEVSUPINT conv_val_str; /**< Convert value to cstring and vise versa*/
} devIsegHal_dset_t;

/**
 * @brief Record configuration
 *
 * Configuration parameters used at initialization
 * of the record
 */
typedef struct {
  const struct link *ioLink;
  const char   access[ACCESS_SIZE];
  const char   type[DATA_TYPE_SIZE];
  const bool   registerCallback;
} devIsegHal_rec_t;

/**
 * @brief Private Device Data
 *
 * Private data needed by device support routines
 */
typedef struct {
  char object[FULLY_QUALIFIED_OBJECT_SIZE]; /**< Object name for isegHAL */
  char interface[20];                       /**< Interface name for isegHAL */
  char unit[UNIT_SIZE];                     /**< Engeneering unit of this item */
  CALLBACK *pcallback;                      /**< Address of EPICS callback structure */
  IOSCANPVT ioscanpvt;                      /**< EPICS Structure needed for I/O Intrupt handling*/
  char value[VALUE_SIZE];                   /**< Value cstring from isegHAL */
  epicsTimeStamp time;                      /**< Timestamp of last change from isegHAL */
} devIsegHal_info_t;

#ifdef __cplusplus
extern "C" {
#endif

epicsShareExtern long devIsegHalInit( int after );
epicsShareExtern long devIsegHalInitRecord( dbCommon *prec, const devIsegHal_rec_t *pconf );
epicsShareExtern long devIsegHalGetIoIntInfo( int cmd, dbCommon *prec, IOSCANPVT *ppvt );
epicsShareExtern long devIsegHalRead( dbCommon *prec );
epicsShareExtern long devIsegHalWrite( dbCommon *prec );
epicsShareExtern long devIsegHalGlobalSwitchInit( dbCommon *prec, const devIsegHal_rec_t *pconf );
epicsShareExtern long devIsegHalGlobalSwitchWrite( dbCommon *prec );

epicsShareExtern void devIsegHalCallback( CALLBACK *pcallback );

#ifdef __cplusplus
} //extern "C"
#endif /* cplusplus */

#endif

