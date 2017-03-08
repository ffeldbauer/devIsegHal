# #!/usr/bin/isegIOC

epicsEnvSet( "ARCH",                 "EPICS_HOST_ARCH" )
epicsEnvSet( "TOP",                  "/etc/epics/isegioc" )
epicsEnvSet( "EPICS_BASE",           "/etc/epics/base" )
epicsEnvSet( "IOCCONFPATH",          "${EPICS_IOC_CONFIG_PATH}" )

## Register all support components
dbLoadDatabase( "$(TOP)/dbd/isegIOC.dbd", 0, 0 )
isegIOC_registerRecordDeviceDriver( pdbbase )

## Load asyn drivers
#< $(IOCCONFPATH)/drivers

## Load record instances
dbLoadTemplate ("$(IOCCONFPATH)/substitutions" )

iocInit()

