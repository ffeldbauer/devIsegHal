//******************************************************************************
// Copyright (C) 2015 Florian Feldbauer <f.feldbauer@him.uni-mainz.de>
//                    - Helmholtz-Institut Mainz
//                    iseg Spezialelektronik GmbH
//
// This file is part of deviseg
//
// deviseg is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// deviseg is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// version 2.0.0; May 25, 2015
//
//******************************************************************************

//! @file devIsegHal.cpp
//! @author F.Feldbauer
//! @date 25 May 2015
//! @brief Global functions of devIsegHal

//_____ I N C L U D E S ________________________________________________________

// ANSI C/C++ includes
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>

// EPICS includes
#include <alarm.h>
#include <dbAccess.h>
#include <errlog.h>
#include <epicsExport.h>
#include <epicsThread.h>
#include <epicsTypes.h>
#include <iocLog.h>
#include <iocsh.h>
#include <recGbl.h>

// local includes
#include "devIsegHalClasses.h"

//_____ D E F I N I T I O N S __________________________________________________

//_____ G L O B A L S __________________________________________________________

//_____ L O C A L S ____________________________________________________________
static isegHalThread* myIsegHalThread = NULL;

//_____ F U N C T I O N S ______________________________________________________

//------------------------------------------------------------------------------
//! @brief       D'tor of class isegHalConnectionHandler
//!
//! Disconnects all registered interfaces
//------------------------------------------------------------------------------
isegHalConnectionHandler::~isegHalConnectionHandler() {
  std::vector< std::string >::iterator it = _interfaces.begin();
  for( ; it != _interfaces.end(); ++it ) {
  	IsegResult status = iseg_disconnect( it->c_str() );
    if ( ISEG_OK != status ) {
      std::cerr << "\033[31;1m Cannot disconnect from isegHAL interface '"
                << (*it) << "'.\033[0m"
                << std::endl;
  	}
  }
  _interfaces.clear();
}

//------------------------------------------------------------------------------
//! @brief       Get instance of the isegHALconnection handler
//! @return      Reference to singleton
//!
//! This function returns a reference to the only instance of this singleton
//! class. If no instance exists yet, it is created.
//! Using a reference for the singleton, the destructor of the class will be
//! correctly called at the end of the programm
//------------------------------------------------------------------------------
isegHalConnectionHandler& isegHalConnectionHandler::instance() {
  static isegHalConnectionHandler myInstance;
  return myInstance;
}

//------------------------------------------------------------------------------
//! @brief       Connect to new interface
//! @param [in]  name        deviseg internal name of the interface handle
//! @param [in]  interface   name of the hardware interface
//! @return      true if interface is already connected or if successfully connected
//------------------------------------------------------------------------------
bool isegHalConnectionHandler::connect( std::string const& name, std::string const& interface ) {
  std::vector< std::string >::iterator it;

  it = std::find( _interfaces.begin(), _interfaces.end(), name );
  if( it != _interfaces.end() ) return true;

  //  std::cout << "Trying to connect to '" << interface << "'" << std::endl;

  IsegResult status = iseg_connect( name.c_str(), interface.c_str(), NULL );
  if ( ISEG_OK != status ) {
    std::cerr << "\033[31;1m Cannot connect to isegHAL interface '"
              << interface << "': "
              << strerror( errno )
              << "\033[0m"
              << std::endl;
    return false;
  }
  // iseg HAL starts collecting data from hardware after connect.
  // wait 5 secs to let alle values 'initialize'
  sleep( 5 ); 

  _interfaces.push_back( interface );
  return true;
}

//------------------------------------------------------------------------------
//! @brief       Check if an interface is connected
//! @param [in]  name    deviseg internal name of the interface handle
//------------------------------------------------------------------------------
bool isegHalConnectionHandler::connected( std::string const& name ) {
  std::vector< std::string >::iterator it;
  it = std::find( _interfaces.begin(), _interfaces.end(), name );
  if( it != _interfaces.end() ) return true;
  return false;
}

//------------------------------------------------------------------------------
//! @brief       Disconnect from an interface
//! @param [in]  name    deviseg internal name of the interface handle
//------------------------------------------------------------------------------
void isegHalConnectionHandler::disconnect( std::string const& name ) {
  std::vector< std::string >::iterator it;
  it = std::find( _interfaces.begin(), _interfaces.end(), name );

  if( it != _interfaces.end() ) {
  	int status = iseg_disconnect( name.c_str() );
    if ( ISEG_OK != status ) {
      std::cerr << "\033[31;1m Cannot disconnect from isegHAL interface '"
                << name << "'.\033[0m"
                << std::endl;
  		return;
  	}
    _interfaces.erase( it );
  }
}


//------------------------------------------------------------------------------
//! @brief       Initialization of device support
//! @param [in]  after  flag telling if function is called after or before
//!                       record initialization
//! @return      In case of error return -1, otherwise return 0
//------------------------------------------------------------------------------
long devIsegHalInit( int after ) {

  if ( 0 == after ) { // before records have been initialized
    static bool firstRunBefore = true;
    if ( !firstRunBefore ) return 0;
    firstRunBefore = false;

    // create polling thread
    myIsegHalThread = new isegHalThread();

  } else {

    static bool firstRunAfter = true;
    if ( !firstRunAfter ) return 0;
    firstRunAfter = false;

    // start thread
    myIsegHalThread->thread.start();
  }

  return OK;

}

//------------------------------------------------------------------------------
//! @brief       Common initialization of the record
//! @param [in]  prec       Address of the record calling this function
//! @param [in]  pconf      Address of record configuration
//! @return      In case of error return -1, otherwise return 0
//------------------------------------------------------------------------------
long devIsegHalInitRecord( dbCommon *prec, const devIsegHal_rec_t *pconf ) {
  devIsegHal_dset_t *pdset = (devIsegHal_dset_t *)prec->dset;
  long status = OK;

  if( INST_IO != pconf->ioLink->type ) {
    std::cerr << prec->name << ": Invalid link type for INP/OUT field: "
              << pamaplinkType[ pconf->ioLink->type ].strvalue
              << std::endl;
    return ERROR;
  }

  std::vector< std::string > options;
  std::istringstream ss( pconf->ioLink->value.instio.string );
  std::string option;
  while( std::getline( ss, option, ' ' ) ) options.push_back( option );

  if( options.size() != 2 ) {
    std::cerr << prec->name << ": Invalid INP/OUT field: " << ss.str() << "\n"
              << "    Syntax is \"@<isegItem> <Interface>\"" << std::endl;
    return ERROR;
  }

  // Test if interface is connected to isegHAL server
  if( !isegHalConnectionHandler::instance().connected( options.at(1) ) ) {
    std::cerr << "\033[31;1m" << "isegHal interface " << options.at(1) << " not connected!"
              << "\033[0m" << std::endl;
    return ERROR;
  }

  IsegItemProperty isegItem = iseg_getItemProperty( options.at(1).c_str(), options.at(0).c_str() );
  if( strcmp( isegItem.quality, ISEG_ITEM_QUALITY_OK ) != 0 ) {
    fprintf( stderr, "\033[31;1m%s: Error while reading item property '%s' (Q: %s)\033[0m\n",
             prec->name, options.at(0).c_str(), isegItem.quality );
    return ERROR; 
  }

  // "Vorsicht ist die Mutter der Porzelankiste",
  // or "Better safe than sorry"
  for ( size_t i = 0; i < strlen( pconf->access ); ++i ) {
    if ( NULL == strchr( isegItem.access, pconf->access[i] ) ) {
      fprintf( stderr, "\033[31;1m%s: Access rights of item '%s' don't match: %s|%s!\033[0m\n",
               prec->name, isegItem.object, pconf->access, isegItem.access );
      return ERROR;
    }
  }
  if ( strncmp( isegItem.type, pconf->type, strlen( pconf->type ) ) != 0 ) {
    fprintf( stderr, "\033[31;1m%s: DataType '%s' of '%s' not supported by this record!\033[0m\n",
             prec->name, isegItem.type, isegItem.object );
    return ERROR; 
  }

  devIsegHal_info_t *pinfo = new devIsegHal_info_t;
  memcpy( pinfo->object, isegItem.object, FULLY_QUALIFIED_OBJECT_SIZE );
  strncpy( pinfo->interface, options.at(1).c_str(), 20 );
  memcpy( pinfo->unit,   isegItem.unit,   UNIT_SIZE );
  pinfo->pcallback = NULL;  // just to be sure

  /// Get initial value from HAL
  IsegItem item = iseg_getItem( pinfo->interface, pinfo->object );
  if( strcmp( item.quality, ISEG_ITEM_QUALITY_OK ) != 0 ) {
    fprintf( stderr, "\033[31;1m%s: Error while reading value '%s' from interface '%s': '%s' (Q: %s)\033[0m\n",
             prec->name, item.object, pinfo->interface, item.value, item.quality );
  }
  epicsUInt32 seconds = 0;
  epicsUInt32 microsecs = 0;
  if( sscanf( item.timeStampLastChanged, "%u.%u", &seconds, &microsecs ) != 2 ) {
    fprintf( stderr, "\033[31;1m%s: Error parsing timestamp for '%s': %s\033[0m\n", prec->name, pinfo->object, item.timeStampLastChanged );
  }
  pinfo->time.secPastEpoch = seconds - POSIX_TIME_AT_EPICS_EPOCH;
  pinfo->time.nsec = microsecs * 100000;
  status = pdset->conv_val_str( prec, item.value );
  if( ERROR == status ) {
    fprintf( stderr, "\033[31;1m%s: Error parsing value for '%s': %s\033[0m\n", prec->name, pinfo->object, item.value );
  }
  if( -2 == prec->tse ) prec->time = pinfo->time;

  /// I/O Intr handling
  scanIoInit( &pinfo->ioscanpvt );
  if( pconf->registerCallback ) myIsegHalThread->registerInterrupt( prec, pinfo );

  prec->dpvt = pinfo;
  prec->udf  = (epicsUInt8)false;

  return OK;
}

//------------------------------------------------------------------------------
//! @brief       Initialization of the record for the broadcast on/off switch
//! @param [in]  prec       Address of the record calling this function
//! @param [in]  pconf      Address of record configuration
//! @return      In case of error return -1, otherwise return 0
//------------------------------------------------------------------------------
long devIsegHalGlobalSwitchInit( dbCommon *prec, const devIsegHal_rec_t *pconf ) {

  if( INST_IO != pconf->ioLink->type ) {
    std::cerr << prec->name << ": Invalid link type for INP/OUT field: "
              << pamaplinkType[ pconf->ioLink->type ].strvalue
              << std::endl;
    return ERROR;
  }

  std::vector< std::string > options;
  std::istringstream ss( pconf->ioLink->value.instio.string );
  std::string option;
  while( std::getline( ss, option, ' ' ) ) options.push_back( option );

  if( options.size() != 2 ) {
    std::cerr << prec->name << ": Invalid INP/OUT field: " << ss.str() << "\n"
              << "    Syntax is \"@<{OnOff|Emergency}> <Interface>\"" << std::endl;
    return ERROR;
  }
  
  bool emergency;
  if( "OnOff" == options[0] ) {
    emergency = false;
  } else if( "Emergency" == options[0] ) {
    emergency = true;
  } else {
    std::cerr << prec->name << ": Invalid INP/OUT field: " << ss.str() << "\n"
              << "    Syntax is \"@<{OnOff|Emergency}> <Interface>\"" << std::endl;
    return ERROR;
  }
  
  // Test if interface is connected to isegHAL server
  if( !isegHalConnectionHandler::instance().connected( options.at(1) ) ) {
    std::cerr << "\033[31;1m" << "isegHal interface " << options.at(1) << " not connected!"
              << "\033[0m" << std::endl;
    return ERROR;
  }

  devIsegHal_info_t *pinfo = new devIsegHal_info_t;
  memset( pinfo->object, 0, FULLY_QUALIFIED_OBJECT_SIZE );
  pinfo->object[0] = emergency ? 'E' : 'O'; // Abuse field for iseg item to store 'O' for normal on/off and 'E' for emergency off
  strncpy( pinfo->interface, options.at(1).c_str(), 20 );
  memset( pinfo->unit, 0, UNIT_SIZE );
  pinfo->pcallback = NULL;  // just to be sure

  if( pconf->registerCallback ) myIsegHalThread->registerInterrupt( prec, pinfo );

  prec->dpvt = pinfo;

  return OK;
}

//------------------------------------------------------------------------------
//! @brief       Get I/O Intr Information of record
//! @param [in]  cmd   0 if record is placed in, 1 if taken out of an I/O scan list 
//! @param [in]  prec  Address of record calling this funciton
//! @param [out] ppvt  Address of IOSCANPVT structure
//! @return      ERROR in case of an error, otherwise OK
//------------------------------------------------------------------------------
long devIsegHalGetIoIntInfo( int cmd, dbCommon *prec, IOSCANPVT *ppvt ) {
  devIsegHal_info_t *pinfo = (devIsegHal_info_t *)prec->dpvt;
  *ppvt = pinfo->ioscanpvt;
  if ( 0 == cmd ) {
    myIsegHalThread->registerInterrupt( prec, pinfo );
  } else {
    myIsegHalThread->cancelInterrupt( pinfo );
  }
  return OK;
}

//------------------------------------------------------------------------------
//! @brief       Common read function of the records
//! @param [in]  prec  Address of record calling this funciton
//! @return      ERROR in case of an error, otherwise OK
//------------------------------------------------------------------------------
long devIsegHalRead( dbCommon *prec ) {
  devIsegHal_info_t *pinfo = (devIsegHal_info_t *)prec->dpvt;
  devIsegHal_dset_t *pdset = (devIsegHal_dset_t *)prec->dset;
  long status = OK;

  if( !prec->pact ) {
    // record "normally" processed
    IsegItem item = iseg_getItem( pinfo->interface, pinfo->object );
    if( strcmp( item.quality, ISEG_ITEM_QUALITY_OK ) != 0 ) {
      fprintf( stderr, "\033[31;1m%s: Error while reading value '%s' from interface '%s': '%s' (Q: %s)\033[0m\n",
               prec->name, item.object, pinfo->interface, item.value, item.quality );
      recGblSetSevr( prec, READ_ALARM, INVALID_ALARM ); // Set record to READ_ALARM
      return ERROR; 
    }

    epicsUInt32 seconds = 0;
    epicsUInt32 microsecs = 0;
    if( sscanf( item.timeStampLastChanged, "%u.%u", &seconds, &microsecs ) != 2 ) {
      fprintf( stderr, "\033[31;1m%s: Error parsing timestamp for '%s': %s\033[0m\n", prec->name, pinfo->object, item.timeStampLastChanged );
      recGblSetSevr( prec, READ_ALARM, INVALID_ALARM ); // Set record to READ_ALARM
      return ERROR; 
    }
    pinfo->time.secPastEpoch = seconds - POSIX_TIME_AT_EPICS_EPOCH;
    pinfo->time.nsec = microsecs * 100000;

#ifdef CHECK_LAST_REFRESHED
    epicsTimeStamp lastRefreshed;
    if( sscanf( item.timeStampLastRefreshed, "%u.%u", &lastRefreshed.secPastEpoch, &lastRefreshed.nsec ) != 2 ) {
      fprintf( stderr, "\033[31;1m%s: Error parsing timestamp for '%s': %s\033[0m\n", prec->name, pinfo->object, item.timeStampLastRefreshed );
      recGblSetSevr( prec, READ_ALARM, INVALID_ALARM ); // Set record to READ_ALARM
      return ERROR; 
    }
    lastRefreshed.secPastEpoch -= POSIX_TIME_AT_EPICS_EPOCH;
    lastRefreshed.nsec *= 100000;
    if( epicsTime::getCurrent() - epicsTime( lastRefreshed ) >= 30.0 ) {
      /// value is older then 30 seconds
      recGblSetSevr( prec, TIMEOUT_ALARM, INVALID_ALARM );
      return ERROR; 
    }
#endif

    status = pdset->conv_val_str( prec, item.value );
    if( ERROR == status ) {
      fprintf( stderr, "\033[31;1m%s: Error parsing value for '%s': %s\033[0m\n", prec->name, pinfo->object, item.value );
      recGblSetSevr( prec, READ_ALARM, INVALID_ALARM ); // Set record to READ_ALARM
      return ERROR;
    }

  } else { 
    // record forced processed by CALLBACK
    status = pdset->conv_val_str( prec, pinfo->value );
    prec->pact = (epicsUInt8)false;
    if( ERROR == status ) {
      fprintf( stderr, "\033[31;1m%s: Error parsing value for '%s': %s\033[0m\n", prec->name, pinfo->object, pinfo->value );
      recGblSetSevr( prec, READ_ALARM, INVALID_ALARM ); // Set record to READ_ALARM
      return ERROR;
    }

  }

  if( -2 == prec->tse ) {
    // timestamp is set by device support
    prec->time = pinfo->time;
  }
  prec->udf = (epicsUInt8)false;

  return status;
}

//------------------------------------------------------------------------------
//! @brief       Common write function of the records
//! @param [in]  prec   Address of record calling this function
//! @return      ERROR in case of an error, otherwise OK
//------------------------------------------------------------------------------
long devIsegHalWrite( dbCommon *prec ) {
  devIsegHal_info_t *pinfo = (devIsegHal_info_t *)prec->dpvt;
  devIsegHal_dset_t *pdset = (devIsegHal_dset_t *)prec->dset;

  if( prec->pact ) {
    long status = pdset->conv_val_str( prec, pinfo->value );
    if( -2 == prec->tse ) prec->time = pinfo->time;
    prec->pact = (epicsUInt8)false;
    prec->udf = (epicsUInt8)false;
    return status;
  }

  myIsegHalThread->disable();

  char value[VALUE_SIZE];
  long status = pdset->conv_val_str( prec, value );
  if( ERROR == status ) {
    fprintf( stderr, "\033[31;1m%s: Error parsing value for '%s'\033[0m\n", prec->name, pinfo->object );
    recGblSetSevr( prec, WRITE_ALARM, INVALID_ALARM ); // Set record to WRITE_ALARM
    return ERROR;
  }

  if( iseg_setItem( pinfo->interface, pinfo->object, value ) != ISEG_OK ) {
    fprintf( stderr, "\033[31;1m%s: Error while writing value '%s': '%s'\033[0m\n",
             prec->name, pinfo->object, value );
    recGblSetSevr( prec, WRITE_ALARM, INVALID_ALARM ); // Set record to WRITE_ALARM 
    return ERROR; 
  }

  if( -2 == prec->tse ) {
    epicsTimeGetCurrent( &pinfo->time );
    prec->time = pinfo->time;
  }

  myIsegHalThread->enable();
  return status;
}

//------------------------------------------------------------------------------
//! @brief       Switch all channels on/off by broadcast message
//! @param [in]  prec   Address of record calling this function
//! @return      ERROR in case of an error, otherwise OK
//------------------------------------------------------------------------------
long devIsegHalGlobalSwitchWrite( dbCommon *prec ) {
  devIsegHal_info_t *pinfo = (devIsegHal_info_t *)prec->dpvt;
  devIsegHal_dset_t *pdset = (devIsegHal_dset_t *)prec->dset;

  myIsegHalThread->disable();

  char value[VALUE_SIZE];
  value[0] = pinfo->object[0];
  long status = pdset->conv_val_str( prec, value );
  if( ERROR == status ) {
    fprintf( stderr, "\033[31;1m%s: Invalid type parameter, cannot create broadcast command.\033[0m\n", prec->name );
    recGblSetSevr( prec, SOFT_ALARM, INVALID_ALARM ); // Set record to SOFT_ALARM 
    return ERROR;
  }

  if ( iseg_setItem( pinfo->interface, "Configuration", "1" ) != ISEG_OK ) {
    fprintf( stderr, "\033[31;1m%s: Error while stopping data collector for sending broadcast.\033[0m\n",
             prec->name );
    recGblSetSevr( prec, WRITE_ALARM, INVALID_ALARM ); // Set record to WRITE_ALARM 
    iseg_setItem( pinfo->interface, "Configuration", "0"); // Restore function
    myIsegHalThread->enable();
    return ERROR; 
  }
  if ( iseg_setItem( pinfo->interface, "Write", value ) != ISEG_OK ) {
    fprintf( stderr, "\033[31;1m%s: Error while sending broadcast command.\033[0m\n",
             prec->name );
    recGblSetSevr( prec, WRITE_ALARM, INVALID_ALARM ); // Set record to WRITE_ALARM 
    iseg_setItem( pinfo->interface, "Configuration", "0"); // Restore function
    myIsegHalThread->enable();
    return ERROR; 
  }
  if ( iseg_setItem( pinfo->interface, "Configuration", "0" ) != ISEG_OK ) {
    fprintf( stderr, "\033[31;1m%s: Error while starting data collector after sending broadcast.\033[0m\n",
             prec->name );
    recGblSetSevr( prec, WRITE_ALARM, INVALID_ALARM ); // Set record to WRITE_ALARM 
    myIsegHalThread->enable();
    return ERROR; 
  }

  if( -2 == prec->tse ) {
    epicsTimeGetCurrent( &pinfo->time );
    prec->time = pinfo->time;
  }

  myIsegHalThread->enable();
  return OK;
}

//------------------------------------------------------------------------------
//! @brief       C'tor of isegHalThread
//------------------------------------------------------------------------------
isegHalThread::isegHalThread()
  : thread( *this, "isegHAL", epicsThreadGetStackSize( epicsThreadStackSmall ), 50 ),
    _run( true ),
    _pause(5.),
    _debug(0)
{
  _recs.clear();
}

//------------------------------------------------------------------------------
//! @brief       D'tor of isegHalThread
//------------------------------------------------------------------------------
isegHalThread::~isegHalThread() {
  _recs.clear();
}

//------------------------------------------------------------------------------
//! @brief       Run operation of thread
//!
//! Go through the list of registered records and check the cached value
//! from isegHAL.
//! If the cached value differs from the current value in the record,
//! the record will be updated.
//! After having checked all records, the thread sleeps for 5 seconds and
//! repeats the check.
//------------------------------------------------------------------------------
void isegHalThread::run() {
  while( true ) {
    std::list<devIsegHal_info_t*>::iterator it = _recs.begin();
    if( _pause > 0. ) this->thread.sleep( _pause ); 

    if( !_run ) continue;

    // some "benchmarking"
    time_t start;
    time(&start);

    for( ; it != _recs.end(); ++it ) {

      if( 3 <= _debug )
        printf( "isegHalThread::run: Reading item '%s'\n", (*it)->object );

      IsegItem item = iseg_getItem( (*it)->interface, (*it)->object );
      if( strcmp( item.quality, ISEG_ITEM_QUALITY_OK ) != 0 ) continue;

      epicsUInt32 seconds = 0;
      epicsUInt32 microsecs = 0;
      if( sscanf( item.timeStampLastChanged, "%u.%u", &seconds, &microsecs ) != 2 ) continue;
      epicsTimeStamp time;
      time.secPastEpoch = seconds - POSIX_TIME_AT_EPICS_EPOCH;
      time.nsec = microsecs * 100000;
      
      if( (*it)->time.secPastEpoch < time.secPastEpoch ) {
        if( 1 <= _debug )
          printf( "isegHalThread::run: New value for item '%s': %s -> %s\n",
                  (*it)->object, (*it)->value, item.value );
        // value was updated in isegHAL
        memcpy( (*it)->value, item.value, VALUE_SIZE );
        (*it)->time = time;
        callbackRequest( (*it)->pcallback );
      }
    }

    // some "benchmarking"
    time_t stop;
    time(&stop);

    if( 2 <= _debug )
      printf( "isegHalThread::run: needed %lf seconds for %lu records\n",
               difftime( stop, start ), (unsigned long)_recs.size() );

  }
}

//------------------------------------------------------------------------------
//! @brief       Add a record to the list
//! @param [in]  prec  Address of the record to be added
//!
//! Registers a new record to be checked by the thread
//------------------------------------------------------------------------------
void isegHalThread::registerInterrupt( dbCommon* prec, devIsegHal_info_t *pinfo ) {
  //devIsegHal_info_t *pinfo = (devIsegHal_info_t *)prec->dpvt;
  if( !pinfo->pcallback ) {
    CALLBACK *pcallback = new CALLBACK;
    callbackSetCallback( devIsegHalCallback, pcallback );
    callbackSetUser( (void*)prec, pcallback );
    callbackSetPriority( priorityLow, pcallback );
    pinfo->pcallback = pcallback;
  }

  if( 1 <= _debug )
    printf( "isegHalThread: Register new record '%s'\n", prec->name );

  _recs.push_back( pinfo );
  // to be sure that each record is only added once
  _recs.sort();
  _recs.unique();
}

//------------------------------------------------------------------------------
//! @brief       Remove a record to the list
//! @param [in]  pinfo  Address of the record's private data structure
//!
//! Removes a record from the list which is checked by the thread for updates
//------------------------------------------------------------------------------
void isegHalThread::cancelInterrupt( const devIsegHal_info_t* pinfo ) {
  std::list<devIsegHal_info_t*>::iterator it = _recs.begin();
  for( ; it != _recs.end(); ++it ) {
    if( pinfo == (*it) ) {
      _recs.erase( it );
      break;
    }
  } 
}

// Configuration routines.  Called from the iocsh function below 
extern "C" {

  static const iocshArg isegConnectArg0 = { "port", iocshArgString };
  static const iocshArg isegConnectArg1 = { "tty",  iocshArgString };
  static const iocshArg * const isegConnectArgs[] = { &isegConnectArg0, &isegConnectArg1 };
  static const iocshFuncDef isegConnectFuncDef = { "isegHalConnect", 2, isegConnectArgs };

  //----------------------------------------------------------------------------
  //! @brief       iocsh callable function to connect to isegHalServer
  //!
  //! This function can be called from the iocsh via "isegHalConnect( PORT, TTY )"
  //! PORT is the deviseg internal name of the interface which is also used inside
  //! the records.
  //! TTY is the name of the hardware interface (e.g. "can0").
  //----------------------------------------------------------------------------
  static void isegConnectCallFunc( const iocshArgBuf *args ) {
    std::cout << "using HAL version [" << iseg_getVersionString() << "]" << std::endl;
    if( !isegHalConnectionHandler::instance().connect( args[0].sval, args[1].sval ) ){
      fprintf( stderr, "\033[31;1mCannot connect to isegHAL interface %s(%s)\033[0m\n", args[0].sval, args[1].sval );
    }
  }

  // iocsh callable function to set options for polling thread
  static const iocshArg setOptArg0 = { "port", iocshArgString };
  static const iocshArg setOptArg1 = { "key", iocshArgString };
  static const iocshArg setOptArg2 = { "value",  iocshArgString };
  static const iocshArg * const setOptArgs[] = { &setOptArg0, &setOptArg1, &setOptArg2 };
  static const iocshFuncDef setOptFuncDef = { "devIsegHalSetOpt", 3, setOptArgs };

  //----------------------------------------------------------------------------
  //! @brief       iocsh callable function to set options for the polling thread
  //!
  //! This function can be called from the iocsh via "devIsegHalSetOpt( PORT, KEY, VALUE )"
  //! PORT is the deviseg internal name of the interface connected to the isegHalServer,
  //! KEY is the name of the option and VALUE the new value for the option.
  //!
  //! Possbile KEYs are:
  //! Intervall  -  set the wait time after going through the list of records with the polling thread
  //! LogLevel   -  Change loglevel of isegHalServer
  //! debug      -  Enable debug output of polling thread
  //----------------------------------------------------------------------------
  static void setOptCallFunc( const iocshArgBuf *args ) {
    // Set new intervall for polling thread
    if( strcmp( args[1].sval, "Intervall" ) == 0 ) {
      double newIntervall = 0.;
      int n = sscanf( args[1].sval, "%lf", &newIntervall );
      if( 1 != n ) {
        fprintf( stderr, "\033[31;1mInvalid value for key '%s': %s\033[0m\n", args[0].sval, args[1].sval );
        return;
      }
      myIsegHalThread->changeIntervall( newIntervall );
    }

    // change log level from isegHAL server
    if( strcmp( args[1].sval, "LogLevel" ) == 0 ) {
      if( iseg_setItem( args[0].sval, "LogLevel", args[2].sval ) != ISEG_OK ) {
        fprintf( stderr, "\033[31;1mCould not change loglevel to '%s'\033[0m\n", args[1].sval );
        return;
      }
    }

    // Set new debug level
    if( strcmp( args[0].sval, "debug" ) == 0 ) {
      unsigned newDbgLvl = 0;
      int n = sscanf( args[1].sval, "%u", &newDbgLvl );
      if( 1 != n ) {
        fprintf( stderr, "\033[31;1mInvalid value for key '%s': %s\033[0m\n", args[0].sval, args[1].sval );
        return;
      }
      myIsegHalThread->setDbgLvl( newDbgLvl );
    }

  }

  //----------------------------------------------------------------------------
  //! @brief       Register functions to EPICS
  //----------------------------------------------------------------------------
  void devIsegHalRegister( void ) {
    static bool firstTime = true;
    if ( firstTime ) {
      iocshRegister( &setOptFuncDef, setOptCallFunc );
      iocshRegister( &isegConnectFuncDef, isegConnectCallFunc );
      firstTime = false;
    }
  }
  
  epicsExportRegistrar( devIsegHalRegister );
}

