TOP = ../..
include $(TOP)/configure/CONFIG

#Reset IOCSH definition so that not all shell scripts get installed

IOCSH = userCalc.iocsh
IOCSH += userCalc_extra.iocsh

ifdef SNCSEQ
	IOCSH += sseq.iocsh
endif

include $(TOP)/configure/RULES
