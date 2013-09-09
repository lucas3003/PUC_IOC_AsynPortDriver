#include "FpgaPcieRecordParams.h"

asynStatus FpgaPcieparamProcess(asynUser *pasynUser, char *pstring, const char *drvInfo,const char **pptypeName, size_t *psize){
	int i=0;
	for (i=0; i<FPGA_PCIE_N_PARAMS; i++) {
		pstring = FpgaPcieParam[i].paramString;
		if (epicsStrCaseCmp(drvInfo, pstring) == 0) {
			pasynUser->reason = FpgaPcieParam[i].paramEnum;
			if (pptypeName) *pptypeName = epicsStrDup(pstring);
			if (psize) *psize = sizeof(FpgaPcieParam[i].paramEnum);
			asynPrint(pasynUser, ASYN_TRACE_FLOW,"drvUserCreate, command=%s\n", pstring);
            		return asynSuccess;
		}
	}
	asynPrint(pasynUser, ASYN_TRACE_ERROR,"drvUserCreate, unknown command=%s\n",drvInfo);
	return asynError;
}
