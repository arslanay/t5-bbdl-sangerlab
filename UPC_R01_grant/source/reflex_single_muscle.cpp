
using namespace std;

//------------------------------------------------------------------------
// DESTester.CPP
//
// This is the C++ source file for the FPGA-based DES encryptor/decryptor.
// This source calls the Opal Kelly FrontPanel C++ API to perform all
// communication with the device including:
//  - PLL Configuration
//  - Spartan-3 configuration file download
//  - Data transfer for the DES block.
//
// The general operation is as follows:
// 1. The device PLL is configured to generate appropriate clocks for
//    the hardware we have implemented.
// 2. A Xilinx configuration file is downloaded to the FPGA to configure
//    it with our hardware.  This file was generated using the Xilinx
//    tools.
// 3. Hardware setup is performed:
//    a. A reset signal is sent using a WireIn.
//    b. The DES key (64-bits) is set using 8 separate WireIns.
//    c. Another WireIn is set to control the DES 'decrypt' input.
//    d. The input and output files are opened.
//    e. The transfer block RAM address pointer is reset using a TriggerIn.
// 4. The DES algorithm is continually run on data from the input file
//    until the input is exhausted.  Results are stored in the output file:
//    a. A block of 2048 bytes is read from the input.
//    b. This block is sent to the FPGA using okCFrontPanel::WriteToPipeIn(...)
//    c. A TriggerIn is activated to start the DES state machine.
//    d. A block of 2048 bytes is read from the FPGA using the method
//       okCXEM::ReadFromPipeOut(...)
//    e. The block is written to the output file.
//    f. This sequence is repeated until the input file is exhausted.
//
//
// Copyright (c) 2004-2009
// Opal Kelly Incorporated
// $Rev: 1010 $ $Date: 2011-10-08 17:49:29 -0700 (Sat, 08 Oct 2011) $
//------------------------------------------------------------------------

#include <iostream>
#include <conio.h>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "okFrontPanelDLL.h"
/*
*	Name:			reflex_single_muscle

*	Author:			C. Minos Niu

*	Email:			minos.niu AT sangerlab.net

*/

#include	<math.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<time.h>
#include	"NIDAQmx.h"
#include	"PxiDAQ.h"
#include	"Utilities.h"
#include	"glut.h"   // The GL Utility Toolkit (Glut) Header
#include	"OGLGraph.h"


int StartPositionRead(TaskHandle *rawHandle);
int StopPositionRead(TaskHandle *rawHandle);


// *** Global variables
double g_force [2];
pthread_t g_threads[NUM_THREADS];
pthread_mutex_t mutexPosition;
TaskHandle g_DOTaskHandle, g_ForceReadTaskHandle, g_AOTaskHandle, g_PositionRead;


OGLGraph* myGraph;

void init ( GLvoid )     // Create Some Everyday Functions
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.f);				// Black Background
    //glClearDepth(1.0f);								// Depth Buffer Setup
    myGraph = OGLGraph::Instance();
    myGraph->setup( 500, 100, 10, 20, 2, 2, 1, 200 );
}

void display ( void )   // Create The Display Function
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float) 800 / (float) 600, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // This is a dummy function. Replace with custom input/data
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    float value;
    value = 5*sin( 5*time ) + 10.f;

    myGraph->update( 10.0 * g_force[0] );
    //printf("%.4lf \n", g_force[0]);
    myGraph->draw();

    glutSwapBuffers ( );
}

void reshape ( int w, int h )   // Create The Reshape Function (the viewport)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float) w / (float) h, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
}

void keyboard ( unsigned char key, int x, int y )  // Create Keyboard Function
{
    switch ( key ) 
    {
    case 27:        // When Escape Is Pressed...

        exit(0);   // Exit The Program
        break;        // Ready For Next Case
    default:        // Now Wrap It Up
        break;
    }
}

void idle(void)
{
    glutPostRedisplay();
}

int32       readEncoder;
float64     dataEncoder[1];
// This Fucntion performs the Experimental Protocol

#define		DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

void* control_loop(void*)
{
    
	int32       error=0;
	char        errBuff[2048]={'\0'};

    while (1)
    {
        int i=0;
		DAQmxErrChk (DAQmxReadCounterF64(g_PositionRead,1,10.0,dataEncoder,1,&readEncoder,0));
		printf("\n\t%f",dataEncoder[0]); 



        //printf("f1 %.4lf :: f2 %.4lf \n", g_force[0], g_force[1]);
        if(_kbhit())
        {
            break;

        }
    } 

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( g_PositionRead!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(g_PositionRead);
		DAQmxClearTask(g_PositionRead);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}


void exitProgram() 
{
    DisableMotors(&g_DOTaskHandle);
    StopPositionRead(&g_PositionRead);
    StopSignalLoop(g_ForceReadTaskHandle);
}

void initProgram()
{
    StartSignalLoop(g_ForceReadTaskHandle);
    EnableMotors(&g_DOTaskHandle);
    StartPositionRead(&g_PositionRead);

}

void main ( int argc, char** argv )   // Create Main Function For Bringing It All Together
{
    glutInit( &argc, argv ); // Erm Just Write It =)
    init();

    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE ); // Display Mode
    glutInitWindowSize( 500, 250 ); // If glutFullScreen wasn't called this is the window size
    glutCreateWindow( "OpenGL Graph Component" ); // Window Title (argv[0] for current directory as title)
    glutDisplayFunc( display );  // Matching Earlier Functions To Their Counterparts
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutIdleFunc(idle);
  
    initProgram();
    atexit( exitProgram );

    int ctrl_handle = pthread_create(&g_threads[0], NULL, control_loop,	(void *)g_force);
   
    glutMainLoop( );          // Initialize The Main Loop
  

    return;

}


int StartPositionRead(TaskHandle *rawHandle)
{
	TaskHandle  encoderTaskHandle = *rawHandle;
	int32       error=0;
	char        errBuff[2048]={'\0'};
    uInt32      dataEnable=0xffffffff;
    uInt32      dataDisable=0x00000000;

    int32		written;

    DAQmxLoadTask ("EncoderSlot3Ctr3",&encoderTaskHandle);

 /*   DAQmxCreateTask ("",&encoderTaskHandle);
    DAQmxErrChk (DAQmxCreateCIAngEncoderChan(encoderTaskHandle,"PXI1Slot3/ctr3","",DAQmx_Val_X4,0,0.0,DAQmx_Val_AHighBHigh,DAQmx_Val_Degrees,24,0.0,""));
	DAQmxErrChk (DAQmxCfgSampClkTiming(encoderTaskHandle,"/PXI1Slot3/PFI24",1000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1));
*/
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