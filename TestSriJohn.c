#include <ansi_c.h>  
#include <cvirte.h> 
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define BIC_ID 0
#define TRI_ID 1
#define MAX_NUM_MN 32
#define NUM_MN 3


// prototypes
int Doer (double *stateMatrix,int bufferInd, int bufferLength,int numDataColumns, double samplFreq, double *motorVoltages, double *param, double *auxVar, double *user, double *exportVars);
void Izhikevich(double *neuron_state, double *neuron_input);
void Spindle(double *neuron_state, double *neuron_input);  
int UpdateMuscleLoop(double *loopState, double *mnPoolState, double *loopInput);

//This one has issues
//Makes the system crash
//Doesnt crash when running without transformer

//ExportVars didnt change. Verify


int main (int argc, char *argv[]) 
{
	double *stateMatrix = (double *) calloc(500*67,sizeof(double));
	const int numMotorsTotal =20;
	int bufferInd;
	int bufferLength = 500;
	int numDataColumns = 67;
	double *motorVoltages = (double *) calloc(3*20,sizeof(double));
	double *param = (double *) calloc(3*numMotorsTotal,sizeof(double));
	double *auxVar = (double *) calloc(22,sizeof(double));
	double samplFreq = 10;
	bufferInd = 0;
	
	stateMatrix[0] = .1;
	
	for(int i=0;i<60;++i)
	{
		param[i]=10;
	}
	
	return 0;
}

int Doer (double *stateMatrix,int bufferInd, int bufferLength, int numDataColumns, double samplFreq, double *motorVoltages, double *param, double *auxVar, double *user, double *exportVars)
{
	const int NUM_STATE = 13;

	const int NUM_INPUT = 11;
	int currVecInd = bufferInd*numDataColumns;
	
	// param key
	// [0] motor neuron - input current
	// [1] motor neuron - digital spike size
	// [2] spindle - gamma dynamic				GAMMA DYNAMIC->T/Ksr (~stretch in the sensory region of each intrafusal fiber)
	// [3] muscle fiber - time constant
	// [4] muscle fiber - peak force	
	// [5] RESET the spindle state variables 1
	// [6] Output volgage 0.1	//LOOK FOR REDUNDANCY
	// [7] Scaling factor 0.05 spindleFiringRate -> izhCurrent(I)
	// [8] Muscle length scale 0.1 - calibrate the encoder
	// [9] Muscle length origin 43 - calibrate the encoder	

	int bicMotorIndex = 1;
	//int triMotorIndex = 0;	//Use a different motor, 
	
	double bicState[NUM_STATE];	//auxvar key
	// [0] motor neuron - voltage
	// [1] motor neuron - recovery variable
	// [2] motor neuron - binary spike
	// [3] spindle state - firing constant x0
	// [4] spindle state - polar region length x1
	// [5] spindle state - polar region velocity x2
	// [6] spindle state - Ia firing rate		GAMMA FIRING RATE?
	// [7] muscle fiber - current force
	// [8] muscle fiber - previous force
	// [9] LCE	
	// [10] dx0
	// [11] dx1
	// [12]	dx2	
	
	double bicMNPool[NUM_MN*3];
	// [0] motor neuron - voltage
	// [1] motor neuron - recovery variable
	// [2] motor neuron - binary spike
	// [0+] motor neuron - voltage
	// [1] motor neuron - recovery variable
	// [2] motor neuron - binary spike
	// [0] motor neuron - voltage
	// [1] motor neuron - recovery variable
	// [2] motor neuron - binary spike
	
	double bicInput[NUM_INPUT]; 
	// [0]	Input current
	// [1]	Digital spike size
	// [2]	Time
	// [3]	Gamma dynamic for bag 1
	// [4]	Muscle Length Lce
	// [5]	1/10 of Time
	// [6]	Muscle Fiber time constant C
	// [7]	Muscle Fiber Peak Force P
	// [8]	Time
	// [9]	RESET
	// [10]	output voltage scaling
	
	memcpy(bicState, auxVar + NUM_STATE * BIC_ID, NUM_STATE * sizeof(double));
	memcpy(bicMNPool, user + NUM_MN * BIC_ID, 3*NUM_MN * sizeof(double));
	
	
	bicInput[0] = param[0] + bicState[6]*param[7];
	bicInput[1] = param[1];
	bicInput[2] = (double) (1.0 / samplFreq);
	
	bicInput[3] = param[2]; // gamma dynamic for bag 1
	bicInput[4] = param[8]*(-stateMatrix[currVecInd + 1 + bicMotorIndex]+param[9])+1.0; // muscle length, Lce in Loeb model
	bicInput[5] = (double) (0.1 / samplFreq);
	
	bicInput[6] = param[3];
	bicInput[7] = param[4];
	bicInput[8] = (double) (1.0 / samplFreq);
	
	bicInput[9] = param[5];
	bicInput[10] = param[6];
	
	UpdateMuscleLoop(bicState, bicMNPool, bicInput);	///???? Add ID
	
	if (bicState[7] > 0) motorVoltages[bicMotorIndex] = bicState[7]*bicInput[10];
	else motorVoltages[bicMotorIndex] = param[6];// I agree
	
	exportVars[0] = bicMNPool[0];	//mn0 voltage
	exportVars[1] = bicMNPool[3];	//mn1 voltage
	exportVars[2] = bicMNPool[6];	//mn2 voltage
	exportVars[3] = bicState[6];	//afferent firing rate
	exportVars[4] = bicState[10];	//efferent firing rate
	exportVars[5] = bicState[9];	//muscle length Lce
	
	//Update auxvar
	memcpy(auxVar + NUM_STATE * BIC_ID, bicState, NUM_STATE * sizeof(double));
	memcpy(user + NUM_MN * BIC_ID, bicMNPool, NUM_MN *3* sizeof(double));

	//*** Triceps
	
	// int triMotorIndex = 2;
	
	// double triState[NUM_STATE];	//auxvar key
	// // [0] motor neuron - voltage
	// // [1] motor neuron - recovery variable
	// // [2] motor neuron - binary spike
	// // [3] spindle state - firing constant x0
	// // [4] spindle state - polar region length x1
	// // [5] spindle state - polar region velocity x2
	// // [6] spindle state - Ia firing rate		GAMMA FIRING RATE?
	// // [7] muscle fiber - current force
	// // [8] muscle fiber - previous force
	// // [9] LCE	
	// // [10] dx0
	// // [11] dx1
	// // [12]	dx2	
	
	// double triInput[NUM_INPUT]; 
	// // [0]	Input current
	// // [1]	Binary spike voltage in mV
	// // [2]	Time
	// // [3]	Gamma dynamic for bag 1
	// // [4]	Muscle Length Lce
	// // [5]	1/10 of Time
	// // [6]	Muscle Fiber time constant C
	// // [7]	Muscle Fiber Peak Force P
	// // [8]	Time
	// // [9]	RESET
	// // [10]	output voltage scaling
	
	// memcpy(triState, auxVar + NUM_STATE * TRI_ID, NUM_STATE * sizeof(double));
	
	
	// triInput[0] = param[0] + triState[6]*param[7];
	// triInput[1] = param[1];
	// triInput[2] = (double) (1.0 / samplFreq);
	
	// triInput[3] = param[2]; // gamma dynamic for bag 1
	// triInput[4] = param[8]*(-stateMatrix[currVecInd + 1 + triMotorIndex]+param[9])+1.0; // muscle length, Lce in Loeb model
	// triInput[5] = (double) (0.1 / samplFreq);
	
	// triInput[6] = param[3];
	// triInput[7] = param[4];
	// triInput[8] = (double) (1.0 / samplFreq);
	
	// triInput[9] = param[5];
	// triInput[10] = param[6];
	
	// UpdateMuscleLoop(triState, triInput);
	
	// if (triState[7] > 0) motorVoltages[triMotorIndex] = triState[7]*triInput[10];
	// else motorVoltages[triMotorIndex] = param[6];
	
	// exportVars[0] = triState[0];	//mn voltage
	// exportVars[1] = triState[1];	//mn recovery var
	// exportVars[2] = triState[2];	//mn spike
	// exportVars[3] = triState[6];	//afferent firing rate
	// exportVars[4] = triState[10];	//efferent firing rate
	// exportVars[5] = triState[9];	//muscle length Lce
	
	// //Update auxvar
	// memcpy(auxVar + NUM_STATE * TRI_ID, triState, NUM_STATE * sizeof(double));	
	
	return 0;	
}

int UpdateMuscleLoop(double *loopState, double *mnPoolState, double *loopInput)
{
	// *** Izh motoneurons
	//const int NUM_MN = 2; // <= MAX_NUM_MN
	double mnState[3];
	double mnInput[3];

	
	
	mnInput[0] = loopInput[0];
	mnInput[1] = loopInput[1];
	mnInput[2] = loopInput[2];	
	
	//memset(mnPoolState,0,MAX_NUM_MN*3*sizeof(double));	//Initialize
	
	loopState[2] = 0.0;
	
	if(loopInput[9]>0.01) // RESET -> initialize all motoneuron pool
	{
		for (int i = 0; i < NUM_MN; i++)
		{
			memcpy(mnState, mnPoolState+3*i, 3*sizeof(double));
			Izhikevich(mnState, mnInput);
			
			// mnState[0] = action potential
			// mnState[1] = recovery potential
			if (mnState[2] > 0)
			// mnState[2] = 30 if spikes or 0 if not;
			{
				loopState[2] += 10.0;	//loopInput[1]/10.0;	//loopInput[2];??
			} 
			memcpy(mnPoolState+3*i, mnState, 3*sizeof(double));
		}
		// Temporary : only Save MN#0 to loopState 
		// Goal : Save all MN status bitwise to loopState[0..2]
		loopState[0] = mnPoolState[0];				
		loopState[1] = mnPoolState[1];
		//Ok, so eventually we'll get this to behave according to 
		//the mnPoolState
	}
	else
	{	
		for (int i = 0; i < NUM_MN; i++)
		{
			mnPoolState[3*i] = loopState[0];
			mnPoolState[3*i+1] = loopState[1];
			mnPoolState[3*i+2] = loopState[2]; 		
		}	
	}

	// *** Spindle
	double spindleState[7];

	spindleState[0] = loopState[3]; // x0,
	spindleState[1] = loopState[4]; // x1
	spindleState[2] = loopState[5]; // x2
	spindleState[3] = loopState[6]; // Ia firing rate
	spindleState[4] = loopState[10]; // dx0,
	spindleState[5] = loopState[11]; // dx1
	spindleState[6] = loopState[12]; // dx2
		
	double spindleInput[3];
	spindleInput[0] = loopInput[3];
	spindleInput[1] = loopInput[4];
	spindleInput[2] = loopInput[5];
	
	loopState[9]=spindleInput[1]; 
	
	for (int j = 0; j < 10; j++) 
	{
		Spindle(spindleState, spindleInput);
	}
	
	if(loopInput[9]>0.01)
	{
		loopState[3] = spindleState[0] ; // x0
		loopState[4] = spindleState[1] ;// x1
		loopState[5] = spindleState[2] ; // x2
		loopState[6] = spindleState[3] ;// Ia firing rate
		loopState[10] = spindleState[4] ; //d x0
		loopState[11] = spindleState[5] ;// dx1
		loopState[12] = spindleState[6] ; // dx2
	}
	else
	{
		loopState[3] = 0.0 ; // x0
		loopState[4] = 0.9579 ;// x1
		loopState[5] = 0.0 ; // x2
		loopState[6] = 0.0 ;// Ia firing rate
		loopState[10] = 0.0 ; //d x0
		loopState[11] = 0.0 ;// dx1
		loopState[12] = 0.0 ; // dx2
		
	}
	
	// *** Muscle Fiber
	double C = loopInput[6];
	double P = loopInput[7];
	double h = loopInput[8];
		
	double fiberState[3];
	fiberState[0] = C/(exp(1)*P*h*h)-1/(exp(1)*P*h);         // define the discretization of the ODE 
	fiberState[1] = -2*C/(exp(1)*P*h*h)+1/(exp(1)*P*C);
	fiberState[2] = C/(exp(1)*P*h*h)+1/(exp(1)*P*h);
	double u;
	double f;	
		
	u = loopState[2]/h;
	f = (u-fiberState[0]*loopState[8]-fiberState[1]*loopState[7])/fiberState[2];
	
	loopState[8] = loopState[7];
	loopState[7] = f;
	
	return 0;
}


void
Izhikevich(double *neuron_state, double *neuron_input)
{
  double const A = 0.02; // a: time scale of the recovery variable u
  double const B = 0.2; // b:sensitivity of the recovery variable u to the subthreshold fluctuations of v.
  double const C = -65.0; // c: reset value of v caused by fast high threshold (K+)
  double const D = 6.0; // d: reset of u caused by slow high threshold Na+ K+ conductances
  double const X = 5.0; // x
  double const Y = 140.0; // y

  double v = neuron_state[0];
  double u = neuron_state[1];
  double vv, uu;
  double I = neuron_input[0];
  double vol_spike = neuron_input[1];
  double DT = neuron_input[2]*100.0;

  vv = v + DT * (0.04 * v*v + X * v + Y - u + I); // neuron[0] = v;
  uu = u + DT * A * (B * v - u); // neuron[1] = u; See iZhikevich model
  
  srand( time(NULL) );
  // Firing threshold randomly distributed 25.0 ~ 35.0
  double TH_RANGE = 10.0;
  double TH = 30.0 - TH_RANGE + ( 2.0 * TH_RANGE * rand() / ( RAND_MAX + 1.0 ) );
  
  if (vv >= TH) // if spikes    +randnumber???????????
    {
      neuron_state[0] = C;
      neuron_state[1] = uu + D;
      neuron_state[2] = vol_spike; // a flag in double that tells if the neuron is spiking
      //printf("%.3f\t\n", neuron_state[2]);

    }
  else
    {
      neuron_state[0] = vv;
      neuron_state[1] = uu;
      neuron_state[2] = 0.0;
    };
}

void Spindle(double *neuron_state, double *neuron_input)
{
  const double KSR = 10.4649;
  const double KPR = 0.1127;
  const double BDAMP=0.2356;
  const double GI = 20000.0;
  const double LSR0 = 0.04;
  const double M = 0.0002;
  const double freq = 60.0;
  double gd = neuron_input[0];
  double lce = neuron_input[1];
  double DT = neuron_input[2];

  double dx0;
  double dx1;
  double dx2;
  //double fib;

  double mingd;
  double CSS;

  double x0_prev = neuron_state[0];
  double x1_prev = neuron_state[1];
  double x2_prev = neuron_state[2];
  double fib = neuron_state[3];
  
  double dx0_prev = neuron_state[4];
  double dx1_prev = neuron_state[5];
  double dx2_prev = neuron_state[6];

  double xx0, xx1, xx2, x0, x1, x2;

  x0 = x0_prev + dx0_prev * DT;
  x1 = x1_prev + dx1_prev * DT;
  x2 = x2_prev + dx2_prev * DT;
  
  mingd = pow(gd, 2) / (pow(gd, 2) + pow(freq, 2));
  dx0 = (mingd - x0) / 0.149; // Tao: 0.149
  dx1 = x2;
  if (x2 < 0.0)
    CSS = -1.0;
  else 
    CSS = 1.0;

  //printf("%.6f\n", lce);
  dx2 = (1 / M) * (KSR * lce - (KSR + KPR) * x1 - CSS * (BDAMP * x0) * (fabs(x2)) - 0.4);
  
  xx0 = x0 + dx0 * DT;
  xx1 = x1 + dx1 * DT;
  xx2 = x2 + dx2 * DT;

  fib = GI * (lce - xx1 - LSR0) > 0.0 ? GI * (lce - xx1 - LSR0) : 0.0;
  //CHECK Sign of LSR0 page

  neuron_state[0] = xx0;
  neuron_state[1] = xx1;
  neuron_state[2] = xx2;
  neuron_state[3] = fib;
  neuron_state[4] = dx0;
  neuron_state[5] = dx1;
  neuron_state[6] = dx2;
  //printf("%.6f\n", xx0);
  //printf("%.6f\n", xx2);

}

