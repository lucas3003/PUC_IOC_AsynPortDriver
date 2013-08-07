#!../../bin/linux-x86_64/PUC

## You may have to change PUC to something else
## everywhere it appears in this file

epicsEnvSet("uCIP","$(uCIP=192.168.1.51:10001)")
< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/PUC.dbd"
PUC_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/frontend.db","user=rootHost, PORT=0, TIMEOUT=5")
devFrontendConfigure("0", "$(uCIP)", 0x1);
#drvAsynSerialPortConfigure("test", "/dev/ttyACM0",0,0,0)

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=rootHost"
