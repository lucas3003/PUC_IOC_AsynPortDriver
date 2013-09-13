#include <stdlib.h>
#include <stdio.h>
#include <epicsString.h>
#include <epicsStdio.h>
#include "asynDriver.h"
/** Number of asyn parameters (asyn commands) this driver supports. */
#define FPGA_PCIE_N_PARAMS 7

/** Specific asyn commands for this support module. These will be used and
 * managed by the parameter library (part of areaDetector). */
typedef enum FpgaPcieParam_t {
	bar0,
	bar1,
	dmakm,
        DAdirectory,
	Waveform,
	dbtest,
    FpgaPcieLastParam,
} FpgaPcieParam_t;

typedef struct{
	FpgaPcieParam_t paramEnum;
	char *paramString;
}FpgaPcieParamStruct;

static FpgaPcieParamStruct FpgaPcieParam[FPGA_PCIE_N_PARAMS] = {
	{bar0,      "bar0"},
	{bar1,      "bar1"},
	{dmakm,     "dmakm"},
	{DAdirectory,"DAdirectory"},
	{Waveform,   "Waveform"},
	{dbtest,     "dbtest"}
};

asynStatus FpgaPcieparamProcess(asynUser *pasynUser, char *pstring, const char *drvInfo,const char **pptypeName, size_t *psize);
