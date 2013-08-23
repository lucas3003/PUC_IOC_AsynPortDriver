/************************************************************************\
* Copyright (c) 2011 Lawrence Berkeley National Laboratory, Accelerator
* Technology Group, Engineering Division
* This code is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/

#ifndef asynInterposeCom_H
#define asynInterposeCom_H
#define BPM
#define DEBUG
#include "sllp_client.h"
#include <shareLib.h>
#include <epicsExport.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

epicsShareFunc int devFrontendConfigure(const char *portName, const char *hostInfo,  int priority);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* asynInterposeCom_H */
