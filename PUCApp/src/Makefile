TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = PUC
# PUC.dbd will be created and installed
DBD += PUC.dbd


# PUC.dbd will be made up from these files:
PUC_DBD += base.dbd
PUC_DBD += Frontend.dbd

#PUC_DBD += portInclude.dbd

# Include dbd files from all support applications:
#PUC_DBD += xxx.dbd

# Add all the support libraries needed by this IOC
#PUC_LIBS += xxx

# PUC_registerRecordDeviceDriver.cpp derives from PUC.dbd
PUC_SRCS += PUC_registerRecordDeviceDriver.cpp
PUC_SRCS += devFrontend.c
PUC_SRCS += sllp_client.c
PUC_SRCS += sendrecvlib.c
PUC_SRCS += frontendRecordParams.c


# Build the main IOC entry point on workstation OSs.
PUC_SRCS_DEFAULT += PUCMain.cpp
PUC_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#PUC_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
PUC_LIBS += $(EPICS_BASE_IOC_LIBS)
PUC_LIBS += asyn


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

