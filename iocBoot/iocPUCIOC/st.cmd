#!../../bin/linux-x86_64/PUC

## You may have to change PUC to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/PUC.dbd"
PUC_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/test.db","user=rootHost, PORT=port1, ADDRESS_VAL=20, ID_VAL=5, TYPE_VAL=0, SIZE_VAL=16386")
drvAsynIPPortConfigure("test", "localhost:6543", 0, 0, 0);
#drvAsynSerialPortConfigure("test", "/dev/ttyACM0",0,0,0)
portConnectConfigure("port1", "test")

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=rootHost"
