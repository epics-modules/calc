TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#PROD_HOST += caExample
#caExample_SRCS	+= caExample.c
#caExample_LIBS	+= ca
#caExample_LIBS	+= Com

ca_DIR           = $(EPICS_BASE_LIB)
Com_DIR          = $(EPICS_BASE_LIB)


#=============================
# xxxRecord.h will be created from xxxRecord.dbd

DBDINC += transformRecord
DBDINC += sCalcoutRecord
DBDINC += swaitRecord
DBDINC += genSubRecord

# <name>.dbd will be created from <name>Include.dbd
DBD += calc.dbd
DBD += calcSupport.dbd

#=============================

#PROD_IOC_vxWorks = calc

LIBRARY_IOC += calc

calc_SRCS += transformRecord.c
calc_SRCS += sCalcPostfix.c sCalcPerform.c
#calc_SRCS_vxWorks += test_sCalc.c
calc_SRCS += sCalcoutRecord.c
calc_SRCS += swaitRecord.c
calc_SRCS += genSubRecord.c
calc_SRCS += subAve.c

# gensub-based interpolation
calc_SRCS += interp.c

# Register stuff
calc_SRCS += calcRegister.c

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
calc_SRCS += calc_registerRecordDeviceDriver.cpp
#calc_SRCS_DEFAULT += calcMain.cpp #maybe only for win32
#calc_SRCS_vxWorks += -nil- #?

#The following adds support from base/src/vxWorks
#calc_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

### The following builds SNL programs as components of std
# NOTE: for suffix .st, code is run through cpp; for .stt, it's not
#calc_SRCS += calc.st
#
#calc_LIBS += seq pv
#seq_DIR    = $(SNCSEQ_LIB)
#pv_DIR     = $(SNCSEQ_LIB)

calc_LIBS += std
std_DIR = $(STD_LIB)
calc_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================
# build an ioc application for testing

PROD_IOC = calcApp

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
calcApp_SRCS_DEFAULT += calc_registerRecordDeviceDriver.cpp
#calcApp_SRCS_vxWorks += calcVX_registerRecordDeviceDriver.cpp
calcApp_SRCS_DEFAULT += calcMain.cpp
calcApp_SRCS_vxWorks += -nil-
#The following adds support from base/src/vxWorks
calcApp_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

calcApp_LIBS += calc std
#calcApp_LIBS += seq pv
calcApp_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

