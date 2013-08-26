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

int sendCommandEPICS(uint8_t *data, uint32_t count);

int sendBPM(uint8_t *data, uint32_t *count);

int sendPUC(uint8_t *data, uint32_t *count);

int recvCommandEPICS(uint8_t *data, uint32_t *count);

int sendCommandtest(uint8_t *data, uint32_t *count);

int receiveCommandtest(uint8_t *data, uint32_t *count);

void setEpicsuser(asynUser *us);
