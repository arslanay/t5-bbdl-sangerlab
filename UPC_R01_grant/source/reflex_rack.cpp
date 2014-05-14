

#include "UdpClient.h"
#include <iostream>
using namespace std;
#include <conio.h>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <AntTweakBar.h>
#include "okFrontPanelDLL.h"
/*
Name:			reflex_single_muscle    
Authors:			
	C. Minos Niu   minos.niu AT sangerlab.net 
	John Rocamora    johnrocamora AT gmail.com
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

#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include <iostream>

class TimeData
{
	//+++ Add status string to send to console
	// or other debugging output

	LARGE_INTEGER initialTick, currentTick, frequency;

public:
	double actualTime;

	// Initialize tick and frequency
	TimeData(void);	

	// Generate status exit string
	~TimeData(void);

	// Resets the timer
	int resetTimer();

	//Get current time in seconds
	double getCurrentTime(void);
};




// DataLogger class declaration. For now let's leave it here
// Data Logging Class
// Allows us to record data by giving the function the values from 
// global variables or from variables calculated in data adquisition
// classes or others
class DataLogger
{
public:

    bool bIsRecording;   // Used to let user know the recording state

    DataLogger(DWORD delayMS, char* sDirectoryContainer, char* sDataHeader);
    ~DataLogger(void);

    int startRecording(void);
    int closeRecordingFile(void);
    void sendLogString(char *);
    int setFileName(char *);

private:
    static void staticRecordingCallback(void* param);
    void recordingCallback(void);
    int stopRecording(void);

    bool bClosingFile;
    char sDirectoryContainer[512];
    char sData[600];
    char sDataHeader[600];
    int fileOpenCounter;
    char sFileName[400];
    FILE *dataFile;
    int kill;
    HANDLE hIOMutex;	
    DWORD delayMS;
    bool bInSingleTrial;
};

// *** Global variables
TimeData		    gTimeData;
DataLogger		gDataLogger = DataLogger(
    3,		// 100 Hz Recording
    "C:\\data\\%s_fpga",
    "time,ctrlFpga0,ctrlFpga1,emg0,emg1,musLce0,musLce1,spkCnt0,spkCnt1,spinIa0,spinIa1,spinII0,spinII1,musDamp\n"
    );


UdpClient               gUdpClient;
float                   gAuxvar [NUM_AUXVAR*NUM_MOTOR];
pthread_t               gThreads[NUM_THREADS];
pthread_mutex_t         gMutex;  
TaskHandle              gEnableHandle, gForceReadTaskHandle, gAOTaskHandle, gEncoderHandle[NUM_MOTOR];
float                   gLenOrig[NUM_MOTOR], gLenScale[NUM_MOTOR], gMuscleLce[NUM_MOTOR], gMuscleVel[NUM_MOTOR];
bool                    gResetSim=false, gIsKinematic = false,
                        gIsPerturbing = false, gIsRecording=false, gResetGlobal=false, gIsP2pMoving=false;
float                   gP2pIndex = 0.0f, gDeltaLen = 0.0f;
LARGE_INTEGER           gInitTick, gCurrentTick, gClkFrequency;
FILE                    *gDataFile, *gConfigFile;
int                     gCurrMotorState = MOTOR_STATE_INIT;
double                  gEncoderCount[NUM_MOTOR];
float64                 gMotorCmd[NUM_MOTOR]={0.0, 0.0};
float32                 gGammaDyn = 0.0;
float32                 gGammaSta = 0.0;

const float             gGain = 0.4 / 2000.0; //0.4/2000.0 as the safe value

float                   gMusDamp = 200.0;//120.0;
bool                    gAlterDamping = false; //Damping flag
time_t                  randSeedTime;

SomeFpga                *gXemSpindleBic; 
SomeFpga                *gXemSpindleTri; 
SomeFpga                *gXemMuscleBic ; 
SomeFpga                *gXemMuscleTri ; 

float                   gCtrlFromFPGA[NUM_MUSCLE];
int                     gMuscleEMG[NUM_FPGA], gMNCount[NUM_FPGA];

OGLGraph*               gMyGraph;
char                    gLceLabel1[60];
char                    gLceLabel2[60];
char                    gTimeStamp[200];
char                    gTimeStampSend[200];
char                    gStateLabel[5][30] = {  "MOTOR_STATE_INIT",
                                                "MOTOR_STATE_WINDING_UP",
                                                "MOTOR_STATE_OPEN_LOOP",
                                                "MOTOR_STATE_CLOSED_LOOP",                            
                                                "MOTOR_STATE_SHUTTING_DOWN"};
//IPP
Ipp32f *taps0;
Ipp32f *taps1;
Ipp32f *dly0;
Ipp32f *dly1;
Ipp32f *tapsVel0IIR, *tapsVel1IIR;
Ipp32f *dlysVel0IIR, *dlysVel1IIR;

//IppsFIRState_32f *pFIRState0, *pFIRState1;
IppsIIRState_32f *pIIRStateVel0, *pIIRStateVel1;
IppsIIRState_32f *pIIRState0, *pIIRState1;

// Sinewave
int gWave[1024];

//Descending command
int gM1Voluntary = 0;
int gM1Dystonia = 0;

//AntTweakBar
TwBar *gBar; // Pointer to the tweak bar

//Fpga Data Logging
float32         gLenBic;
float32         gEmgBic;
float32         gLenTri;
float32         gEmgTri;
float           gForceBic;
float           gForceTri;
int             gSpikeCountBic;
int             gSpikeCountTri;
float32         gSpindleIaBic;
float32         gSpindleIIBic;
float32         gSpindleIaTri;
float32         gSpindleIITri;


void ledIndicator ( float w, float h );

void init ( GLvoid )     // Create Some Everyday Functions
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.f);				// Black Background
    //glClearDepth(1.0f);								// Depth Buffer Setup
    gMyGraph = OGLGraph::Instance();
    gMyGraph->setup( 500, 100, 10, 20, 2, 2, 1, 200 );
}
void outputText(int x, int y, char *string)
{
    int len, i;
    glRasterPos2f(x, y);
    len = (int) strlen(string);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
    }
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


    // gMyGraph->update( 10.0 * gAuxvar[0] );
    //  gMyGraph->update( gMuscleVel[0] );
    gMyGraph->update( gCtrlFromFPGA[1] );

    gMyGraph->draw();


    if(MOTOR_STATE_WINDING_UP)
        glColor3f(1.0f,0.0f,0.0f);
    else
        glColor3f(0.0f,1.0f,0.0f);
    ledIndicator( 10.0f,80.0f );


    if(!gIsRecording)
        glColor3f(1.0f,0.0f,0.0f);
    else
        glColor3f(0.0f,1.0f,0.0f);
    ledIndicator( 50.0f,80.0f );
    glColor3f(1.0f,1.0f,1.0f);

    // Draw tweak bars
    TwDraw();

    sprintf_s(gLceLabel1,"%f    %d   %.2f", gMuscleLce[0], gM1Voluntary, gCtrlFromFPGA[0]);
    outputText(10,95,gLceLabel1);
    sprintf_s(gLceLabel2,"%f    %d   %.2f   %f   %f", gMuscleLce[1], gM1Dystonia, gCtrlFromFPGA[NUM_FPGA - 1],gSpindleIaBic,gSpindleIIBic);
    outputText(10,85,gLceLabel2);
    outputText(300,95,gStateLabel[gCurrMotorState]);
    if(gIsKinematic) {
        outputText(300, 80, "Kinematic");
    }
    else {
        outputText(300, 80, "Phantom");
    }



    glutSwapBuffers ( );
}

void ledIndicator ( float w, float h )   // Create The Reshape Function (the viewport)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(w,h,0.0f);
    glVertex3f(w,h+10.0f,0.0f);
    glVertex3f(w+10.0,h+10.0f,0.0f);
    glVertex3f(w+10.0f,h,0.0f);
    glEnd();
}
void reshape ( int w, int h )   // Create The Reshape Function (the viewport)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float) w / (float) h, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);

    // Send the new window size to AntTweakBar
    TwWindowSize(w, h);
}

int SendButton(okCFrontPanel *xem, int buttonValue, char *evt)
{
    if (0 == strcmp(evt, "BUTTON_RESET_SIM"))
    {
        if (buttonValue) 
            xem -> SetWireInValue(0x00, 0xff, 0x02);
        else 
            xem -> SetWireInValue(0x00, 0x00, 0x02);
        xem -> UpdateWireIns();
    }
    else if (0 == strcmp(evt, "BUTTON_RESET_GLOBAL"))
    {
        if (buttonValue) 
            xem -> SetWireInValue(0x00, 0xff, 0x01);
        else 
            xem -> SetWireInValue(0x00, 0x00, 0x01);
        xem -> UpdateWireIns();
    }
    return 0;
}
int ProceedFSM(int *state);
int InitMotor(int *state);

int ProceedFSM(int *state)
{
    switch(*state)
    {
    case MOTOR_STATE_INIT:
        EnableMotors(&gEnableHandle);
        *state = MOTOR_STATE_WINDING_UP;
        break;
    case MOTOR_STATE_WINDING_UP:
        *state = MOTOR_STATE_OPEN_LOOP;
        break;
    case MOTOR_STATE_OPEN_LOOP:
        *state = MOTOR_STATE_CLOSED_LOOP;
        break;
    case MOTOR_STATE_CLOSED_LOOP:
        DisableMotors(&gEnableHandle);
        *state = MOTOR_STATE_SHUTTING_DOWN;
        break;
    case MOTOR_STATE_SHUTTING_DOWN:
        *state = MOTOR_STATE_SHUTTING_DOWN;
        break;
    default:
        *state = MOTOR_STATE_INIT;
    }
    return 0;
}

int InitMotor(int *state)
{
    DisableMotors(&gEnableHandle);
    *state = MOTOR_STATE_INIT;
    return 0;
}

int ShutdownMotor(int *state);
int ShutdownMotor(int *state)
{
    DisableMotors(&gEnableHandle);
    *state = MOTOR_STATE_SHUTTING_DOWN;
    return 0;
}

int countNameSendEvent ;

void keyboard ( unsigned char key, int x, int y )  // Create Keyboard Function
{
    switch ( key ) 
    {
    case 27:        // When Escape Is Pressed...

        exit(0);   // Exit The Program
        break;        

        //case 32:        // SpaceBar 
        //    //ShutdownMotor(&gCurrMotorState);
        //    break;  
    case 'N':       //Alter the damping
    case 'n':
        if(countNameSendEvent == 0) {
            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            sprintf_s(
                gTimeStampSend,
                "%4d%02d%02d%02d%02d%02d",
                timeinfo->tm_year+1900, 
                timeinfo->tm_mon+1, 
                timeinfo->tm_mday, 
                timeinfo->tm_hour, 
                timeinfo->tm_min, 
                timeinfo->tm_sec
                );
            gDataLogger.setFileName(gTimeStampSend);
            gUdpClient.sendMessageToServer(gTimeStampSend);
            countNameSendEvent++;
        }
        break;

    case 'T':       // Terminate the current trial
    case 't':
        gIsRecording = false;
        gIsPerturbing = false;
        gUdpClient.sendMessageToServer("TTT");
	gDataLogger.closeRecordingFile();
        countNameSendEvent = 0;
        break;

    case 'K':       // Selects the perturbation mode: Phantom or Kinematic
    case 'k':
        gIsKinematic = !gIsKinematic;
        gUdpClient.sendMessageToServer("KKK");
        break;

    case 'P':       // Generic perturbing state
    case 'p':
        if(!gIsPerturbing) {
            gIsPerturbing = true;
            gUdpClient.sendMessageToServer("PPP");
        }
        break;

    case 'G':       //Proceed in FSM
    case 'g':
        ProceedFSM(&gCurrMotorState);
        break;
    case 'L':       //Alter the damping
    case 'l':
        gAlterDamping = true;
        break;
    case 'R':       //Winding up
    case 'r':
	gTimeData.resetTimer();
	gIsRecording = true;
	gDataLogger.startRecording();
	gUdpClient.sendMessageToServer("RRR");
	
        break;
    case '9':       //Reset GLOBAL
        if(!gResetGlobal)
            gResetGlobal=true;
        else
            gResetGlobal=false;
        //SendButton(gXemMuscleBic, (int) gResetGlobal, "BUTTON_RESET_GLOBAL");
        //SendButton(gXemMuscleTri, (int) gResetGlobal, "BUTTON_RESET_GLOBAL");
        //SendButton(gXemSpindleBic, (int) gResetGlobal, "BUTTON_RESET_GLOBAL");
        //SendButton(gXemSpindleTri, (int) gResetGlobal, "BUTTON_RESET_GLOBAL");
        break;      

    case 'M':       //Minos: workaround for p2p movement
    case 'm':
        // FUNCTIONALITY NOT SUPPORTED IN THIS VERSION
        //gIsP2pMoving = true;
        break;
    case 'z':       //Rezero
    case 'Z':
        gLenOrig[0]=gAuxvar[2] + gEncoderCount[0];
        gLenOrig[1]=gAuxvar[2+NUM_AUXVAR] + gEncoderCount[1];
        break;
    case 'E':         
    case 'e':     
        InitMotor(&gCurrMotorState);
        break;  
    default:        
        break;
    }
    TwEventKeyboardGLUT(NULL,NULL,NULL);
}

void idle(void)
{
    glutPostRedisplay();
}

int WriteFPGA(okCFrontPanel *xem, int32 bitVal, int32 trigEvent)
{
    //bitVal = 0;

    int32 bitValLo = bitVal & 0xffff;
    int32 bitValHi = (bitVal >> 16) & 0xffff;


    //xem -> SetWireInValue(0x01, bitValLo, 0xffff);
    if (okCFrontPanel::NoError != xem -> SetWireInValue(0x01, bitValLo, 0xffff)) {
        printf("SetWireInLo failed.\n");
        //delete xem;
        return -1;
    }
    if (okCFrontPanel::NoError != xem -> SetWireInValue(0x02, bitValHi, 0xffff)) {
        printf("SetWireInHi failed.\n");
        //delete xem;
        return -1;
    }

    xem -> UpdateWireIns();
    xem -> ActivateTriggerIn(0x50, trigEvent)   ;

    return 0;
}



void* ControlLoopBic(void*)
{

    int32       error=0;
    char        errBuff[2048]={'\0'};	

    long iLoop = 0;
    while (1)
    {
        if(GetAsyncKeyState(VK_SPACE))
        { 
            ShutdownMotor(&gCurrMotorState);
        }

        if ((MOTOR_STATE_CLOSED_LOOP != gCurrMotorState) && (MOTOR_STATE_OPEN_LOOP != gCurrMotorState)) continue;
        //printf("\n\t%f",dataEncoder[0]); 
        iLoop++;
        /*        if (10000 <= iLoop)
        {
        gM1Dystonia = 5000;
        gM1Voluntary = gWave[iLoop % 1024];
        }    */    

        //Read FPGA values for data Logging
        //ReadFpga(gXemSpindleBic->xem, 0x26, "float32", &gLenBic);
        gXemMuscleBic->ReadFpga(0x20, "float32", &gEmgBic);
        //gXemMuscleBic->ReadFpga(0x30, "int32", &gSpikeCountBic);
        //gXemSpindleBic->ReadFpga(0x22, "float32", &gSpindleIaBic);
        //gXemSpindleBic->ReadFpga(0x24, "float32", &gSpindleIIBic);
        //ReadFpga(gXemMuscleBic->xem, 0x30, "int32", &gSpikeCountBic);
        // ReadFpga(gXemSpindleBic->xem, 0x26, "float32", &gLenTri);
        //ReadFpga(gXemMuscleBic->xem, 0x20, "float32", &gEmgTri);

        if(gAlterDamping && (gMusDamp>0.03f))
            //gMusDamp -= 0.03f;
            gMusDamp = 0.0f;


        //float32 tGainBic = 0.00005;// 0.051; // working = 0.141
        //float32 tGainTri = 0.00005;// 0.051; // working = 0.141
        float32 forceBiasBic = 10.0f;
        float   coef_damp = 0.004; // working = 0.04

        int32 bitValLce, bitValVel;
        int32   bitM1VoluntaryBic = 0, 
            bitM1DystoniaBic = 000;

        //gMuscleLce[0] += (-0.01f + (float)(rand() % 2000)/1000.0f/100.0f)*5.0;

        ReInterpret((float32)(gMuscleLce[0]), &bitValLce);
        ReInterpret((float32)(gMusDamp * gMuscleVel[0]), &bitValVel);

        gXemMuscleBic->ReadFpga(0x32, "float32", &gForceBic);

        gXemMuscleBic->WriteFpgaLceVel(bitValLce, bitValVel, bitM1VoluntaryBic, bitM1DystoniaBic, DATA_EVT_LCEVEL);
        const float tBias = 0.0;

        float tCtrl = (gForceBic - tBias) * gGain;
        gCtrlFromFPGA[0] = (tCtrl>= 0.0) ? tCtrl : 0.0;
        
        //ReInterpret((float32)(gMuscleLce[0]), &bitValLce);
        //gXemMuscleBic->SendPara(bitValLce, DATA_EVT_LCE);
        gXemSpindleBic->SendPara(bitValLce, DATA_EVT_LCEVEL);

        //Sleep(1);
        if(_kbhit()) break;
    } 
    return 0;
}

void* ControlLoopTri(void*)
{

    int32       error=0;
    char        errBuff[2048]={'\0'};

    long iLoop = 0;
    while (1)
    {
        if(GetAsyncKeyState(VK_SPACE))
        { 
            ShutdownMotor(&gCurrMotorState);
        }

        if ((MOTOR_STATE_CLOSED_LOOP != gCurrMotorState) && (MOTOR_STATE_OPEN_LOOP != gCurrMotorState)) continue;
        //printf("\n\t%f",dataEncoder[0]); 
        iLoop++;
        /*        if (10000 <= iLoop)
        {
        gM1Dystonia = 5000;
        gM1Voluntary = gWave[iLoop % 1024];
        }    */    

        //Read FPGA values for data Logging
        //ReadFpga(gXemSpindleBic->xem, 0x26, "float32", &gLenBic);
        //ReadFpga(gXemMuscleBic->xem, 0x20, "float32", &gEmgBic);
        // ReadFpga(gXemSpindleBic->xem, 0x26, "float32", &gLenTri);
        gXemMuscleTri->ReadFpga(0x20, "float32", &gEmgTri);
        //gXemMuscleTri->ReadFpga(0x30, "int32", &gSpikeCountTri);
        //gXemSpindleBic->ReadFpga(0x22, "float32", &gSpindleIaTri);
        //gXemSpindleBic->ReadFpga(0x24, "float32", &gSpindleIITri);


        float32 forceBiasTri = 10.0f;
        float   coef_damp = 0.004; // working = 0.04

        int32 bitValLce, bitValVel;
        int32   bitM1VoluntaryTri = 0, 
            bitM1DystoniaTri = 000;

        ReInterpret((float32)(gMuscleLce[1]), &bitValLce);
        ReInterpret((float32)(gMusDamp * gMuscleVel[1]), &bitValVel);
        
        gXemMuscleTri->ReadFpga(0x32, "float32", &gForceTri);
        gXemMuscleTri->WriteFpgaLceVel(bitValLce, bitValVel, bitM1VoluntaryTri, bitM1DystoniaTri, DATA_EVT_LCEVEL);

        const float tBias = 0.0;//9000000.0;  
        float tCtrl = (gForceTri - tBias) * gGain;
        gCtrlFromFPGA[1] = (tCtrl = 0.0) ? tCtrl : 0.0;



        //ReInterpret((float32)(gMuscleLce[1]), &bitValLce);
        //gXemMuscleTri->SendPara(bitValLce, DATA_EVT_LCE);	
        gXemSpindleTri->SendPara(bitValLce, DATA_EVT_LCEVEL);


        //Sleep(1);
        if(_kbhit()) break;
    } 
    return 0;
}

void SaveConfigCache()
{
    FILE *ConfigCacheFile;
    ConfigCacheFile= fopen("ConfigPXI.cache","w");
    fprintf(ConfigCacheFile,"%lf\t%lf",gLenScale[0],gLenScale[1]);
    fclose(ConfigCacheFile);
}

int InitFpga(okCFrontPanel *xem0)
{   
    if (okCFrontPanel::NoError != xem0->OpenBySerial()) {
        delete xem0;
        printf("Device could not be opened.  Is one connected?\n");
        return(NULL);
    }

    printf("Found a device: %s\n", xem0->GetBoardModelString(xem0->GetBoardModel()).c_str());

    // Configure the PLL using default config
    xem0->LoadDefaultPLLConfiguration();
	
    // Get some general information about the XEM.
    printf("Device firmware version: %d.%d\n", xem0->GetDeviceMajorVersion(), xem0->GetDeviceMinorVersion());
    printf("Device serial number: %s\n", xem0->GetSerialNumber().c_str());
    printf("Device ID string: %s\n", xem0->GetDeviceID().c_str());


    okCPLL22393 *pll;
    pll = new okCPLL22393;
    pll -> SetReference(48);        //base clock frequency
    const int HIGHEST_CLK_FREQ = 48 * 4; // in MHz
    int baseRate = HIGHEST_CLK_FREQ; //in MHz
    pll -> SetPLLParameters(0, baseRate, 48,  true);
    pll -> SetOutputSource(0, okCPLL22393::ClkSrc_PLL0_0);
    int clkRate = HIGHEST_CLK_FREQ; //mhz; 200 is fastest
    //pll -> SetOutputDivider(0, 1) ;
    pll -> SetOutputEnable(0, true);
    xem0 -> SetPLL22393Configuration(*pll);

    // Download the configuration file.
    if (okCFrontPanel::NoError != xem0->ConfigureFPGA(FPGA_BIT_FILENAME)) {
        printf("FPGA configuration failed.\n");
        delete xem0;
        return(-1);
    }


    //int newHalfCnt = 1 * 200 * (10 **6) / SAMPLING_RATE / NUM_NEURON / (value*4) / 2 / 2;
    //const int32 X_REAL_TIME = 1;
    const int32 X_REAL_TENTH_TIME = 10;

    //int32 newHalfCnt = HIGHEST_CLK_FREQ * (int32)(1e6) / 1024 / 128 / X_REAL_TIME / 2 / 2;

    int32 newHalfCnt = HIGHEST_CLK_FREQ * (int32)(1e6) * 10 / 1024 / 128 / X_REAL_TENTH_TIME / 2 / 2;
    printf("newHalfCnt = %d \n", newHalfCnt);
    //    WriteFPGA(xem0, 197, DATA_EVT_CLKRATE);
    WriteFPGA(xem0, newHalfCnt, DATA_EVT_CLKRATE);


    // Check for FrontPanel support in the FPGA configuration.
    if (false == xem0->IsFrontPanelEnabled()) {
        printf("FrontPanel support is not enabled.\n");
        delete xem0;
        return(-1);
    }

    SendButton(xem0, (int) true, "BUTTON_RESET_SIM");
    SendButton(xem0, (int) false, "BUTTON_RESET_SIM");
    printf("FrontPanel support is enabled.\n");


    return 0;
}

FileContainer *gSwapFiles;	

void InitProgram()
{
    gAlterDamping = false;
    srand((unsigned) time(&randSeedTime));

    // Load Fpga DLLs
    char dll_date[32], dll_time[32];

    printf("---- Opal Kelly ---- FPGA-DES Application v1.0 ----\n");
    if (FALSE == okFrontPanelDLL_LoadLib(NULL)) {
        printf("FrontPanel DLL could not be loaded.\n");
        return;
    }
    okFrontPanelDLL_GetVersion(dll_date, dll_time);
    printf("FrontPanel DLL loaded.  Built: %s  %s\n", dll_date, dll_time);

    // Two muscles, each with one Fpga handle

    //gXemSpindleBic = new SomeFpga(NUM_NEURON, SAMPLING_RATE, "124300046A"); 
    //gXemSpindleTri = new SomeFpga(NUM_NEURON, SAMPLING_RATE, "12320003RN"); 
    //gXemMuscleBic = new SomeFpga(NUM_NEURON, SAMPLING_RATE, "1201000216") ; 
    //gXemMuscleTri = new SomeFpga(NUM_NEURON, SAMPLING_RATE, "12430003T2") ; 

    gXemSpindleBic = new SomeFpga(NUM_NEURON, SAMPLING_RATE, "113700021E"); 
    gXemSpindleTri = new SomeFpga(NUM_NEURON, SAMPLING_RATE, "11160001CG"); 
    gXemMuscleBic = new SomeFpga(NUM_NEURON, SAMPLING_RATE, "0000000542") ; 
    gXemMuscleTri = new SomeFpga(NUM_NEURON, SAMPLING_RATE, "1137000222") ; 


    gSwapFiles = new FileContainer;

    //IPP_VEL_IIR
    dlysVel0IIR = ippsMalloc_32f(2 * (lenFilterVel_IIR + 1));
    dlysVel1IIR = ippsMalloc_32f(2 * (lenFilterVel_IIR + 1));
    tapsVel0IIR  = ippsMalloc_32f(2 * (lenFilterVel_IIR + 1));
    tapsVel1IIR  = ippsMalloc_32f(2 * (lenFilterVel_IIR + 1));

    ippsZero_32f(dlysVel0IIR, 2 * (lenFilterVel_IIR + 1) );
    ippsZero_32f(dlysVel1IIR, 2 * (lenFilterVel_IIR + 1) );

    tapsVel0IIR[0] =  0.0078; // for Lowpass filter velocity
    tapsVel0IIR[1] =  0.0156;
    tapsVel0IIR[2] =  0.0078;
    tapsVel0IIR[3] =  1.0000;
    tapsVel0IIR[4] = -1.7347;
    tapsVel0IIR[5] =  0.7660;

    tapsVel1IIR[0] =  0.0078;
    tapsVel1IIR[1] =  0.0156;
    tapsVel1IIR[2] =  0.0078;
    tapsVel1IIR[3] =  1.0000;
    tapsVel1IIR[4] = -1.7347;
    tapsVel1IIR[5] =  0.7660;

    ippsIIRInitAlloc_32f(&pIIRStateVel0, tapsVel0IIR, lenFilterVel_IIR, dlysVel0IIR);
    ippsIIRInitAlloc_32f(&pIIRStateVel1, tapsVel1IIR, lenFilterVel_IIR, dlysVel1IIR);


    //IPP_FR
    taps0 = ippsMalloc_32f(lenFilter);
    taps1 = ippsMalloc_32f(lenFilter);
    dly0  = ippsMalloc_32f(lenFilter);
    dly1  = ippsMalloc_32f(lenFilter);

    float   P   =   1.0;
    float   e   =   2.7183;
    float   T   =   0.001;
    float   tau   =   0.090; // rising time of muscle twitch in seconds
    float   a   =   exp(-T / tau);
    float   pefat = P * e * T * a / tau;


    taps0[0] =   0.0000000 * pefat;
    taps0[1] =   1.00 * pefat;
    taps0[2] =   0.0000000 * pefat;
    taps0[3] =   1.00 * pefat;
    taps0[4] =  -2 * a * pefat;
    taps0[5] =   a * a * pefat;

    taps1[0] =   0.0000000 * pefat;
    taps1[1] =   1.00 * pefat;
    taps1[2] =   0.0000000 * pefat;
    taps1[3] =   1.00 * pefat;
    taps1[4] =  -2 * a * pefat;
    taps1[5] =   a * a * pefat;


    ippsZero_32f(dly0,lenFilter);
    ippsZero_32f(dly1,lenFilter);

    //ippsFIRInitAlloc_32f( &pFIRState0, taps0, lenFilter, dly0 );
    //ippsFIRInitAlloc_32f( &pFIRState1, taps1, lenFilter, dly1 );
    ippsIIRInitAlloc_32f( &pIIRState0, taps0, lenFilter, dly0 );
    ippsIIRInitAlloc_32f( &pIIRState1, taps1, lenFilter, dly1 );

    // *** Get a sine wave
    SineGen(gWave);
    //{
    //    float F = 1.0f; // in Hz
    //    float BIAS = 20000.0f;
    //    float AMP = 10000.0f;
    //    float PHASE = 0.0f;
    //    float SAMPLING_RATE = 1024.0f;
    //    float dt = 1.0f / SAMPLING_RATE; // Sampling interval in seconds
    //    float periods = 1.0;

    //    auto w = F * 2 * PI * dt;
    //    int max_n = floor(periods * SAMPLING_RATE / F);
    //    printf("max_n = %d\n", max_n);
    //    for (int i = 0; i < max_n; i++)
    //    {
    //        gWave[i] = (int) (AMP * sin(w * i + PHASE) + BIAS);
    //    }
    //}

    //WARNING: DON'T CHANGE THE SEQUENCE BELOW
    StartReadPos(&gEncoderHandle[0], &gEncoderHandle[1]);


    StartSignalLoop(&gAOTaskHandle, &gForceReadTaskHandle); 
    InitMotor(&gCurrMotorState);

}

inline void LogData( void)
{   
    // Approximately 100 Hz Recording
    double actualTime;
    QueryPerformanceCounter(&gCurrentTick);
    actualTime = gCurrentTick.QuadPart - gInitTick.QuadPart;
    actualTime /= gClkFrequency.QuadPart;
    if (gIsRecording)
    {   
        fprintf(gDataFile,"%.3lf\t",actualTime );																	

        fprintf(gDataFile,"%f\t%f\t", gCtrlFromFPGA[0], gCtrlFromFPGA[1]);			
        fprintf(gDataFile,"%f\t%f\t%f\t%f\t%u\t%u\t", gEmgBic, gEmgTri, gMuscleLce[0], gMuscleLce[1], gSpikeCountBic, gSpikeCountTri);			
        fprintf(gDataFile,"%f\t%f\t%f\t%f\t%f\t%f\t", gSpindleIaBic, gSpindleIaTri, gSpindleIIBic, gSpindleIITri, gMusDamp, -gAuxvar[2]);			
        fprintf(gDataFile,"\n");

    }
}


void ExitProgram();


void TW_CALL SendGamma(void * foo)
{ 
    int32 bitValGammaDyn, bitValGammaSta;
    
    ReInterpret((float32)(gGammaDyn), &bitValGammaDyn);
    ReInterpret((float32)(gGammaSta), &bitValGammaSta);
    gXemSpindleBic->SendPara(bitValGammaDyn, DATA_EVT_GAMMA_DYN);
    gXemSpindleBic->SendPara(bitValGammaSta, DATA_EVT_GAMMA_STA);
    gXemSpindleTri->SendPara(bitValGammaDyn, DATA_EVT_GAMMA_DYN);
    gXemSpindleTri->SendPara(bitValGammaSta, DATA_EVT_GAMMA_STA);
  
}


int main ( int argc, char** argv )   // Create Main Function For Bringing It All Together
{
    countNameSendEvent = 0;
    gLenOrig[0]=0.0;
    gLenOrig[1]=0.0;

    gM1Voluntary= 0.0;
    gM1Dystonia= 0.0;
    //gLenScale=0.0001;

    FILE *ConfigFile;
    ConfigFile= fopen("ConfigPXI.txt","r");

    fscanf(ConfigFile,"%f\t%f",&gLenScale[0],&gLenScale[1]);

    fclose(ConfigFile);
    glutInit( &argc, argv ); // Erm Just Write It =)
    init();

    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE ); // Display Mode
    glutInitWindowSize( 800, 600); // If glutFullScreen wasn't called this is the window size
    glutCreateWindow( "OpenGL Graph Component" ); // Window Title (argv[0] for current directory as title)
    glutDisplayFunc( display );  // Matching Earlier Functions To Their Counterparts
    glutReshapeFunc( reshape );

    TwInit(TW_OPENGL, NULL);

    glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
    glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
    glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT); // same as MouseMotion
    glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);
    glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);

    TwGLUTModifiersFunc(glutGetModifiers);

    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);

    InitProgram();

    atexit(ExitProgram);


    pthread_mutex_init (&gMutex, NULL);
    int ctrl_handle_bic = pthread_create(&gThreads[1], NULL, ControlLoopBic,	(void *)gAuxvar);
    int ctrl_handle_tri = pthread_create(&gThreads[2], NULL, ControlLoopTri,	(void *)gAuxvar);

    gBar = TwNewBar("TweakBar");
    TwDefine(" GLOBAL help='This is our interface for the T5 Project BBDL-SangerLab.' "); // Message added to the help bar.
    TwDefine(" TweakBar size='400 200' color='96 216 224' "); // change default tweak bar size and color

    // Add 'g_Zoom' to 'bar': this is a modifable (RW) variable of type TW_TYPE_FLOAT. Its key shortcuts are [z] and [Z].
    TwAddVarRW(gBar, "Len Scale", TW_TYPE_FLOAT, &gLenScale, 
        " min=0.0000 max=0.0002 step=0.000001 keyIncr=l keyDecr=L help='Scale the object (1=original size).' ");

    TwAddVarRW(gBar, "GammaDyn", TW_TYPE_FLOAT, &gGammaDyn, 
        " min=0.00 max=400.00 step=10 ");
    TwAddVarRW(gBar, "GammaSta", TW_TYPE_FLOAT, &gGammaSta, 
        " min=0.00 max=400.00 step=10 ");

    
    TwAddButton(gBar, "GammaBoth", SendGamma, NULL, " label='Set Gammas' ");

    glutMainLoop( );          // Initialize The Main Loop  

    return 0;

}



void ExitProgram() 
{
    //pthread_join(gThreads[0], NULL);
    SaveConfigCache();
    DisableMotors(&gEnableHandle);    
    StopSignalLoop(&gAOTaskHandle, &gForceReadTaskHandle);
    StopPositionRead(&gEncoderHandle[0],&gEncoderHandle[1]);

    //IPP
    //ippsFIRFree_32f(pFIRState0);
    //ippsFIRFree_32f(pFIRState1);



    ippsFree(taps0);
    ippsFree(taps1);
    ippsFree(dly0);
    ippsFree(dly1);


    ippsIIRFree_32f(pIIRState0);
    ippsIIRFree_32f(pIIRState1);


    //IPP_VEL_IIR

    ippsFree(tapsVel0IIR);
    ippsFree(tapsVel1IIR);
    ippsFree(dlysVel0IIR);
    ippsFree(dlysVel1IIR);

    ippsIIRFree_32f(pIIRStateVel0);
    ippsIIRFree_32f(pIIRStateVel1);
    delete gXemSpindleBic;
    delete gXemSpindleTri;
    delete gXemMuscleBic ;
    delete gXemMuscleTri ;

    TwDeleteBar(gBar);

    TwTerminate();    
    delete gSwapFiles;


}



DataLogger::DataLogger(DWORD delayMS, char *sDirectoryContainer, char *sDataHeader)
{
    bInSingleTrial = false;
    bClosingFile = false;
    this->delayMS = delayMS;
    strcpy_s(this->sDirectoryContainer, sDirectoryContainer);
    strcpy_s(this->sDataHeader, sDataHeader);
    fileOpenCounter = 0;

    hIOMutex = CreateMutex(NULL, FALSE, NULL);
    kill = 0;
    bIsRecording = false;
    _beginthread(DataLogger::staticRecordingCallback, 0, this);
}

DataLogger::~DataLogger(void)
{
    bIsRecording = false;
    kill = 1;
}

void DataLogger::sendLogString(char *logString)
{
    strcpy_s(sData, logString);
}

void DataLogger::recordingCallback() 
{ 
    int count = 0;

    while (!kill) {
        if(delayMS > 0) {
            Sleep(delayMS); 
        }
        WaitForSingleObject(hIOMutex, INFINITE);

        if (bInSingleTrial) {
            if (fileOpenCounter == 0) {
                dataFile = fopen(sFileName, "w");
                if (dataFile == NULL) {
                    MessageBoxA(
                        NULL, 
                        "Could not open data file", 
                        "File Error",
                        MB_OK
                        );
                }

                fprintf(
                    dataFile,
                    sDataHeader
                    );
                fileOpenCounter++;
            }

            if(fileOpenCounter == 1 && bIsRecording) {
                fprintf(dataFile,"%.3lf,", gTimeData.getCurrentTime());																	
                fprintf(dataFile,"%.4f,%.4f,", gCtrlFromFPGA[0], gCtrlFromFPGA[1]);			
                fprintf(dataFile,"%.4f,%.4f,%.4f,%.4f,%u,%u,", gEmgBic, gEmgTri, gMuscleLce[0], gMuscleLce[1], gSpikeCountBic, gSpikeCountTri);			
                fprintf(dataFile,"%.4f,%.4f,%.4f,%.4f,%.4f", gSpindleIaBic, gSpindleIaTri, gSpindleIIBic, gSpindleIITri, gMusDamp);			
                fprintf(dataFile,"\n");

                //fprintf(dataFile, "%s", sData);
            }

            if(fileOpenCounter == 1 && bClosingFile) {
                if(dataFile != NULL) {
                    fclose(dataFile);
                }
                // If this was successfull then exit the single trial and reset flags
                dataFile = NULL;
                fileOpenCounter = 0;
                bClosingFile = false;
                bInSingleTrial = false;

            }   

        }	
        ReleaseMutex( hIOMutex);
    }
}


void DataLogger::staticRecordingCallback(void* a) {
    //std::cout << "In threadFunc\n";
    ((DataLogger*)a)->recordingCallback();
}


// Corresponds to the R message
int DataLogger::startRecording()
{
    bIsRecording = true;
    return 0;
}

// Corresponds to the T message

int DataLogger::stopRecording()
{
    bIsRecording = false;
    return 0;
}

int DataLogger::closeRecordingFile()
{
    bClosingFile = true;
    stopRecording();
    return 0;
}


// Corresponds to the N message
int DataLogger::setFileName(char *inFileName)
{
    sprintf(
        sFileName, 
        sDirectoryContainer, 
        inFileName
        );
    bInSingleTrial = true;
    return 0;
}


TimeData::TimeData(void)
{
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&initialTick);
}


TimeData::~TimeData(void)
{
}

// Reset the timer
int TimeData::resetTimer(void)
{
	QueryPerformanceCounter(&initialTick);
	return 0;
}


// Get current time in seconds
double TimeData::getCurrentTime(void)
{
	QueryPerformanceCounter(&currentTick);
	actualTime = (double)(currentTick.QuadPart - initialTick.QuadPart);
	actualTime /= (double)frequency.QuadPart;
	return actualTime;
}
