#include <stdlib.h>
#include <stdio.h>
#include <epicsString.h>
#include <epicsStdio.h>
#include "asynDriver.h"
/** Number of asyn parameters (asyn commands) this driver supports. */
#define FRONTEND_N_PARAMS 10
#define FPGA_SINGLE_N_PARAMS 2
#define FPGA_CURVE_N_PARAMS 2

typedef enum DeviceType_t {
	FPGA_CURVE,
	FPGA_SINGLE_DATA,
	FRONTEND
} DeviceType_t;	

/** Specific asyn commands for this support module. These will be used and
 * managed by the parameter library (part of areaDetector). */
typedef enum FrontendParam_t {
	t_setpoint1         /** Temperature setpoint*/,
	t_setpoint2         /** Temperature setpoint*/,
	t_setpoint3         /** Temperature setpoint*/,
	t_setpoint4         /** Temperature setpoint*/,
	t_sensor1   /** Temeperature sensor 1*/,	
	t_sensor2 /** Temperature sensor 2*/,	
	t_sensor3 /** Temperature sensor 3*/,	
	t_sensor4    /** Temeperature sensor 4*/,	
	c1_switchstate      /** Switch state*/,
				
    FrontendLastParam
} FrontendParam_t;

typedef struct{
	FrontendParam_t paramEnum;
	char *paramString;
}FrontendParamStruct;

static FrontendParamStruct FrontendParam[FRONTEND_N_PARAMS] = {
	{t_setpoint1,      "T_SetPoint1"},
	{t_setpoint2,      "T_SetPoint2"},
	{t_setpoint3,      "T_SetPoint3"},
	{t_setpoint4,      "T_SetPoint4"},
	{t_sensor1,         "T_Sensor1"},
	{t_sensor2,  "T_Sensor2"},
	{t_sensor3,   "T_Sensor3"},
	{t_sensor4, "T_Sensor4"},
	{c1_switchstate, "S_State"},
};

typedef enum FpgaSingleParam_t {
	test_sdram,
	FpgaLastParam
} FpgaSingleParam_t;

typedef struct {
	FpgaSingleParam_t paramEnum;
	char *paramString;
}FpgaSingleParamStruct;

static FpgaSingleParamStruct FpgaSingleParam[FPGA_SINGLE_N_PARAMS] = {
	{test_sdram, "Test_Sdram"}
};

typedef enum FpgaCurveParam_t {
	test_curve,
	FpgaCurveLastParam
} FpgaCurveParam_t;

typedef struct {
	FpgaCurveParam_t paramEnum;
	char *paramString;
}FpgaCurveParamStruct;

static FpgaCurveParamStruct FpgaCurveParam[FPGA_CURVE_N_PARAMS] = {
	{test_curve, "Test_Curve"}
};

