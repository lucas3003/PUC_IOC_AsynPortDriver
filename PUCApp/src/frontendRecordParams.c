#include "frontendRecordParams.h"

asynStatus frontendparamProcess(asynUser *pasynUser, char *pstring, const char *drvInfo,const char **pptypeName, size_t *psize){
	int i=0;
	/*if (epicsStrCaseCmp("Test_Sdram",pstring) == 0){
		for (i=0; i<FPGA_N_PARAMS; i++) {
			pstring = FpgaParam[i].paramString;
			if (epicsStrCaseCmp(drvInfo, pstring) == 0) {
				pasynUser->reason = FpgaParam[i].paramEnum;
				if (pptypeName) *pptypeName = epicsStrDup(pstring);
				if (psize) *psize = sizeof(FpgaParam[i].paramEnum);
				asynPrint(pasynUser, ASYN_TRACE_FLOW,"drvUserCreate, command=%s\n", pstring);
            		return asynSuccess;
			}
		}
	}else if (epicsStrCaseCmp("Test_Curve",pstring) == 0){
		for (i=0; i<FPGA_CURVE_N_PARAMS; i++) {
			pstring = FpgaCurveParam[i].paramString;
			if (epicsStrCaseCmp(drvInfo, pstring) == 0) {
				pasynUser->reason = FpgaCurveParam[i].paramEnum;
				if (pptypeName) *pptypeName = epicsStrDup(pstring);
				if (psize) *psize = sizeof(FpgaCurveParam[i].paramEnum);
				asynPrint(pasynUser, ASYN_TRACE_FLOW,"drvUserCreate, command=%s\n", pstring);
            		return asynSuccess;
			}
		}
	}else{*/
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
	//}
	asynPrint(pasynUser, ASYN_TRACE_ERROR,"drvUserCreate, unknown command=%s\n",drvInfo);
	return asynError;
}
