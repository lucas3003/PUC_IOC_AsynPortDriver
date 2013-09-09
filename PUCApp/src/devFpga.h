#ifndef asynInterposeCom_H
#define asynInterposeCom_H
#define BPM
#define DEBUG
#define octettest
#include "sllp_client.h"
#include <shareLib.h>
#include <epicsExport.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

epicsShareFunc int devFpgaPcieConfigure(const char *portName, const char *hostInfo,  int priority,int device);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* asynInterposeCom_H */
