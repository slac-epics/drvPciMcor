TOP=..

include $(TOP)/configure/CONFIG

#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#USR_CFLAGS += 

#=============================

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord

#=============================

LIBRARY  = drvPciMcor
PROD_IOC = pciMcorDemo
# <name>.dbd will be created from <name>Include.dbd
DBD		 = drvPciMcor.dbd
DBD     += pciMcorDemo.dbd

pciMcorDemo_DBD  = base.dbd
pciMcorDemo_DBD += epicspci.dbd
pciMcorDemo_DBD += devBusMapped.dbd
pciMcorDemo_DBD += drvUioPciGen.dbd

drvPciMcor_SRCS += drvPciMcor.c

pciMcorDemo_SRCS = pciMcorDemo_registerRecordDeviceDriver.cpp
pciMcorDemo_SRCS_DEFAULT += pciMcorDemoMain.c
pciMcorDemo_SRCS_RTEMS   += -nil-
pciMcorDemo_SRCS_vxWorks += -nil-

pciMcorDemo_LIBS = $(EPICS_BASE_IOC_LIBS)
pciMcorDemo_LIBS+= drvUioPciGen
pciMcorDemo_LIBS+= devBusMapped
pciMcorDemo_LIBS+= epicspci
pciMcorDemo_LIBS+= $(CEXP_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
