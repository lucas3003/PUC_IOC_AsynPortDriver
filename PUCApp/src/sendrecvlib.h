#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <cantProceed.h>
#include <epicsStdio.h>
#include <epicsString.h>
#include <epicsThread.h>
#include <epicsTime.h>
#include <errlog.h>
#include "asynDriver.h"
#include <epicsExport.h>
#include <stdint.h>
#include "asynOctetSyncIO.h"
#include "devFrontend.h"


int sendCommandEPICSsg(uint8_t *data, uint32_t *count);

int recvCommandEPICSsg(uint8_t *data, uint32_t *count);

int sendCommandEPICScurve(uint8_t *data, uint32_t *count);

int recvCommandEPICScurve(uint8_t *data, uint32_t *count);

int recvCommandEPICSfrontend(uint8_t *data, uint32_t *count);

int sendCommandEPICSfrontend(uint8_t *data, uint32_t *count);

int sendCommandtest(uint8_t *data, uint32_t *count);

int receiveCommandtest(uint8_t *data, uint32_t *count);

void setEpicsuserfpgasg(asynUser *us);

void setEpicsuserfrontend(asynUser *us);

void setEpicsuserfpgacurve(asynUser *us);
