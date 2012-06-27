#ifndef __EmgDAQ_h__
#define __EmgDAQ_h__

#include	<NIDAQmx.h>

extern TaskHandle PxiDAQHandle;
extern TaskHandle startleDaqHandle;


int32 CVICALLBACK EveryNCallback(TaskHandle taskHandleDAQmx, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandleDAQmx, int32 status, void *callbackData);

int StartEmg(void);
int StopEmg(void);
#endif