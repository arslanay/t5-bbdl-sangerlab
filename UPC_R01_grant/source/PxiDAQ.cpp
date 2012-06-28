#include	<stdio.h>
#include	<time.h>
#include	"Utilities.h"
#include    "PxiDAQ.h"
#define		DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

/*********************************************************************
*
* ANSI C Example program:
*    ContAcq-ExtClk-DigStart.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to acquire a continuous amount of
*    data using an external sample clock, started by a digital edge.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the minimum and maximum voltage ranges.
*    Note: For better accuracy try to match the Input Ranges to the
*          expected voltage level of the measured signal.
*    3. Select a source for the sample clock.
*    4. Set the approximate EMG_SAMPLING_RATE of the external clock. This allows
*       the internal characteristics of the acquisition to be as
*       efficient as possible. Also set the Samples to Read control.
*       This will determine how many samples are read at a time. This
*       also determines how many points are plotted on the graph each
*       time.
*    5. Select a source for the digital edge start trigger.
*    6. Select the edge, rising or falling, on which to trigger.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel.
*    3. Define the parameters for an External Clock Source.
*       Additionally, define the sample mode to be continuous. The
*       external clock rate is given to allow the internal
*       characteristics of the acquisition to be as efficient as
*       possible.
*    4. Set the parameters for a digital edge start trigger.
*    5. Call the Start function to start the acquisition.
*    6. Read the waveform data in the EveryNCallback function until
*       the user hits the stop button or an error occurs.
*    Note: This example reads data from one or more channels and
*          returns an array of data. Use the Index Array function to
*          access an individual channel of data.
*    7. Call the Clear Task function to clear the Task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O control. Also, make sure that your digital trigger
*    signal is connected to the terminal specified in Trigger Source.
*
*********************************************************************/
int StartEmg(TaskHandle taskHandleDAQmx)
{
	int32       error=0;
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandleDAQmx));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandleDAQmx,"PXI1Slot5/ai1","force0", DAQmx_Val_NRSE ,-5.0,5.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandleDAQmx,"PXI1Slot5/ai9","force1", DAQmx_Val_NRSE ,-5.0,5.0,DAQmx_Val_Volts,NULL));
	//DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandleDAQmx,"PXI1Slot5/ai2","flex2",DAQmx_Val_Diff ,-1.0,1.0,DAQmx_Val_Volts,NULL));
	//DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandleDAQmx,"PXI1Slot5/ai3","ext2",DAQmx_Val_Diff ,-1.0,1.0,DAQmx_Val_Volts,NULL));
	//DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandleDAQmx,"PXI1Slot5/ai4","flex3",DAQmx_Val_Diff ,-1.0,1.0,DAQmx_Val_Volts,NULL));
	//DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandleDAQmx,"PXI1Slot5/ai5","ext3",DAQmx_Val_Diff ,-1.0,1.0,DAQmx_Val_Volts,NULL));
	//DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandleDAQmx,"PXI1Slot5/ai6","flex4",DAQmx_Val_Diff ,-1.0,1.0,DAQmx_Val_Volts,NULL));
	//DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandleDAQmx,"PXI1Slot5/ai7","ext4",DAQmx_Val_Diff ,-10.0,10.0,DAQmx_Val_Volts,NULL));

	// DAQmxErrChk(TaskHandle, ChannelName, MaxExternalClkEMG_SAMPLING_RATE, DAQMode, BufferSize);
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandleDAQmx,"",1000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1));
	//DAQmxErrChk (DAQmxCreateCOPulseChanFreq(taskHandleDAQmx,"PXI1Slot3/Ctr4","",DAQmx_Val_Hz,DAQmx_Val_Rising,0.0,100.00,0.50));
	//DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandleDAQmx,"PXI1Slot3/Ctr4",DAQmx_Val_Hz,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));
	//DAQmxErrChk (DAQmxRegisterEveryNSamplesEvent(taskHandleDAQmx,DAQmx_Val_Acquired_Into_Buffer,100,0,EveryNCallback,NULL));
	DAQmxErrChk (DAQmxRegisterSignalEvent(taskHandleDAQmx,DAQmx_Val_SampleClock, 0, update_data, NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(taskHandleDAQmx,0,DoneCallback,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandleDAQmx));

	//printf("Ready for EMG recording!\n");
	//getchar();
	return 0;

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	return 0;
}

int StopEmg(TaskHandle taskHandleDAQmx)
{
	int32       error=0;
	char        errBuff[2048] = {'\0'};

	//printf( "\nStopping EMG ...\n" );
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandleDAQmx!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandleDAQmx);
		DAQmxClearTask(taskHandleDAQmx);
	}
    printf ("\nDAQ stopped!");
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	//fclose(emgLogHandle);
	//printf("\nStopped EMG !\n");
	return 0;
}

int32 CVICALLBACK update_data(TaskHandle taskHandleDAQmx, int32 signalID, void *callbackData)
{
	int32   error=0;
	float64   data[200]={0.0};
	int32   numRead;
	uInt32  i=0;
	char    buff[512], *buffPtr;
	char    errBuff[2048]={'\0'};
	char	*timeStr;
	time_t	currTime;

	if( taskHandleDAQmx ) {
		//time (&currTime);
		//timeStr = ctime(&currTime);
		//timeStr[strlen(timeStr)-1]='\0';  // Remove trailing newline.



		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		//DAQmxErrChk (DAQmxReadDigitalLines(taskHandle,1,10.0,DAQmx_Val_GroupByScanNumber,data,8,&numRead,NULL,NULL));
		DAQmxErrChk (DAQmxReadAnalogF64(taskHandleDAQmx,1,10.0,DAQmx_Val_GroupByScanNumber, data, 1*CHANNEL_NUM,&numRead,NULL));

		if( numRead ) {
//			printf("f1 %.4lf :: f2 %.4lf \n", data[0], data[1]);
			g_force[0] = data[0];
			g_force[1] = data[1];
		}
	}
	return 0;

Error:
	if( DAQmxFailed(error) )
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		DAQmxStopTask(taskHandleDAQmx);
		DAQmxClearTask(taskHandleDAQmx);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}
//
//int32 CVICALLBACK EveryNCallback(TaskHandle taskHandleDAQmx, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
//{
//	int32       error =0;
//	char        errBuff[2048] = {'\0'};
//	static int  totalRead = 0;
//	int			i = 0;
//	int32       read = 0;
//	float64     data[MAX_SAMPLE_NUM * CHANNEL_NUM];//MAX_SAMPLE_NUM
//
//	/*********************************************/
//	// DAQmx Read Code
//	/*********************************************/
//	DAQmxErrChk (DAQmxReadAnalogF64(taskHandleDAQmx,100,10.0,DAQmx_Val_GroupByScanNumber,&data[0],1*CHANNEL_NUM,&read,NULL));
//	// (taskHandleDAQmx, NumOfSamplesPerChann, TimeOut, FillMode, Where to store data, Size of data, Sample Num Read, NULL);
//
//	if( read>0 ) {
//		//printf("Acquired %d samples. Total %d\r",read,totalRead+=read);
//		for (i = 0; i < read; i++)
//		{
//			for (int j = 0; j <= CHANNEL_NUM - 2 ; j++)
//			{
//				fprintf(emgLogHandle, "%2.4f\t", data[i*CHANNEL_NUM + j]);
//			}
//			fprintf(emgLogHandle, "%2.4f\n", data[(i+1) * CHANNEL_NUM - 1]);
//		}
//		fflush(stdout);
//	}
//
//Error:
//	if( DAQmxFailed(error) ) {
//		DAQmxGetExtendedErrorInfo(errBuff,2048);
//		/*********************************************/
//		// DAQmx Stop Code
//		/*********************************************/
//		DAQmxStopTask(taskHandleDAQmx);
//		DAQmxClearTask(taskHandleDAQmx);
//		printf("DAQmx Error: %s\n",errBuff);
//	}
//	return 0;
//}

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
