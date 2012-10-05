#ifndef __EmgDAQ_h__
#define __EmgDAQ_h__

#include	<NIDAQmx.h>
#include	"pthread.h"
#include    "Utilities.h"
#include    <windows.h>
#include    <ipp.h>

class TimeData
{
private:


public:
    LARGE_INTEGER tick0, tick1, tick2, frequency;
    float lce00, lce01, lce02;
    float lce10, lce11, lce12;
    float h1, h2;

    TimeData();
    //TODO: initialize tick0, tick1, tick2, etc.

    //
};

int const ORDER_VEL_LOWPASS = 2;
int const ORDER_FR_LOWPASS = 2;
const int lenFilter = 2 * (1 + ORDER_VEL_LOWPASS); // IIR, len = 2 * (1 + ORDER)
const int lenFilterFR = 2 * (1 + ORDER_FR_LOWPASS); // FIR, len = 1 + ORDER
extern Ipp32f *taps0FR, *taps0;
extern Ipp32f *taps1FR, *taps1;
extern Ipp32f *dly0FR, *dly0;
extern Ipp32f *dly1FR, *dly1;
extern IppsIIRState_32f *pIIRState0FR, *pIIRState1FR;
extern IppsIIRState_32f *pIIRState0, *pIIRState1;

extern int gFiringRateBic;
extern int gFiringRateTri;


extern float gAuxvar[];
extern TaskHandle gAOTaskHandle;
extern TaskHandle gEncoderHandle[];
extern int gCurrMotorState;


extern bool gIsRecording;
extern float gLenOrig[], gLenScale[], gMuscleLce[], gMuscleVel[];
extern double gEncoderCount[];

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandleDAQmx, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandleDAQmx, int32 status, void *callbackData);
int32 CVICALLBACK UpdatePxiData(TaskHandle taskHandleDAQmx, int32 signalID, void *callbackData);
//int32 CVICALLBACK update_dataEnableMotors(TaskHandle taskHandleDAQmxEnableMotors, int32 signalID, void *callbackData);


int StartReadPos(TaskHandle *rawHandle0,TaskHandle *rawHandle1);
int StopPositionRead(TaskHandle *rawHandle0,TaskHandle *rawHandle1);

int StartSignalLoop(TaskHandle *rawAOHandle, TaskHandle *rawForceHandle);
int StopSignalLoop(TaskHandle *rawAOHandle, TaskHandle *rawForceHandle);

int EnableMotors(TaskHandle *rawHandle);
int DisableMotors(TaskHandle *rawHandle);

extern FILE *gDataFile;
extern float64 gMotorCmd[];
extern float gCtrlFromFPGA[];

extern char gTimeStamp[20];

extern LARGE_INTEGER gInitTick, gCurrentTick, gClkFrequency;
#endif