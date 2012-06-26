/*
*	Name:			CMN_LtoR.cpp

*	Version:		build0415

*	Function:		HapticMaster is allowed to move on a horizontal plane,
					enclosed by a tunnel. Back and forth movements, servoed-
					perturbation implemented.

*	Modify:			- Servo points = 1000
					- 3 perturbation types
					- Only keep the Go beep
					- Split the HUGE program into files.
					- Use boost::program_options to parse paraName.ini
					- Debugged the TimerCB() function, added mutex lock.
					- Slow down the Servo3D process, not getting stuck
					- Perturbation triggered on force. Initial rest.
					- Added options of inertial, viscous and elastic load
					- ESC to exit; 'p' to pause; Space to show the cursor
					- Detect reaching the target with new criterion
					- Cleaned stdin after getchar()
					- Mediated the haunting '\n' after gets() 

*	To-do			- boost::lexical_cast
					- perturbation in different directions
					- NIDAQmx goes to thread

*	Author:			Minos Niu

*	Email:			minos.niu AT gmail.com

*	Date:			03-14-2008
*/



#include	"Utilities.h"
#include	"ExampleLib.h"
#include	<FcsHapticMaster.h>
#include	<FcsSpring.h>
#include	<FcsDamper.h>
#include	<FcsBlock.h>
#include	<fstream>
#include	<Inventor/SbLinear.h>
#include	<HapticAPI.h>
#include	<GL/glut.h>
#include	<math.h>
#include	<pthread.h>
#include	<ScopeAPI.h>
#include <semaphore.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<time.h>
#include	<process.h>
#include	"NIDAQmx.h"
#include	"EmgDAQ.h"

using namespace std;

FILE* emgLogHandle;	// EMG data file handle.
TaskHandle emgDaqHandle = 0;
ParaType g_thisPara;



string numStr("");
string subjectName("Out");	// Data File information
string paraName("");

// Variable to Monitor Testing Status
// Overall status of current experiment
// 0 - resetting ; 1 - countdown; 2 - testing; 
// 3 - testing complete; 4 - Pause at final position;
// 5 - Wait for force condition to be met
int			flag = 0;	
//int			n = -1;
//float		TargetForce = 2.0;

double		servoRefTraj[3][10000][3], servoRefTime[3][10000];

double		PosTol = 0.01;
double		onTargetTime = 1;

/* 
	Display the cursor or not
	1 - Always display; 0 - Display when moving slowly
*/
int displayCursor = 0;
int servoSampleNum[2] = {0, 0}, pertType = 3, ShowTarg2 = 0;

double		ForceDir = 90;
float		trajectoryDir = 90; //O corresponds to movement in ONLY x-direction

/*
	Thread variables
*/
pthread_t threads[NUM_THREADS];
pthread_mutex_t mutexPosition;
sem_t waitHapticMaster;


CFcsHapticMASTER *pHapticMaster = NULL;
CFcsSpring *pSpring;
CFcsDamper *pDamper;
CFcsBlock *pCube1;
CFcsBlock *pCube2;
CFcsBlock *pFloor;

double g_currentPos[3];
double g_currentVel[3];
double g_currentForce[3];
double g_jointVel = 0.0;
double g_jointForce = 0.0;
double gPkVel = -1000.0;
int g_listening = 0; // 0 - Not listerning; 1- Listening, time demanding!!!

double		Force[3] = {0.0, 0.0, 0.0};

int			gPause = 0;	// Pause flag
int			gReachTarget = 0;	// global reach target flag
int			gStartTrial = 1;

// Properties for the CFcsSpring object created in main()
double		springPos[3] = {0.0, 0.0, 0.0};
double		springDir[3] = {0.0, 0.0, 0.0};
double		springStiffness = 20000.0;
double		springDamping = 0.0;
double		springDeadband = 0.0;

// Location, Orientation And Size Parameters For The Haptic Floor Plane Object
double		floorCenter[3] = {0.0, 0.0, 0.0};
double		floorOrient[3] = {0.0, 0.0, 0.0};
double		FloorSize[3] = {0.8, 0.8, 0.1};


//double DamperCoeff[3] = {63.20, 63.20, 63.20};
double		DamperCoeff[3] = {1789.0, 1789.0, 1789.0};

//Location, Orientation and Size of Haptic Cube1
double		CubeCenter1[3] = {0, 0, 0.01};
double		CubeOrient1[3] = {0.0, 0.0, 0.0};
double		CubeSize1[3] = {0.8, 0.8, 0.02};

//Location, Orientation and Size of Haptic Cube2
double		CubeCenter2[3] = {0, 0, -0.01};
double		CubeOrient2[3] = {0.0, 0.0, 0.0};
double		CubeSize2[3] = {0.8, 0.8, 0.02};

// EndEffector OpenGL Material Parameters.
GLfloat		EndEffectorAmbient[] = {0.91, 0.44, 0.00, 1.00};
GLfloat		EndEffectorDiffuse[] = {0.90, 0.38, 0.00, 1.00};

// Targets OpenGL Material Parameters.
GLfloat		TargetAmbient[2][4] = {{0.00, 0.44, 0.91, 0.50}, {0.44, 0.00, 0.91, 0.50}};
GLfloat		TargetDiffuse[] = {0.90, 0.38, 0.00, 1.00};


// Target2 OpenGL Material Parameters.
GLfloat		InitialPositionAmbient[] = {0.00, 0.44, 0.91, 0.50};
GLfloat		InitialPositionDiffuse[] = {0.90, 0.38, 0.00, 1.00};


// Peak Velocity Indicator Material Parameters.
GLfloat		PkVelAmbient[] = {0.91, 0.44, 0.00, 1.00};
GLfloat		PkVelDiffuse[] = {0.90, 0.38, 0.00, 1.00};

// Acceptable velcity zone Material Parameters.
GLfloat		VelZoneAmbient[] = {0.5, 0.5, 0.5, 0.25};
GLfloat		VelZoneDiffuse[] = {0.90, 0.38, 0.00, 1.00};

// Target Force OpenGL Material Parameters.
GLfloat		AppliedForceAmbient[] = {1.0, 0.5, 0.0, 1.0};
GLfloat		AppliedForceDiffuse[] = {0.90, 0.38, 0.00, 1.00};

// Target Force OpenGL Material Parameters.
GLfloat		ForceConeAmbient[] = {1.0, 0.5, 0.0, 1.0};
GLfloat		ForceConeDiffuse[] = {0.90, 0.38, 0.00, 1.00};

// Floor Object OpenGL Material Parameters
GLfloat		FloorAmbient[] = {1.00, 1.00, 0.00, 1.00};
GLfloat		FloorDiffuse[] = {1.00, 1.00, 0.00, 1.00};

// General OpenGl Material Parameters.
GLfloat		Specular[] = {1.00, 1.00, 1.00, 1.00};
GLfloat		Emissive[] = {0.00, 0.00, 0.00, 1.00};
GLfloat		Shininess = {128.00};

GLfloat		SpecularOff[] = {0.00, 0.00, 0.00, 0.00};
GLfloat		EmissiveOff[] = {0.50, 0.50, 0.50, 0.00};
GLfloat		ShininessOff = {0.00};



// EndEffectorMaterial() Sets The Current OpenGl Material Paremeters. 
// Call This Function Prior To Drawing The EndEffector.



extern	int 
ReadParadigm(ParaType& pendingPara);

void
EndEffectorMaterial()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, EndEffectorAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, EndEffectorDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
	glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

// TargetMaterial() Sets The Current OpenGl Material Paremeters. 
// Call This Function Prior To Drawing The Targets.
void
TargetMaterial(void)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, TargetAmbient[gReachTarget]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, TargetDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
	glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

void
MiddleTargetMaterial(void)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, TargetAmbient[g_thisPara.middleSwitch]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, TargetDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
	glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

void 
InitialPositionMaterial()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, InitialPositionAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, InitialPositionDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
	glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}


void
PkVelMaterial()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, PkVelAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, PkVelDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
	glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

void
VelZoneMaterial()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, VelZoneAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, VelZoneDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
	glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

// ObjectMaterial() Sets The Current OpenGl Material Parameters. 
// Call This Function Prior To Drawing The Floor Object.
void 
FloorMaterial()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, FloorAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, FloorDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, SpecularOff);
	glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
	glMaterialf(GL_FRONT, GL_SHININESS, ShininessOff);
}

void 
AppliedForceMaterial()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, AppliedForceAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, AppliedForceDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
	glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

void 
ForceConeMaterial()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, ForceConeAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, ForceConeDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
	glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

//-----------------


// Countdown Timer for countdown before next trial.
void 
CountdownTimer(float seconds, double Show)
{

	LARGE_INTEGER	sample1;
	LARGE_INTEGER	sample2;
	LARGE_INTEGER	frequency;

	double		cdTime = seconds,	// For OpenGL display of cdTimer
		time = 0.0,
		actualTime = 0;

	QueryPerformanceCounter(&sample1);
	QueryPerformanceFrequency(&frequency);
	while (time < seconds)
	{
		//Setup the counting loop
		QueryPerformanceCounter(&sample2);
		actualTime = sample2.QuadPart - sample1.QuadPart;
		actualTime /= frequency.QuadPart;
		if (actualTime >= 1.0)
		{ 

			QueryPerformanceCounter(&sample1);
			time = time + actualTime;
			cdTime = int(seconds)-time;

		}
	}

}




// This Function Is Called To Draw The Graphic Equivalent Of 
// The EndEffector In OpenGl.
// The EndEffector Is Drawn At The Current Position

void 
DrawEndEffector(void)
{

	EndEffectorMaterial();

	glPushMatrix();

	if (g_listening == 0)
	{

		PthreadMutexLock(&mutexPosition);
		if (pHapticMaster != NULL)
		{
			pHapticMaster -> GetParameter(FCSPRM_POSITION, g_currentPos);
			pHapticMaster -> GetParameter(FCSPRM_VELOCITY, g_currentVel);
		}
		PthreadMutexUnlock(&mutexPosition);
		g_jointVel = sqrt(pow(g_currentVel[0], 2) + pow(g_currentVel[1], 2));
	}

	if ((displayCursor == 1)
		|| ((displayCursor == 0) && (g_jointVel < 0.1)))
	{
		glTranslatef(g_currentPos[0], g_currentPos[1], 
			g_currentPos[2]);
		glutSolidSphere(0.005, 20, 20);
	}
	glPopMatrix(); 

	if (gReachTarget == 1)
	{

		PkVelMaterial();
		glLineWidth(4);
		glBegin(GL_LINES);
		glVertex2f(-0.2, 0.25 + (gPkVel - 1.0) * 0.2);    // Vertical Line.
		glVertex2f(-0.23, 0.25 + (gPkVel - 1.0) * 0.2);
		glEnd();
	}
}


void 
DrawTargets(void)
{

	// Display Sphere for target 1
	TargetMaterial();
	glPushMatrix();

	/* Draw targets by accessing g_thisPara, need synchronizing */
	glTranslatef(g_thisPara.xTarget,g_thisPara.yTarget,g_thisPara.zTarget);

	glutSolidSphere(0.015, 20, 20);
	glPopMatrix();	

	VelZoneMaterial();
	glLineWidth(10);
	glBegin(GL_LINES);
	glVertex2f(-0.215, 0.25 - 0.15 * 1.0 * 0.2);    // Vertical Line.
	glVertex2f(-0.215, 0.25 + 0.15 * 1.0 * 0.2);
	glEnd();
}

void 
DrawMiddleTarget(void)
{

	// Display Sphere for target 1
	MiddleTargetMaterial();
	glPushMatrix();

	/* Draw targets by accessing g_thisPara, need synchronizing */
	glTranslatef(g_thisPara.xMiddle,g_thisPara.yMiddle,g_thisPara.zMiddle);

	glutSolidSphere(0.01, 20, 20);
	glPopMatrix();		
}

void 
DrawInitialPosition(void)
{

	// Display Sphere for target 1
	InitialPositionMaterial();

	glPushMatrix();
	glTranslatef(g_thisPara.xInit,g_thisPara.yInit,g_thisPara.zInit);
	glutSolidSphere(0.005, 20, 20);
	glPopMatrix();		
}

void 
DrawFloor(void)
{
	FloorMaterial();
	glScalef(0.8, 0.10, 0.08);
	glTranslatef(0.0, 0.0, 0.04);
	glutSolidCube(1.0);
}

// This Function Initializes the OpenGl Graphics Engine
void 
InitOpenGl (void)
{

	glShadeModel(GL_SMOOTH);

	glLoadIdentity();

	GLfloat		GrayLight[] = {0.75, 0.75, 0.75, 1.0};
	GLfloat		LightPosition[] = {1.0, 2.0, 1.0, 0.0};
	GLfloat		LightDirection[] = {0.0, 0.0, -1.0, 0.0};

	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, GrayLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, GrayLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, GrayLight);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	glEnable (GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0, 0.0, 0.3, 0.0);
}





// This Function Connects To The HapticMASTER And
// Initializes It.
int 
InitHapticMaster(void)
{
	int returnValue = 0;
	FCSSTATE currentState;

	// Call ConnectToHapticMASTER To Get A Pointer To A
	// CFcsHapticMASTER Object.
	pHapticMaster = ConnectToHapticMASTER ("vmd");

	// THIS IS VERY IMPORTANT!!!! WITHOUT WHICH THE ARM WILL BUMP IN THE BEGINNING
	pHapticMaster -> DeleteAll();

	// If A HapticMASTER Was Not Found At The Specified IP Address 
	// In The 'SERVERS.DB' File NULL Is Returned.
	if (pHapticMaster == NULL) 
		returnValue = -1;
	else	
	{
		// First Check The Current HapticMASTER State
		pHapticMaster  ->  GetCurrentState (currentState);
		if  (currentState != FCSSTATE_NORMAL)
		{
			// Set The HapticMASTER State To Initialised
			pHapticMaster -> SetRequestedState ( FCSSTATE_INITIALISED);

			// Wait for The HapticMASTER To Enter The Initialized State
			while (currentState != FCSSTATE_INITIALISED)
			{
				pHapticMaster -> GetCurrentState (currentState);
				Sleep(100);
			}

			// Set The HapticMASTER State To NormalForce
			pHapticMaster -> SetRequestedState(FCSSTATE_NORMAL);

			// Wait For The HapticMASTER To Enter The NormalForce State
			while (currentState != FCSSTATE_NORMAL)
			{
				pHapticMaster -> GetCurrentState(currentState);
				Sleep(100);
			}
		}

		// Call pHapticMaster -> DeleteAll() To Delete All Haptic Objects
		// Currently Active In The HapticMASTER
		pHapticMaster -> DeleteAll();

		// Set The HapticMASTER Inertia To background inertia.
		pHapticMaster -> SetParameter (FCSPRM_INERTIA, BG_INERTIA);
	}

	return returnValue;
}


//  Initializes the Scope for data logging
void 
InitScope(void)
{

	int Ret = 0;

	printf("Waiting for connection to Scope...");


	// If connection succesfull
	if(pHapticMaster != NULL)
	{
		printf("Connected Scope\n");

		// Initialize the HapticMASTER

		// Clear a possible previous list of channels to sample
		Ret = pHapticMaster -> RemoveAllScopeChannels();

		printf("Adding Scope Channels\n");

		// Add some new channels to sample
		Ret = pHapticMaster -> AddScopeChannel(FCSPRM_FORCEX);
		Ret = pHapticMaster -> AddScopeChannel(FCSPRM_FORCEY);
		Ret = pHapticMaster -> AddScopeChannel(FCSPRM_FORCEZ);

		Ret = pHapticMaster -> AddScopeChannel(FCSPRM_POSX);
		Ret = pHapticMaster -> AddScopeChannel(FCSPRM_POSY);
		Ret = pHapticMaster -> AddScopeChannel(FCSPRM_POSZ);

		Ret = pHapticMaster -> AddScopeChannel(FCSPRM_VELX);
		Ret = pHapticMaster -> AddScopeChannel(FCSPRM_VELY);
		Ret = pHapticMaster -> AddScopeChannel(FCSPRM_VELZ);

	}
	else
	{
		printf("Scope Not Connected\n");
	}


	// First check whether the Scope mechanism is supported
	if( pHapticMaster -> SupportScope() != SCOPE_NOT_SUPPORTED)
	{

		// Tell the ScopeAPI the IP address 
		ScopeSetIpAddress1(10, 4, 0, 34);

		// Connect only once during the application
		// Trials with different sample data files are
		// created by starting and stopping the scope 
		// and changing the sample dataFile name inbetween
		if( ScopeConnect() == SCOPE_CONNECTED )
		{
			printf("Scope Connected... \n");


		} // if connected
		else 
		{
			printf("Scope not connected");
		}
	}
	else
		printf("Scope NOT Supported... \n");


}


// This Function Is Called By OpenGL To Redraw The Scene
// Here's Where You Put The EndEffector And Block Drawing FuntionCalls
void 
Display(void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix ();

	// define eyepoint in such a way that
	// drawing can be done as in lab-frame rather than sgi-frame
	// (so X towards user, Z is up)

	gluLookAt (0.0, 0.0000001, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1);

	//gluLookAt (1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1);
	glRotated(90, 0, 0, 1);
	glutPostRedisplay();

	DrawTargets();
	DrawMiddleTarget();
	DrawEndEffector();
	DrawInitialPosition();

	//DrawFloor();

	glPopMatrix ();
	glutSwapBuffers();

}


// The Function Is Called By OpenGL Whenever The Window Is Resized
void 
Reshape(int iWidth, int iHeight)
{
	glViewport (0, 0, (GLsizei)iWidth, (GLsizei)iHeight);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();

	float fAspect = (float)iWidth/iHeight;
	gluPerspective (30.0, fAspect, 0.05, 20.0);            

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}


// This Function Is Called By OpenGl WhenEver A Key Was Hit
void 
Keyboard(unsigned char ucKey, int iX, int iY)
{
	switch (ucKey) 
	{
	case 27:	// for ESC
		if (pHapticMaster != NULL)
		{
			pHapticMaster -> DeleteAll();
			pHapticMaster -> SetRequestedState (FCSSTATE_INITIALISED);
		}
		exit(0);
		break;

	case 112:	// for 'p', lower case
		if (gPause != 1)
		{
			gPause = 1;
		}
		else
			gPause = 0;
		break;

	case 32:	// for Space
		displayCursor = 1 - displayCursor; //toggle the curser display
		break;

	default :
		break;
	}
}

void 
TimerCB (int iTimer)
{

	// Get The Current EndEffector Position From THe HapticMASTER

	// Set The Timer For This Function Again
	glutTimerFunc (10, TimerCB, 1);
}


void 
CheckPosition (double tx, double ty, double tz)
{
	double Px, Py, Pz;

	//printf("\nChecking...\n");
	PthreadMutexLock (&mutexPosition);
	if (pHapticMaster != NULL)
	{
		pHapticMaster -> GetParameter(FCSPRM_POSX, Px);
		pHapticMaster -> GetParameter(FCSPRM_POSY, Py);
		pHapticMaster -> GetParameter(FCSPRM_POSZ, Pz);
	}
	PthreadMutexUnlock (&mutexPosition);



	while ((abs(tx-Px) > 0.001) || 
		(abs(ty-Py) > 0.001) || 
		(abs(tz-Pz) > 0.001))
	{
		PthreadMutexLock (&mutexPosition);

		if (pHapticMaster != NULL){
			pHapticMaster -> GetParameter(FCSPRM_POSX, Px);
			pHapticMaster -> GetParameter(FCSPRM_POSY, Py);
			pHapticMaster -> GetParameter(FCSPRM_POSZ, Pz);}
		PthreadMutexUnlock (&mutexPosition);

	}
	//printf("Checked!!!\n");

}


/* Testing if the target has been reached */
void 
CheckReachTarget(double time)
{
	double xyDistance = 0.0;	// Distance between currentPos and target
	static double	
		reachTargetTimer = 0.0,	// Time elapsed since slowed below REACH_TARGET_VEL
		reachTargetOnset = 0.0;	// Time onset when firstly slowed below REACH_TARGET_VEL
	static int
		reachTargetFlag = 0; // 0: not slowing down; 1: slowing down not reached; 2: slowing down and reached

	if (gReachTarget == 1)
	{
		return;
	}

	PthreadMutexLock (&mutexPosition);
	if (pHapticMaster) 
	{
		pHapticMaster -> GetParameter(FCSPRM_POSITION, g_currentPos);
		pHapticMaster -> GetParameter(FCSPRM_VELOCITY, g_currentVel);// Reading velodity information
	}
	PthreadMutexUnlock (&mutexPosition);

	// Once reached the target, the OnTarget will be 1 till the end of current trial
	xyDistance = sqrt(pow(g_currentPos[0]-g_thisPara.xTarget,2)+pow(g_currentPos[1]-g_thisPara.yTarget,2));

	g_jointVel = sqrt(pow(g_currentVel[0], 2) + pow(g_currentVel[1], 2));

	if (reachTargetFlag == 2)
	{
		reachTargetFlag = 0;
		reachTargetTimer = 0.0;
		gReachTarget = 0;
		return;
	}
	if (g_jointVel > REACH_TARGET_VEL)
	{
		reachTargetFlag = 0;
		reachTargetTimer = 0.0;
		gReachTarget = 0;
		return;
	}	
	if ((reachTargetFlag == 0) && (g_jointVel <= REACH_TARGET_VEL))
	{
		reachTargetFlag = 1;
		reachTargetOnset = time;
		reachTargetTimer = 0.0;
		gReachTarget = 0;
		return;
	}
	if ((reachTargetFlag == 1) 
		&& (reachTargetTimer < REACH_TARGET_DELAY)) //0.3s
	{
		reachTargetTimer += time - reachTargetOnset;
		gReachTarget = 0;
		return;
	}	
	if ((reachTargetFlag == 1) 
		&& (xyDistance <= REACH_TARGET_DIST) && !gReachTarget) // 0.2
	{
		reachTargetFlag = 2;
		gReachTarget = 1;
		//	printf("Target Reached!\n");
		return;
	}				
}

void 
Servo3D(float xf, float yf, float zf, float duration, float rate)
{

	LARGE_INTEGER sample1;
	LARGE_INTEGER sample2;
	LARGE_INTEGER frequency;

	//period = SpringMoveSpeed; // 200Hz sampling frequency
	//float duration = 1.0;

	double	elapsedTime = 0.0,
		lastTime = 0.0;

	double tau;
	double dCoef[3] = {0.0, 0.0, 0.0};
	double xi, yi, zi; 

	if (pFloor != NULL)
		pFloor -> Disable();

	QueryPerformanceCounter (&sample1);
	QueryPerformanceFrequency (&frequency);


	dCoef[0] = 1789.0;  dCoef[1] = 1789.0; dCoef[2] = 1789.0;
	// Enable the damper
	if (pDamper != NULL)
		pDamper -> SetBaseParameters(dCoef);
	// Getting the current spring position


	PthreadMutexLock (&mutexPosition);
	if (pHapticMaster) 
		pHapticMaster -> GetParameter(FCSPRM_POSITION, g_currentPos);
	PthreadMutexUnlock (&mutexPosition);

	pSpring -> SetParameter(FCSPRM_POSITION, g_currentPos);

	if (pSpring != NULL)
		pSpring -> Enable();


	xi = g_currentPos[0];
	yi = g_currentPos[1];
	zi = g_currentPos[2];

	do 
	{

		//Setup the sampling loop
		QueryPerformanceCounter(&sample2);
		elapsedTime = sample2.QuadPart - sample1.QuadPart;
		elapsedTime /= frequency.QuadPart;

		//QueryPerformanceCounter(&sample1);
		if ((elapsedTime - lastTime) < 1 / rate)
		{
			continue;
		}				

		//Move Along a Minimum Jerk Trajectory
		tau = elapsedTime / duration;
		//springPos[0] = xi + (xi - xf) * (-tau);
		//	springPos[1] = yi + (yi - yf) * (-tau);
		//	springPos[2] = zi + (zi - zf) * (-tau);
		g_currentPos[0] = xi + (xi - xf) * ((15.0 * pow(tau, 4.0)) 
			- (6.0 * pow(tau, 5.0)) - (10.0 * pow(tau, 3.0)));
		g_currentPos[1] = yi + (yi - yf) * ((15.0 * pow(tau, 4.0)) 
			- (6.0 * pow(tau, 5.0)) - (10.0 * pow(tau, 3.0)));
		g_currentPos[2] = zi + (zi - zf) * ((15.0 * pow(tau, 4.0)) 
			- (6.0 * pow(tau, 5.0)) - (10.0 * pow(tau, 3.0)));
		//printf("%f %f %f\n",springPos[0],springPos[1],springPos[2]);
		//PthreadMutexLock (&mutexPosition);
		pSpring -> SetParameter(FCSPRM_POSITION, g_currentPos);
		//PthreadMutexUnlock (&mutexPosition);

		lastTime = elapsedTime;
	} 
	while (elapsedTime < duration);

}

inline void
ServoTraj(int pertIndex, double duration, double rate)
{

	LARGE_INTEGER sample1;
	LARGE_INTEGER sample2;
	LARGE_INTEGER frequency;

	//period = SpringMoveSpeed; // 200Hz sampling frequency
	//float duration = 1.0;

	double time;
	double actualTime = 0;
	//double servoActualTraj[3];
	double servoInitPos[3];


	//float tau;

	int i = 0;

	double dCoef[3] = {0.0, 0.0, 0.0};

	//double xi, yi, zi; 

	int t;
	t = displayCursor;
	displayCursor = 3;

	if (pFloor != NULL)
		pFloor -> Disable();

	time = 0.0;
	QueryPerformanceCounter(&sample1);
	QueryPerformanceFrequency(&frequency);


	dCoef[0] = 400.0;  dCoef[1] = 400.0; dCoef[2] = 400.0;
	// Enable the damper
	if (pDamper != NULL)
		pDamper -> SetBaseParameters(dCoef);
	// Getting the current spring position

	if (pHapticMaster != NULL)	// Background inertia + controlled inertia
	{
		pHapticMaster -> SetParameter (FCSPRM_INERTIA, BG_INERTIA);
	}

	/* Grab the semaphore */
	//PthreadMutexLock(&mutexPosition);
	if (pHapticMaster) 
		pHapticMaster -> GetParameter(FCSPRM_POSITION, servoInitPos);
	//PthreadMutexUnlock(&mutexPosition);

	pSpring -> SetParameter(FCSPRM_POSITION, servoInitPos);

	if (pSpring != NULL)
		pSpring -> Enable();


	// Moving the spring to the current position

	// Enable the spring

	while (i < servoSampleNum[pertIndex] - 10)
	{

		QueryPerformanceCounter(&sample2);
		actualTime = sample2.QuadPart - sample1.QuadPart;
		actualTime /= frequency.QuadPart;

		//QueryPerformanceCounter(&sample1);
		//time = time + actualTime;

		i = actualTime * rate;
		for (int j = 0; j < 3; j++)
			g_currentPos[j] = servoRefTraj[pertIndex][i][j] + servoInitPos[j];
		pSpring -> SetParameter(FCSPRM_POSITION, g_currentPos);

		//CheckReachTarget(time);
	}
	printf("\nServo completed!");
	displayCursor = t;
}



//-----------------
// Create a function FreeMove (Release Spring  ->  movement ->  reach target  ->  distance and velocity limits
// ->  Lock Spring)
//FreeMove3D(xi, yi, zi, Rx, Ry, zf, TRAJ_TIME, SERVO_SAMPLING_RATE);
//-----------

//-----------
// The function enables 3D free movement. The stopping condition is going slower than VelLim 
// for more than onTargetTime.  On Assaf_Exp.cpp there is another version FreeMove3D_Pos which is position
// oriented



void 
FreeMove3D(int iCurrentTrial, double xi, double yi, double zi, 
		   double xf, double yf, double zf, double elasticInitPosition,
		   double duration, double rate)
{

	LARGE_INTEGER sample1, sample2, frequency;
	double elapsedTime = 0.0, lastTime = 0.0;
	int ptbFlag = FLG_BELOW_DELAY;
	double dCoef[3] = {0.0, 0.0, 0.0};

	if (g_thisPara.sequence[iCurrentTrial - 1] != 0)
		g_listening = 1;

	Beep(800, 70);

	QueryPerformanceCounter(&sample1);
	QueryPerformanceFrequency(&frequency);


	if (g_thisPara.sequence[iCurrentTrial - 1] != 0) 
		printf("\nPertType = %d\n", g_thisPara.sequence[iCurrentTrial - 1]);


	do
	{
		PthreadMutexLock(&mutexPosition);
		if (pHapticMaster)
		{
			pHapticMaster -> GetParameter(FCSPRM_FORCE, g_currentForce);
			pHapticMaster -> GetParameter(FCSPRM_VELOCITY, g_currentVel);
		}
		PthreadMutexUnlock(&mutexPosition);

		g_jointVel = sqrt(pow(g_currentVel[0],2)+pow(g_currentVel[1],2));

		if (fabs(g_currentVel[0]) > gPkVel)
		{
			gPkVel = fabs(g_currentVel[0]);
		}


		g_jointForce = g_currentForce[0]*g_currentForce[0] + g_currentForce[1]*g_currentForce[1];

		if ((g_listening) 
			&& (g_currentVel[0] < -g_thisPara.trigThreshold)
//			&& (g_currentVel[0] < -g_thisPara.trigThreshold * 0.90)
			&& (g_currentForce[0] < 0))
//			&& (g_jointForce >= g_thisPara.trigThreshold))
//			&& (ptbFlag == FLG_ABOVE_DELAY_BELOW_THRESHOLD))
		{
			ServoTraj(g_thisPara.sequence[iCurrentTrial - 1] - 1, 2.0, SERVO_SAMPLING_RATE);
			gReachTarget = 1;
			g_listening = 0;
		}

		if (gReachTarget < 1)
		{
			//Calculate the time for Loop 1
			QueryPerformanceCounter(&sample2);
			elapsedTime = sample2.QuadPart - sample1.QuadPart;
			elapsedTime /= frequency.QuadPart;

			if (elapsedTime > duration)
			{
				gReachTarget = 1;
				//Beep(823, 500);
				continue;
			}

			if ((elapsedTime - lastTime) < 1 / rate)
			{
				continue;
			}				

			lastTime = elapsedTime;
			if (elapsedTime > 1.0)
				CheckReachTarget(elapsedTime);
		}

		//if ((elapsedTime > g_thisPara.trigDelay)
		//	&& (g_jointVel < g_thisPara.trigThreshold))
		//	ptbFlag = FLG_ABOVE_DELAY_BELOW_THRESHOLD;

		//if ((elapsedTime > g_thisPara.trigDelay)
		//	&& (g_jointVel > g_thisPara.trigThreshold))
		//	ptbFlag = FLG_ABOVE_DELAY_ABOVE_THRESHOLD;
	} 
	while  (elapsedTime < duration && gReachTarget < 1);

	CountdownTimer(1.0,0);

	dCoef[0] = 1789.0;  
	dCoef[1] = 1789.0; 
	dCoef[2] = 1789.0;

	// Enable the damper
	if (pDamper != NULL)
		pDamper -> SetBaseParameters(dCoef);

	//printf("Damping increased!!!!!!!!!!!!!!!\n");

	if (pHapticMaster != NULL)
	{
		pHapticMaster -> GetParameter(FCSPRM_POSITION, g_currentPos);
		pHapticMaster -> SetParameter(FCSPRM_INERTIA, BG_INERTIA);
	}

	// Enable the spring
	if (pSpring != NULL)
	{
		pSpring -> SetParameter(FCSPRM_POSITION, g_currentPos);
		pSpring -> Enable();
	}
}





// This Fucntion performs the Experimental Protocol
void* 
HapticProtocol(void*)
{

	float duration = 1.0;
	//int ii = 0, jj = 0;
	//int iForce_i = 1;
	//int iForce_f = 1;
	//int iMove_i = 1;
	//int		iMove_f = 1;
	char	finishExp = 'N';
	char	keepData = 'Y';

	string taskFile(""), sequenceFile(""), perturbFile(""), tempEmgFile(""), 
		dataFile(""), emgFile(""), rawSequence(""), tempDatFile("");

	float t_xf, t_yf, t_zf;
	double dCoef[3];


	FILE* hpPerturbHandle;

	int iCurrentTrial;

	do
	{
		ReadParadigm(g_thisPara);
		if (pHapticMaster != NULL)
		{
			floorCenter[2] = g_thisPara.zInit + 0.03;
			pFloor = pHapticMaster -> CreateBlock(floorCenter, floorOrient, FloorSize, 20000.0, 20000.0);
		}


		cout << "\n xInit = " << g_thisPara.xInit << g_thisPara.yInit << endl;

		printf("\nStart from which trial:");
		scanf( "%d", &gStartTrial);

		printf ("\n%d trials are to be conducted.\n", 
				g_thisPara.trialNum/* - gStartTrial + 1*/);

		for (int ii = 0; ii < pertType; ii++)
		{
			ostringstream oss;
			oss << (ii + 1);
			numStr = oss.str();
			perturbFile = "C:/HapticMASTER/People/Mark/Protocol/"
				+ paraName + "_" + numStr + ".ptb";
			cout << perturbFile << endl;

			hpPerturbHandle = fopen(ToChar(perturbFile),"r");

			if (hpPerturbHandle == NULL)
			{
				printf("\nError reading .ptb file!\n");
				break;
			}
			else
				printf("\nPerturbation file found!\n");

			fscanf(hpPerturbHandle, "%d\n", &servoSampleNum[ii]);
			for (int jj = 0; jj < servoSampleNum[ii]; jj++)
			{
				fscanf(hpPerturbHandle, "%lf %lf %lf %lf\n", 
					&servoRefTime[ii][jj], &servoRefTraj[ii][jj][0], 
					&servoRefTraj[ii][jj][1], &servoRefTraj[ii][jj][2]);
			}

			// Close protocol file
			if (hpPerturbHandle != NULL)
			{
				fclose(hpPerturbHandle);
			}
		}

		iCurrentTrial = gStartTrial;
		do 
		{
			gPkVel = 0.0;
			printf("\nTrial No. = %d ", iCurrentTrial);

			if (g_thisPara.sequence[iCurrentTrial - 1] != 0)
				printf("Perturbation pending...");
			gReachTarget = 0;

			ostringstream oss;
			oss << iCurrentTrial;
			numStr = oss.str();


			tempDatFile = "C:/HapticMASTER/People/Mark/Data/temp_" +
				numStr + ".dat";
			tempEmgFile = "C:/HapticMASTER/People/Mark/Data/temp_" +
				numStr + ".emg";

			FILE *fp1 = fopen("tempDatFile","r");
			FILE *fp2 = fopen("tempEmgFile","r");
			if( fp1 || fp2 ) 
			{
				// exists
				fclose(fp1);
				fclose(fp2);
				printf("\nData file exists! Continue? (y/N)");
				char ignoreOverwrite;
				do
				{
					cin >> ignoreOverwrite;
				}
				while (toupper(ignoreOverwrite) == 'Y');
			} 

			ScopeSetDataFileName(ToChar(tempDatFile));
			emgLogHandle = fopen(ToChar(tempEmgFile), "w");	//unified temp filename for EMG data

			Servo3D(g_thisPara.xInit, g_thisPara.yInit, g_thisPara.zInit, 2.0, 100); // Servoing to the initposition

			CountdownTimer(0.4,0);

			t_xf = g_thisPara.xTarget; 
			t_yf = g_thisPara.yTarget;
			t_zf = g_thisPara.zTarget;



			flag = 5;
			CheckPosition(g_thisPara.xInit, g_thisPara.yInit, g_thisPara.zInit);
			flag = 6;

			if (pFloor != NULL)
				pFloor -> Enable();
			if (pSpring != NULL)
				pSpring -> Disable();
			if (pHapticMaster != NULL)	// Background inertia + controlled inertia
			{
				pHapticMaster -> SetParameter (FCSPRM_INERTIA, BG_INERTIA + g_thisPara.loadCoef[0]);
			}


			// Disable the damper
			if (pDamper != NULL)
			{
				dCoef[0] = 0.0 + g_thisPara.loadCoef[1];  // Some damping + controlled damping
				dCoef[1] = 0.0 + g_thisPara.loadCoef[1]; 
				dCoef[2] = 0.0 + g_thisPara.loadCoef[1];
				pDamper -> SetBaseParameters(dCoef);
			}

			CountdownTimer(1.0, 0);

			//Prepare Scope
			if ( ScopeSetSampleRate(SCOPE_RATE_2500HZ) != SCOPE_NO_ERROR)
				printf("Error setting SYNC rate to 2500 Hz\n");

			if ( ScopeSetSyncRate(SCOPE_RATE_2500HZ) != SCOPE_NO_ERROR)
				printf("Error setting Syncpulse rate to 1250 Hz\n");

			// In the new scope version, set the Fixed sample count to
			// the exact nr of samples, instead of adding one
			//if (ScopeSetFixedSampleCount(500) != SCOPE_NO_ERROR)
			//printf("Error setting Samples");

			if ( ScopeConfigure() != SCOPE_NO_ERROR)
				printf("Error configuring rate\n");

			//Pause to allow subject preparation before trial
			//****************

			//EMG waiting for the ScopeAPI clock to synchronize
			StartEmg();

			//Start of trial
			if (ScopeStart() != SCOPE_NO_ERROR)
				printf("Scope NOT Started\n");

			// Disable the spring


			//Servo3D(g_thisPara.xTarget, g_thisPara.yTarget, g_thisPara.zTarget, 3.0, SERVO_SAMPLING_RATE);
			FreeMove3D(iCurrentTrial, g_thisPara.xInit, g_thisPara.yInit, 
				g_thisPara.zInit, g_thisPara.xTarget, g_thisPara.yTarget,
				g_thisPara.zTarget, g_thisPara.elasticOrigin, 7.0, 
				SERVO_SAMPLING_RATE);
			//printf("FreeMove3D ended\n");
			printf("\nPkVel = %3.2lf\n", gPkVel);



			//Beep(725, 350);



			StopEmg();

			// displayCursor = 0;
			// Modifying the final position

			// Stop the sampling
			if ( ScopeStop() != SCOPE_NO_ERROR )
				printf("Scope NOT Stopped\n");


			//Check if the user wants a rest
			while(gPause == 1)
			{}
			iCurrentTrial++;
		} 
		while (iCurrentTrial <= g_thisPara.trialNum); // End of trials

		//printf("\nSession Ended!");

		while (getchar() != '\n');

		printf ("\nKeep and Name the Data? (Y/N): ");
		cin >> keepData;

		if (toupper(keepData) != 'N')
		{
			// Input Subject Name
			printf ("Enter an Identifier (Initials and Condition Names) :");
			cin >> subjectName;
		}
		else
		{
			cout << "Warning: Data will be Deleted! (Y/N):";
			cin >> keepData;
			if (toupper(keepData) == 'N')
				keepData = 'Y';
			else
				keepData = 'N';
		}

		for (int ii = 0; ii < g_thisPara.trialNum; ii++)
		{
			ostringstream oss;

			oss << ii + 1;
			numStr = oss.str();

			dataFile ="C:\\HapticMASTER\\People\\Mark\\Data\\" + 
				subjectName + "_" + numStr + ".dat";	

			emgFile = "C:\\HapticMASTER\\People\\Mark\\Data\\" + 
				subjectName + "_" + numStr + ".emg";

			tempDatFile = "C:\\HapticMASTER\\People\\Mark\\Data\\temp_" + 
				numStr + ".dat";

			tempEmgFile = "C:\\HapticMASTER\\People\\Mark\\Data\\temp_" + 
				numStr + ".emg";

			if (toupper(keepData) != 'N')
			{
				rename(ToChar(tempDatFile), ToChar(dataFile));
				rename(ToChar(tempEmgFile), ToChar(emgFile));
			}
			else
			{
				remove(ToChar(tempDatFile));
				remove(ToChar(tempEmgFile));
			}
		}
		printf ("\nFinish the Experiment(Y/n)? ");
		cin >> finishExp;
	} while (toupper(finishExp) != 'Y');
	printf ("\nSwitch to the 3D Window, Hit ESC to Quit!");
	return 0;	
}



// Main Function
int 
main(int argc, char** argv)
{


	int trajectory;
	double tempSpringPos[3];

	sem_init(&waitHapticMaster, 0, 1);

	pthread_mutex_init (&mutexPosition, NULL);

	// Call The Initialize HapticMASTER Function
	InitHapticMaster();

	// Call The Initialize HapticMASTER Function
	InitScope();


	if (pHapticMaster != NULL)
	{
		pHapticMaster -> GetParameter(FCSPRM_POSITION, tempSpringPos);// Reading location information

		//printf("%f %f %f\n",tempSpringPos[0],tempSpringPos[1],tempSpringPos[2]);
		pSpring = pHapticMaster -> CreateSpring(tempSpringPos, springStiffness,
			springDamping, springDeadband); // Setting the spring at the current location
		pDamper = pHapticMaster -> CreateDamper(DamperCoeff);
	}

	if (pSpring != NULL)
		pSpring -> Enable();

	if (pDamper != NULL)
		pDamper -> Enable();

	if (pFloor != NULL)
		pFloor -> Enable();

	// OpenGL Initialization Calls
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE| GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize (800, 600);
	glutCreateWindow ("Fast Reaching Experiment");

	InitOpenGl();

	// More OpenGL Initialization Calls
	glutReshapeFunc (Reshape);
	glutDisplayFunc(Display);
	glutKeyboardFunc (Keyboard);
	glutTimerFunc (10, TimerCB, 1);

	trajectory = pthread_create(&threads[0], NULL, HapticProtocol,
		(void *)g_currentPos);
	//CountdownTimer(2.0,0);
	// Changing the spring stiffness and damping factor to the original one


	glutMainLoop();
	if (pHapticMaster != NULL)
	{
		// When all the trials are finished, close the scope
		ScopeClose();
	}


	if (pSpring != NULL)
	{
		pHapticMaster -> DeleteSpring(pSpring);
		delete pSpring;
	}

	if (pDamper != NULL)
	{
		pHapticMaster -> DeleteDamper(pDamper);
		delete pDamper;
	}

	if (pFloor != NULL)
	{
		pHapticMaster -> DeleteBlock(pFloor);
		delete pFloor;
	}

	sem_destroy(&waitHapticMaster);
	return 0; 
}
