#!../../bin/linux-x86_64/PUC

## You may have to change PUC to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/PUC.dbd"
PUC_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/frontendv0.db","user=rootHost, PORT=0, TIMEOUT=5")
drvAsynIPPortConfigure("test", "localhost:6791", 0, 0, 0);
#drvAsynSerialPortConfigure("test", "/dev/ttyACM0",0,0,0)
portConnectConfigure("0", "test")

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=rootHost"
