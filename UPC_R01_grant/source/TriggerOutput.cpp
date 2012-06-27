#include	<stdio.h>
#include	<math.h>
#include	"Utilities.h"
#include    "TriggerOutput.h"
#define		DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else
extern float64 gStartleData[400];
extern int error;
extern TaskHandle startleDaqHandle;
extern char        errBuff[2048];

int
ConfigStartle(void)
{
	int i;
	for(i = 0;i<250;i++)
	{
		gStartleData[i] = 5.0 ;//* sin(i / 5000.0 * 1400.0 * 2.0 * PI);//(i / 1000 * 400 * 2 * PI);
//		printf("%lf, ", gStartleData[i]);
	}
	for (i = 250; i < 400; i++)
		gStartleData[i] = 0.0;
	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&startleDaqHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(startleDaqHandle,"Dev1/ao0","",-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(startleDaqHandle,"",5000.0,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,400));
Error:
	return 0;
}

int
DoneStartle(void)
{
	if( startleDaqHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(startleDaqHandle);
		DAQmxClearTask(startleDaqHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
Error:
    return 0;
}

int 
StartStartle(void)
{

	int			i=0;
	int32   	written;


	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxWriteAnalogF64(startleDaqHandle,400,0,10.0,DAQmx_Val_GroupByChannel,gStartleData,&written,NULL);

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxStartTask(startleDaqHandle);
	/*********************************************/
	// DAQmx Wait Code
	/*********************************************/
	//DAQmxWaitUntilTaskDone(startleDaqHandle,10.0);
	return 0;
}
