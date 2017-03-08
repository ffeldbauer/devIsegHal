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

#ifndef devIsegHal_CLASSES_H
#define devIsegHal_CLASSES_H

//_____ I N C L U D E S ________________________________________________________

// ANSI C/C++ includes  */
#include <list>
#include <string>
#include <vector>

// EPICS includes
#include <dbAccess.h>
#include <epicsThread.h>

// local includes
#include "devIsegHal.h"

//_____ D E F I N I T I O N S __________________________________________________

//! @brief   Handler for iseg interfaces
//!
//! This class handles the connection of the used
//! interfaces to the isegHal server.
//! This class uses the singleton design pattern
class isegHalConnectionHandler {
 public:

   static isegHalConnectionHandler& instance();

   bool connect( std::string const& name, std::string const& interface );
   bool connected( std::string const& name );
   void disconnect( std::string const& interface );

 private:
  isegHalConnectionHandler() {};
  ~isegHalConnectionHandler();
  isegHalConnectionHandler( isegHalConnectionHandler const& rother ); //!< copy constructor, not implemented
  isegHalConnectionHandler& operator=( isegHalConnectionHandler const& rother ); //!< Copy assignment operator not implemented

  std::vector< std::string > _interfaces;
};

//! @brief   thread monitoring set values from isegHAL
//!
//! This thread checks regulary the value of all set-parameters
//! and updates the corresponding output-records if the values
//! within the EPICS db and the isegHAL are out of sync.
class isegHalThread: public epicsThreadRunable {
 public:
  isegHalThread();
  virtual ~isegHalThread();
  virtual void run();
  epicsThread thread;

  void registerInterrupt( dbCommon* prec, devIsegHal_info_t* pinfo );
  void cancelInterrupt( const devIsegHal_info_t* pinfo );

  inline void changeIntervall( double val ) { _pause = val; }
  inline double getIntervall(){ return _pause; }

  inline void setDbgLvl( int dbglvl ) { _debug = dbglvl; }
  inline void disable() { _run = false; }
  inline void enable() { _run = true; }

 private:
  bool _run;
  double _pause;
  unsigned _debug;
  std::list< devIsegHal_info_t* > _recs;
};


#endif

