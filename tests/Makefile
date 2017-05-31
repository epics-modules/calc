#*************************************************************************
# Copyright (c) 2006 The University of Chicago, as Operator of Argonne
#     National Laboratory.
# Copyright (c) 2002 The Regents of the University of California, as
#     Operator of Los Alamos National Laboratory.
# EPICS BASE is distributed subject to a Software License Agreement found
# in file LICENSE that is included with this distribution.
#*************************************************************************
TOP=..

include $(TOP)/configure/CONFIG

INC_DIRS += $(TOP)/calcApp/src

TESTPROD_HOST += acalcTest
acalcTest_SRCS += acalcTest.cpp
testHarness_SRCS += acalcTest.cpp
TESTS += acalcTest

TESTPROD_HOST += scalcTest
scalcTest_SRCS += scalcTest.cpp
testHarness_SRCS += scalcTest.cpp
TESTS += scalcTest

testHarness_SRCS += runTests.cpp

PROD_vxWorks = calcTestHarness
calcTestHarness_SRCS += $(testHarness_SRCS)
TESTSPEC_vxWorks = calcTestHarness.munch; runTests

PROD_LIBS += calc
PROD_LIBS += $(EPICS_BASE_IOC_LIBS)

TESTSCRIPTS_HOST += $(TESTS:%=%.t)

include $(TOP)/configure/RULES
