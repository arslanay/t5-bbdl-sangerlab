#include	<stdio.h>
#include	<time.h>
#include	"Utilities.h"
#include    "PxiDAQ.h"
#include    <math.h>
#define		DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int StartSignalLoop(TaskHandle ForceReadTaskHandle)
{
	int32       error=0;
	char        errBuff[2048]={'\0'};
    
	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&ForceReadTaskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(ForceReadTaskHandle,"PXI1Slot5/ai1","force0", DAQmx_Val_NRSE ,-5.0,5.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(ForceReadTaskHandle,"PXI1Slot5/ai9","force1", DAQmx_Val_NRSE ,-5.0,5.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(ForceReadTaskHandle,"",1000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1));
    
    DAQmxErrChk (DAQmxCreateTask("",&g_AOTaskHandle));
    DAQmxErrChk (DAQmxCreateAOVoltageChan(g_AOTaskHandle,"PXI1Slot2/ao11","motor1", -5.0,5.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(g_AOTaskHandle,"",1000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1));
	

    DAQmxErrChk (DAQmxRegisterSignalEvent(ForceReadTaskHandle,DAQmx_Val_SampleClock, 0, update_data ,NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(ForceReadTaskHandle,0,DoneCallback,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(ForceReadTaskHandle));
	DAQmxErrChk (DAQmxStartTask(g_AOTaskHandle));




	//printf("Ready for EMG recording!\n");
	//getchar();
	return 0;

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	
    if( DAQmxFailed(error) )
		printf("StartSignalLoop Error: %s\n",errBuff);
	return 0;
}

int StopSignalLoop(TaskHandle ForceReadTaskHandle)
{
	int32       error=0;
	char        errBuff[2048] = {'\0'};
    const float64     ZERO_VOLTS[1]={0.0};

    DAQmxErrChk (DAQmxWriteAnalogF64(g_AOTaskHandle, 1, TRUE, 10.0, DAQmx_Val_GroupByChannel, ZERO_VOLTS, NULL, NULL));
    
	//printf( "\nStopping EMG ...\n" );
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( ForceReadTaskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(ForceReadTaskHandle);
        DAQmxStopTask(g_AOTaskHandle);
		DAQmxClearTask(ForceReadTaskHandle);
		DAQmxClearTask(g_AOTaskHandle);
	}
Error:
	if( DAQmxFailed(error) )
		printf("StopSignalLoop Error: %s\n",errBuff);
	//fclose(emgLogHandle);
	//printf("\nStopped EMG !\n");
	return 0;
}

int32 CVICALLBACK update_data(TaskHandle taskHandleDAQmx, int32 signalID, void *callbackData)
{
	int32   error=0;
	float64 loadcell_data[200]={0.0};
    float64 encoder_data[200]={0.0};
	int32   numRead;
	uInt32  i=0;
	char    buff[512], *buffPtr;
	char    errBuff[2048]={'\0'};
	char	*timeStr;
	time_t	currTime;
    float64     AOdata[1]={1.7};



	if( taskHandleDAQmx ) {
		//time (&currTime);
		//timeStr = ctime(&currTime);
		//timeStr[strlen(timeStr)-1]='\0';  // Remove trailing newline.

		/*********************************************/
		// DAQmx Read Code
		/*********************************************/

        DAQmxErrChk (DAQmxReadAnalogF64(taskHandleDAQmx,1,10.0,DAQmx_Val_GroupByScanNumber, loadcell_data, 1*CHANNEL_NUM,&numRead,NULL));
        AOdata[0] = fabs(g_auxvar[0]) * 1.1;
        DAQmxErrChk (DAQmxWriteAnalogF64(g_AOTaskHandle, 1, TRUE, 10.0, DAQmx_Val_GroupByChannel, AOdata, NULL, NULL));

		if( numRead ) {
//			printf("f1 %.4lf :: f2 %.4lf \n", data[0], data[1]);
			g_auxvar[0] = loadcell_data[0];
			g_auxvar[1] = loadcell_data[1];
		}

		DAQmxErrChk (DAQmxReadCounterF64(g_PositionRead,1,10.0,encoder_data,1,&numRead,0));        
		if( numRead ) {
//			printf("f1 %.4lf :: f2 %.4lf \n", data[0], data[1]);
			g_auxvar[2] = encoder_data[0];
			g_auxvar[3] = encoder_data[1];
		}

	}
    

	return 0;

Error:
	if( DAQmxFailed(error) )
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		DAQmxStopTask(taskHandleDAQmx);
		DAQmxStopTask(g_AOTaskHandle);
		DAQmxClearTask(taskHandleDAQmx);
		DAQmxClearTask(g_AOTaskHandle);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}


int32 CVICALLBACK DoneCallback(TaskHandle taskHandleDAQmx, int32 status, void *callbackData)
{
	int32   error=0;
	char    errBuff[2048]={'\0'};

	// Check to see if an error stopped the task.
	DAQmxErrChk (status);

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		DAQmxClearTask(taskHandleDAQmx);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}

int EnableMotors(TaskHandle *rawHandle)
{
    TaskHandle  motorTaskHandle = *rawHandle;
	int32       error=0;
	char        errBuff[2048]={'\0'};
    uInt32      dataEnable=0xffffffff;
    uInt32      dataDisable=0x00000000;
    int32		written;

	DAQmxErrChk (DAQmxCreateTask("",&motorTaskHandle));
    DAQmxErrChk (DAQmxCreateDOChan(motorTaskHandle,"PXI1Slot2/port0","enable07",DAQmx_Val_ChanForAllLines));
	DAQmxErrChk (DAQmxStartTask(motorTaskHandle));
   	DAQmxErrChk (DAQmxWriteDigitalU32(motorTaskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,&dataEnable,&written,NULL));

	*rawHandle = motorTaskHandle;

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	
    if( DAQmxFailed(error) )
		printf("EnableMotor Error: %s\n",errBuff);
	return 0;
}

int DisableMotors(TaskHandle *rawHandle)
{
    TaskHandle motorTaskHandle = *rawHandle;

	int32       error=0;
	char        errBuff[2048] = {'\0'};
    uInt32      dataDisable=0x00000000;
    int32		written;

    DAQmxErrChk (DAQmxWriteDigitalU32(motorTaskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,&dataDisable,&written,NULL));

	printf( "\nStopping Enable ...\n" );

	if( motorTaskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(motorTaskHandle);
		DAQmxClearTask(motorTaskHandle);
	}

    *rawHandle = motorTaskHandle;
    return 0;

Error:
	if( DAQmxFailed(error) )
		printf("DisableMotor Error: %s\n",errBuff);
	//fclose(emgLogHandle);
	//printf("\nStopped EMG !\n");
	return 0;
}


int StartPositionRead(TaskHandle *rawHandle)
{
	TaskHandle  encoderTaskHandle = *rawHandle;
	int32       error=0;
	char        errBuff[2048]={'\0'};
    uInt32      dataEnable=0xffffffff;
    uInt32      dataDisable=0x00000000;

    int32		written;

    //DAQmxLoadTask ("EncoderSlot3Ctr3",&encoderTaskHandle);

    DAQmxCreateTask ("",&encoderTaskHandle);
    DAQmxErrChk (DAQmxCreateCIAngEncoderChan(encoderTaskHandle,"PXI1Slot3/ctr3","",DAQmx_Val_X4,0,0.0,DAQmx_Val_AHighBHigh,DAQmx_Val_Degrees,24,0.0,""));

	DAQmxErrChk (DAQmxStartTask(encoderTaskHandle));

	*rawHandle = encoderTaskHandle;

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	
    if( DAQmxFailed(error) )
		printf("EnableEncoder Error: %s\n",errBuff);
	return 0;
}

int StopPositionRead(TaskHandle *rawHandle)
{
    TaskHandle encoderTaskHandle = *rawHandle;

	int32       error=0;
	char        errBuff[2048] = {'\0'};
    uInt32      dataDisable=0x00000000;
    int32		written;

	printf( "\nStopping Encoder ...\n" );

	if( encoderTaskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(encoderTaskHandle);
		DAQmxClearTask(encoderTaskHandle);
	}

    *rawHandle = encoderTaskHandle;
    return 0;

Error:
	if( DAQmxFailed(error) )
		printf("DisableEncoder Error: %s\n",errBuff);
	//fclose(emgLogHandle);
	//printf("\nStopped EMG !\n");
	return 0;
}
