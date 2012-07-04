#ifndef __EmgDAQ_h__
#define __EmgDAQ_h__

#include	<NIDAQmx.h>
#include	"pthread.h"
#include    "Utilities.h"
#include    <windows.h>

extern double gAuxvar[NUM_AUXVAR];
extern TaskHandle gAOTaskHandle;
extern TaskHandle gEncoderHandle;
extern bool gIsWindingUp;
extern bool gIsRecording;
extern float64 gLenOrig, gLenScale, gMuscleLce;

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandleDAQmx, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandleDAQmx, int32 status, void *callbackData);
int32 CVICALLBACK update_data(TaskHandle taskHandleDAQmx, int32 signalID, void *callbackData);
//int32 CVICALLBACK update_dataEnableMotors(TaskHandle taskHandleDAQmxEnableMotors, int32 signalID, void *callbackData);


int StartPositionRead(TaskHandle *rawHandle);
int StopPositionRead(TaskHandle *rawHandle);

int StartSignalLoop(TaskHandle taskHandleDAQmx);
int StopSignalLoop(TaskHandle taskHandleDAQmxs);

int EnableMotors(TaskHandle *rawHandle);
int DisableMotors(TaskHandle *rawHandle);

extern FILE *gDataFile;
extern float64 gMotorCmd[];

extern LARGE_INTEGER gInitTick, gCurrentTick, gClkFrequency;
#endif