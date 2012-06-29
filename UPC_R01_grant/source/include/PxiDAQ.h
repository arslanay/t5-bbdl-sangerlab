#ifndef __EmgDAQ_h__
#define __EmgDAQ_h__

#include	<NIDAQmx.h>
#include	"pthread.h"

extern double g_force[2];
extern TaskHandle g_AOTaskHandle;

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandleDAQmx, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandleDAQmx, int32 status, void *callbackData);
int32 CVICALLBACK update_data(TaskHandle taskHandleDAQmx, int32 signalID, void *callbackData);
//int32 CVICALLBACK update_dataEnableMotors(TaskHandle taskHandleDAQmxEnableMotors, int32 signalID, void *callbackData);


//int StartEmg(TaskHandle taskHandleDAQmx);
int StartEmg(TaskHandle taskHandleDAQmx);
int StopEmg(TaskHandle taskHandleDAQmxs);
//int StopEmg(TaskHandle taskHandleDAQmx);
#endif