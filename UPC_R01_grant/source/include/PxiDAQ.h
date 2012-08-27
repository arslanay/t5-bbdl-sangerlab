#ifndef __EmgDAQ_h__
#define __EmgDAQ_h__

#include	<NIDAQmx.h>
#include	"pthread.h"
#include    "Utilities.h"
#include    <windows.h>

extern float32 gAuxvar[];
extern TaskHandle gAOTaskHandle;
extern TaskHandle gEncoderHandle[];
extern int gCurrMotorState;

extern bool gIsRecording;
extern float32 gLenOrig[], gLenScale, gMuscleLce[], gMuscleVel[];

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandleDAQmx, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandleDAQmx, int32 status, void *callbackData);
int32 CVICALLBACK update_data(TaskHandle taskHandleDAQmx, int32 signalID, void *callbackData);
//int32 CVICALLBACK update_dataEnableMotors(TaskHandle taskHandleDAQmxEnableMotors, int32 signalID, void *callbackData);


int StartReadPos(TaskHandle *rawHandle0,TaskHandle *rawHandle1);
int StopPositionRead(TaskHandle *rawHandle0,TaskHandle *rawHandle1);

int StartSignalLoop(TaskHandle *rawAOHandle, TaskHandle *rawForceHandle);
int StopSignalLoop(TaskHandle *rawAOHandle, TaskHandle *rawForceHandle);

int EnableMotors(TaskHandle *rawHandle);
int DisableMotors(TaskHandle *rawHandle);

extern FILE *gDataFile;
extern float64 gMotorCmd[];
extern float32 gCtrlFromFPGA[];

extern char gTimeStamp[20];

extern LARGE_INTEGER gInitTick, gCurrentTick, gClkFrequency;
#endif