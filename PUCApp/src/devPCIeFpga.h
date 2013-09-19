#include "asynPortDriver.h"
#include "epicsEvent.h"
#include "unionConversion.h"
#include "drvPCIeDMA.h"
#include "asynOctetSyncIO.h"
#define M_MAX_DATA	1

/* These are the drvInfo strings that are used to identify the parameters.
 * They are used by asyn clients, including standard asyn device support */

#define SDRAM_Acquire_value            	"SDRAM_ACQUIRE_SingleValue"                 /* asynInt32,    r/w */


/** Base class to control mythen **/
class devPCIeFpga : public asynPortDriver {
public:
    devPCIeFpga(const char *portName, const char *serialName);

    /* These are the methods that we override from asynPortDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);

protected:
    /** Values used for pasynUser->reason, and indexes into the parameter library. */
    #define FIRST_M_COMMAND SDRAM_Acquire
    int SDRAM_Acquire;
    #define LAST_M_COMMAND SDRAM_Acquire

private:
    asynUser *user;
    int timeout;

};


#define NUM_M_PARAMS (&LAST_M_COMMAND - &FIRST_M_COMMAND + 1)
