TOP=..

include $(TOP)/configure/CONFIG

SRC_DIRS += $(TOP)/calcApp/src

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
