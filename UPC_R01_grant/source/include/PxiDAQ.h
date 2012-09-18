#ifndef __EmgDAQ_h__
#define __EmgDAQ_h__

#include	<NIDAQmx.h>
#include	"pthread.h"
#include    "Utilities.h"
#include    <windows.h>


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

extern float gAuxvar[];
extern TaskHandle gAOTaskHandle;
extern TaskHandle gEncoderHandle[];
extern int gCurrMotorState;


extern bool gIsRecording;
extern float gLenOrig[], gLenScale[], gMuscleLce[], gMuscleVel[];
extern double gEncoderTick[];
extern float dEncoderTicksFilteredQueue0[];
extern float dEncoderTicksFilteredQueue1[];
extern float dEncoderTicksQueue0[];
extern float dEncoderTicksQueue1[];
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