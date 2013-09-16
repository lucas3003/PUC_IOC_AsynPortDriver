#!../../bin/linux-x86_64/PUC

## You may have to change PUC to something else
## everywhere it appears in this file

#epicsEnvSet("uCIP","$(uCIP=localhost:6791)")
epicsEnvSet("uCIP","$(uCIP=10.0.17.31:6791)")
< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/PUC.dbd"
PUC_registerRecordDeviceDriver pdbbase

# Load record instances
##devFrontendConfigure("1", "$(uCIP)", 0x1);
##dbLoadRecords("db/frontend.db","user=rootHost, PORT=1, TIMEOUT=5")
#drvAsynSerialPortConfigure("test", "/dev/ttyACM0",0,0,0)
drvPcieDMAConfigure(1,0,1,1,1)
#dbLoadRecords("db/fpgapcietest.db","user=rootHost, PORT=1, TIMEOUT=5")
cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=rootHost"
