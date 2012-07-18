#include	<stdio.h>
#include	<time.h>
#include	"Utilities.h"
#include    "PxiDAQ.h"
#include    <math.h>
#define		DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else



int StartSignalLoop(TaskHandle *rawAOHandle, TaskHandle *rawForceHandle)
{
    TaskHandle AOHandle = *rawAOHandle;
    TaskHandle ForceReadTaskHandle = *rawForceHandle;
	int32       error=0;
	char        errBuff[2048]={'\0'};
    

    gDataFile = fopen("TestRec06.txt","a");


	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&ForceReadTaskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(ForceReadTaskHandle,"PXI1Slot5/ai1","force0", DAQmx_Val_NRSE ,-5.0,5.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(ForceReadTaskHandle,"PXI1Slot5/ai9","force1", DAQmx_Val_NRSE ,-5.0,5.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(ForceReadTaskHandle,"",1000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1));
    
    DAQmxErrChk (DAQmxCreateTask("",&AOHandle));
    DAQmxErrChk (DAQmxCreateAOVoltageChan(AOHandle,"PXI1Slot2/ao11","motor1", -5.0,5.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(AOHandle,"",1000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1));
	

    DAQmxErrChk (DAQmxRegisterSignalEvent(ForceReadTaskHandle,DAQmx_Val_SampleClock, 0, update_data ,NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(ForceReadTaskHandle,0,DoneCallback,NULL));
    
    QueryPerformanceCounter(&gInitTick);
	QueryPerformanceFrequency(&gClkFrequency);


	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
    if (0 != ForceReadTaskHandle) // Some error occurred
    {    
        // Sequence SENSITIVE: Start the tasks at the very end
        *rawForceHandle = ForceReadTaskHandle;
        *rawAOHandle = AOHandle;

        DAQmxErrChk (DAQmxStartTask(ForceReadTaskHandle));
        DAQmxErrChk (DAQmxStartTask(AOHandle));
        
    }
    else
    {
        //printf("Ready for EMG recording!\n");
        //getchar();
        *rawForceHandle = 0;
        *rawAOHandle = 0;
    }
	return 0;

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	
    if( DAQmxFailed(error) )
		printf("StartSignalLoop Error: %s\n",errBuff);
	return 0;
}

int StopSignalLoop(TaskHandle *rawAOHandle, TaskHandle *rawForceHandle)
{
    TaskHandle AOHandle = *rawAOHandle;
    TaskHandle ForceReadTaskHandle = *rawForceHandle;
	int32       error=0;
	char        errBuff[2048] = {'\0'};
    const float64     ZERO_VOLTS[1]={0.0};
    
    fclose(gDataFile);

    DAQmxErrChk (DAQmxWriteAnalogF64(AOHandle, 1, TRUE, 10.0, DAQmx_Val_GroupByChannel, ZERO_VOLTS, NULL, NULL));
    
	//printf( "\nStopping EMG ...\n" );
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( ForceReadTaskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(ForceReadTaskHandle);
        DAQmxStopTask(AOHandle);
		DAQmxClearTask(ForceReadTaskHandle);
		DAQmxClearTask(AOHandle);
        *rawForceHandle = 0;
        *rawAOHandle = 0;
	}
    else
    {
        *rawForceHandle = ForceReadTaskHandle;
        *rawAOHandle = AOHandle;
    }
Error:
	if( DAQmxFailed(error) )
		printf("StopSignalLoop Error: %s\n",errBuff);
	//fclose(emgLogHandle);
	//printf("\nStopped EMG !\n");
	return 0;
}

inline void LogData( void)
{
    double actualTime;
    QueryPerformanceCounter(&gCurrentTick);
    actualTime = gCurrentTick.QuadPart - gInitTick.QuadPart;
    actualTime /= gClkFrequency.QuadPart;
    if (gIsRecording)
    {
        fprintf(gDataFile,"%.3lf\t",actualTime );																			//1
        fprintf(gDataFile,"%f\t%f\t%f\t%f\t", gAuxvar[0], gMuscleLce, gMotorCmd[0],gCtrlFromFPGA[0]);										//2,3
        fprintf(gDataFile,"\n");

        //printf("%lf\t",actualTime );																			//1
        //printf("%f\t%f\t", gAuxvar[0], gAuxvar[2]);										//2,3
        //printf("\n");
    }
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
        double motor_cmd;
        switch(gCurrMotorState)
        {
        case MOTOR_STATE_INIT:
            motor_cmd = ZERO_MOTOR_VOLTAGE;
            break;
        case MOTOR_STATE_WINDING_UP:
            motor_cmd = SAFE_MOTOR_VOLTAGE;
            break;
        case MOTOR_STATE_OPEN_LOOP:
            motor_cmd = SAFE_MOTOR_VOLTAGE;
            break;
        case MOTOR_STATE_CLOSED_LOOP:
            motor_cmd = SAFE_MOTOR_VOLTAGE + gCtrlFromFPGA[0] * 0.5;
            break;
        case MOTOR_STATE_SHUTTING_DOWN:
            motor_cmd = ZERO_MOTOR_VOLTAGE;
            break;
        default:
            motor_cmd = ZERO_MOTOR_VOLTAGE;
        }

        //AOdata[0] = (motor_cmd > MAX_VOLT) ? MAX_VOLT : motor_cmd;
        AOdata[0] = (motor_cmd > MAX_VOLT) ? MAX_VOLT : ( (motor_cmd < 0.0 ) ? 0.0 :motor_cmd) ;
        //printf("\nAO handle = %d \n", gAOTaskHandle);
        DAQmxErrChk (DAQmxWriteAnalogF64(gAOTaskHandle, 1, TRUE, 10.0, DAQmx_Val_GroupByChannel, AOdata, NULL, NULL));

		if( numRead ) {
//			printf("f1 %.4lf :: f2 %.4lf \n", data[0], data[1]);
			gAuxvar[0] = loadcell_data[0];
			gAuxvar[1] = loadcell_data[1];
		}

		DAQmxErrChk (DAQmxReadCounterF64(gEncoderHandle,1,10.0,encoder_data,1,&numRead,0));        
		if( numRead ) {
//			printf("f1 %.4lf :: f2 %.4lf \n", data[0], data[1]);
			gAuxvar[2] = encoder_data[0];
			gAuxvar[3] = encoder_data[1];
		}

        gMuscleLce = -gLenScale * (-gAuxvar[2] + gLenOrig) + 1.0;
		//printf("\n\t%f",gMuscleLce); 
        LogData();


	}
    

	return 0;

Error:
	if( DAQmxFailed(error) )
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		DAQmxStopTask(taskHandleDAQmx);
		DAQmxStopTask(gAOTaskHandle);
		DAQmxClearTask(taskHandleDAQmx);
		DAQmxClearTask(gAOTaskHandle);
		printf("UpdateData Error: %s\n",errBuff);
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
    
	if( motorTaskHandle!=0 ) {
        DAQmxErrChk (DAQmxWriteDigitalU32(motorTaskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,&dataDisable,&written,NULL));
	    //printf( "\nStopping Enable ...\n" );
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(motorTaskHandle);
		DAQmxClearTask(motorTaskHandle);
        *rawHandle = 0;
	}  
    else
    {
        *rawHandle = motorTaskHandle;
    }
    return 0;

Error:
	if( DAQmxFailed(error) )
		printf("DisableMotor Error: %s\n",errBuff);
	//fclose(emgLogHandle);
	//printf("\nStopped EMG !\n");
	return 0;
}


int StartReadPos(TaskHandle *rawHandle)
{
	TaskHandle  encoderTaskHandle = *rawHandle;
	int32       error=0;
	char        errBuff[2048]={'\0'};
    uInt32      dataEnable=0xffffffff;
    uInt32      dataDisable=0x00000000;

    int32		written;
    DAQmxCreateTask ("",&encoderTaskHandle);
    //printf("IN");
    //DAQmxLoadTask ("EncoderSlot3Ctr3",&encoderTaskHandle);
	if( encoderTaskHandle!=0 ) {
        DAQmxErrChk (DAQmxCreateCIAngEncoderChan(encoderTaskHandle,"PXI1Slot3/ctr3","",DAQmx_Val_X4,0,0.0,DAQmx_Val_AHighBHigh,DAQmx_Val_Degrees,24,0.0,""));

	    DAQmxErrChk (DAQmxStartTask(encoderTaskHandle));
    }
    //printf("IN");

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

	if( encoderTaskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(encoderTaskHandle);
		DAQmxClearTask(encoderTaskHandle);
        *rawHandle = 0;
    }
    else 
    {
        *rawHandle = encoderTaskHandle;
    }
    return 0;

Error:
	if( DAQmxFailed(error) )
		printf("DisableEncoder Error: %s\n",errBuff);
	//fclose(emgLogHandle);
	//printf("\nStopped EMG !\n");
	return 0;
}
