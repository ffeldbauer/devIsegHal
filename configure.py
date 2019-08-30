#!/usr/bin/env python
import argparse, os

try:
  base=os.environ['EPICS_BASE']
except KeyError:
  base=''

parser = argparse.ArgumentParser( description="Configure this package for building",
                                  epilog='Example:\n./configure.py -i linux-x86_64=${HOME}/iseg/build-x86_64 linux-arm=${HOME}/iseg/build-arm' )
parser.add_argument( '--epics-base', '-e', default=base, help='Installation path of EPICS base. Environment variable $EPICS_BASE is used as default' )
parser.add_argument( '--iseghal', '-i', nargs='*', help='Installation path of isegHAL. Value should be "TARGETARCH=PATH".' )
parser.add_argument( '--modules', '-m', nargs='*', help='Add additional device support modules to the IOC. List them as "MODULE=PATH". Currently only CALC and AUTOSAVE are linked into the IOC.' )

args = parser.parse_args()

if not args.epics_base:
  parser.error( 'EPICS base not set' )

f = open( 'configure/RELEASE.local', 'w' )
f.write( 'EPICS_BASE = {}\n'.format( args.epics_base ) )
if args.modules:
  for entry in args.modules:
    f.write( '{}\n'.format( entry ) )

f.close()

if args.iseghal:
  for entry in args.iseghal:
    try:
      target, hal_path = entry.split( '=' )
    except:
      parser.error( 'Invalid value for iseghal: {}. Use PATH|TAGETARCH'.format( entry ) )

    f = open( 'configure/RELEASE.Common.{}'.format( target ), 'w+' )
    f.write( 'ISEGHAL={}\n'.format( hal_path ) )
    f.close()

