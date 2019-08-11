TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
devIsegHal_SYS_LIBS += isegHAL-client

ifneq ($(ISEGHAL),)
    USR_INCLUDES += -I$(ISEGHAL)
    USR_LDFLAGS  += -L$(ISEGHAL)
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

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

