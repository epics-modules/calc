#Makefile at top of application tree
TOP = .
include $(TOP)/configure/CONFIG
DIRS += configure calcApp tests
calcApp_DEPEND_DIRS = configure
tests_DEPEND_DIRS = configure calcApp
include $(TOP)/configure/RULES_TOP
