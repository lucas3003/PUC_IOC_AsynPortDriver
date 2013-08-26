#include "frontendRecordParams.h"

asynStatus frontendparamProcess(asynUser *pasynUser, char *pstring, const char *drvInfo,const char **pptypeName, size_t *psize){
	int i=0;
	for (i=0; i<FRONTEND_N_PARAMS; i++) {
		pstring = FrontendParam[i].paramString;
		if (epicsStrCaseCmp(drvInfo, pstring) == 0) {
			pasynUser->reason = FrontendParam[i].paramEnum;
			if (pptypeName) *pptypeName = epicsStrDup(pstring);
			if (psize) *psize = sizeof(FrontendParam[i].paramEnum);
			asynPrint(pasynUser, ASYN_TRACE_FLOW,"drvUserCreate, command=%s\n", pstring);
            		return asynSuccess;
		}
	}
	asynPrint(pasynUser, ASYN_TRACE_ERROR,"drvUserCreate, unknown command=%s\n",drvInfo);
	return asynError;
}
