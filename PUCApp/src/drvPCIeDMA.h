#include <stdlib.h>

typedef enum FpgaPcieParam_t {
	bar0,
	bar2,
	bar4,
	dmakm,
        DAdirectory,
	Waveform,
	dbtest,
    FpgaPcieLastParam,
} FpgaPcieParam_t;

#ifndef DRVASYNIPPORT_H
#define DRVASYNIPPORT_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

int drvPcieDMAConfigure(const char *portName, const char *hostInfo,
                           unsigned int priority, int noAutoConnect,
                           int noProcessEos);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif  /* DRVASYNIPPORT_H */
