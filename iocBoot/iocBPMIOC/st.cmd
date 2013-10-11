#!../../bin/linux-x86_64/PUC

## You may have to change PUC to something else
## everywhere it appears in this file

#epicsEnvSet("uCIP","$(uCIP=localhost:6791)")
epicsEnvSet("uCIP","$(uCIP=10.0.17.32:6791)")

epicsEnvSet("fpgahardwarecontroller","$(fpgahardwarecontroller=10.0.18.48:7000)")
epicsEnvSet("fpgahardwarecontrollercurve","$(fpgahardwarecontrollercurve=10.0.18.48:7001)")
< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/PUC.dbd"
PUC_registerRecordDeviceDriver pdbbase

# Load record instances

#devFrontendConfigure("1", "$(uCIP)", 0x1,"front end");
#dbLoadRecords("db/frontend.db","user=rootHost, PORT=1, TIMEOUT=5")
#drvAsynSerialPortConfigure("test", "/dev/ttyACM0",0,0,0)

devFrontendConfigure("2", "$(fpgahardwarecontroller)", 0x2,"fpga single data");
dbLoadRecords("db/fpga.db","user=rootHost, PORT=2, TIMEOUT=5")


devFrontendConfigure("3", "$(fpgahardwarecontrollercurve)", 0x3,"fpga curve");
dbLoadRecords("db/fpga_curve.db","user=rootHost, PORT=3, TIMEOUT=5")

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=rootHost"
