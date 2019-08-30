TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
devIsegHal_SYS_LIBS += isegHAL-client
isegIoc_SYS_LIBS += isegHAL-client

ifneq ($(ISEGHAL),)
    USR_INCLUDES += -I$(ISEGHAL) -I$(ISEGHAL)/include
    USR_LDFLAGS  += -L$(ISEGHAL) -L$(ISEGHAL)/lib
endif
ifneq ($(CHECK_TIMESTAMPS),)
    USR_CXXFLAGS += -DCHECK_LAST_REFRESHED
endif

#==================================================
# build a support library

LIBRARY_IOC += devIsegHal

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install devIsegHal.dbd into <top>/dbd
DBD += devIsegHal.dbd

# specify all source files to be compiled and added to the library
devIsegHal_SRCS += devIsegHalAi.c
devIsegHal_SRCS += devIsegHalAo.c
devIsegHal_SRCS += devIsegHalAsync.c
devIsegHal_SRCS += devIsegHalBi.c
devIsegHal_SRCS += devIsegHalBo.c
devIsegHal_SRCS += devIsegHal.cpp
devIsegHal_SRCS += devIsegHalGlobalSwitchBo.c
devIsegHal_SRCS += devIsegHalLi.c
devIsegHal_SRCS += devIsegHalLo.c
devIsegHal_SRCS += devIsegHalMbbid.c
devIsegHal_SRCS += devIsegHalStringin.c
devIsegHal_SRCS += devIsegHalStringout.c

devIsegHal_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================
# build an ioc application
PROD_IOC = isegIoc

DBD += isegIoc.dbd

isegIoc_DBD += base.dbd
isegIoc_DBD += devIsegHal.dbd

isegIoc_LIBS += devIsegHal

ifneq ($(AUTOSAVE),)
    isegIoc_DBD += as.dbd
    isegIoc_LIBS += autosave
endif
ifneq ($(CALC),)
    isegIoc_DBD += calc.dbd
    isegIoc_LIBS += calc
endif

# isegIoc_registerRecordDeviceDriver.cpp derives from isegIoc.dbd
isegIoc_SRCS += isegIoc_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
isegIoc_SRCS_DEFAULT += isegIocMain.cpp
isegIoc_SRCS_vxWorks += -nil-

# Link QSRV (pvAccess Server) if available
ifdef EPICS_QSRV_MAJOR_VERSION
    isegIoc_LIBS += qsrv
    isegIoc_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)
    isegIoc_DBD += PVAServerRegister.dbd
    isegIoc_DBD += qsrv.dbd
endif

isegIoc_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

