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
DBDINC += aCalcoutRecord
ifdef SSCAN
	DBDINC += swaitRecord
endif

# <name>.dbd will be created from <name>Include.dbd, if it exists
DBD += calc.dbd
DBD_INSTALLS += calcSupport.dbd

#=============================

LIBRARY_IOC += calc

calc_SRCS += transformRecord.c
calc_SRCS += sCalcPostfix.c sCalcPerform.c
calc_SRCS += aCalcPostfix.c aCalcPerform.c calcUtil.c
#calc_SRCS_vxWorks += test_sCalc.c
calc_SRCS += sCalcoutRecord.c devsCalcoutSoft.c
calc_SRCS += aCalcoutRecord.c devaCalcoutSoft.c
calc_SRCS += subAve.c

calc_DBD += base.dbd calcSupport_LOCAL.dbd

ifdef SSCAN
	calc_DBD += calcSupport_withSSCAN.dbd
	calc_SRCS += swaitRecord.c
	calc_LIBS += sscan
endif

ifdef GENSUB
	calc_DBD += calcSupport_withGENSUB.dbd
	calc_SRCS += interp.c
	calc_SRCS += arrayTest.c
	calc_LIBS += genSub
endif

# Register stuff
calc_SRCS += calcRegister.c

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

calcApp_LIBS += calc
ifdef SSCAN
	calcApp_LIBS += sscan
endif
ifdef GENSUB
	calcApp_LIBS += genSub
endif

#calcApp_LIBS += seq pv
calcApp_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# calcSupport.dbd is for use by an ioc directory that needs stuff from the calc module.
# We build this by hand so we can make it depend on the modules defined in configure/RELEASE
# and also leave it unexpanded.  (We can't expand, because that would require including base.dbd,
# and support modules mustn't do that.  Otherwise the ioc directory would inherit an expanded
# copy of base.dbd from each support module.)

ifdef SSCAN
ifdef GENSUB
calcSupport.dbd: ../calcSupport_LOCAL.dbd ../calcSupport_withSSCAN.dbd ../calcSupport_withGENSUB.dbd
	cat ../calcSupport_LOCAL.dbd ../calcSupport_withSSCAN.dbd ../calcSupport_withGENSUB.dbd >calcSupport.dbd
else
calcSupport.dbd: ../calcSupport_LOCAL.dbd ../calcSupport_withSSCAN.dbd
	cat ../calcSupport_LOCAL.dbd ../calcSupport_withSSCAN.dbd >calcSupport.dbd
endif
else
ifdef GENSUB
calcSupport.dbd: ../calcSupport_LOCAL.dbd ../calcSupport_withGENSUB.dbd
	cat ../calcSupport_LOCAL.dbd ../calcSupport_withGENSUB.dbd >calcSupport.dbd
else
calcSupport.dbd: ../calcSupport_LOCAL.dbd
	cat ../calcSupport_LOCAL.dbd >calcSupport.dbd
endif
endif
