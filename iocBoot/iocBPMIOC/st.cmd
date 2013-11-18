#!../../bin/linux-x86_64/PUC

## You may have to change PUC to something else
## everywhere it appears in this file

#epicsEnvSet("uCIP","$(uCIP=localhost:6791)")
epicsEnvSet("uCIP","$(uCIP=127.0.0.1:6791)")

epicsEnvSet("fpgahardwarecontroller","$(fpgahardwarecontroller=127.0.0.1:7000)")
epicsEnvSet("fpgahardwarecontrollercurve","$(fpgahardwarecontrollercurve=127.0.0.1:7001)")
< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/PUC.dbd"
PUC_registerRecordDeviceDriver pdbbase

# Load record instances

devFrontendConfigure("1", "$(uCIP)", 0x1,"front end");
dbLoadRecords("db/frontend.db","user=rootHost, PORT=1, TIMEOUT=5")

#devFrontendConfigure("2", "$(fpgahardwarecontroller)", 0x2,"fpga single data");
#dbLoadRecords("db/fpga.db","user=rootHost, PORT=2, TIMEOUT=15")

#devFrontendConfigure("3", "$(fpgahardwarecontrollercurve)", 0x3,"fpga curve");
#dbLoadRecords("db/fpga_curve.db","user=rootHost, PORT=3, TIMEOUT=15")

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=rootHost"
