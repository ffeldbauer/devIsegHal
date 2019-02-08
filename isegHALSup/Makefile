TOP=..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

# The files in this directory are installed first because they are needed to build libisegHAL-client
INC += isegapi.h
INC += isegclientapi.h
INC += isegcommon.h

# Note, the following files were manually copied from the iSeg apalis image
# to this directory after building
ifeq (linux-arm-apalis, $(findstring linux-arm-apalis, $(T_A)))
#LIB_INSTALLS_Linux    += ../os/linux-arm-apalis/libisegHAL-client.so
#LIB_INSTALLS_Linux    += ../os/linux-arm-apalis/libisegHAL-client.so.1
#LIB_INSTALLS_Linux    += ../os/linux-arm-apalis/libisegHAL-client.so.1.1
#LIB_INSTALLS_Linux    += ../os/linux-arm-apalis/libisegHAL-client.so.1.1.2
#LIB_INSTALLS_Linux    += ../os/linux-arm-apalis/libisegHAL.so
#LIB_INSTALLS_Linux    += ../os/linux-arm-apalis/libisegHAL.so.1
#LIB_INSTALLS_Linux    += ../os/linux-arm-apalis/libisegHAL.so.1.1
#LIB_INSTALLS_Linux    += ../os/linux-arm-apalis/libisegHAL.so.1.1.2
endif

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

