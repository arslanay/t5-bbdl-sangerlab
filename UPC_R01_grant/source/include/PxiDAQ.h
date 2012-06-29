#ifndef __EmgDAQ_h__
#define __EmgDAQ_h__

#include	<NIDAQmx.h>
#include	"pthread.h"
#include    "Utilities.h"

extern double g_auxvar[NUM_AUXVAR];
extern TaskHandle g_AOTaskHandle;
extern TaskHandle g_PositionRead;

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

#endif