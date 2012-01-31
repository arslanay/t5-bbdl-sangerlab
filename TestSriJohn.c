#include <ansi_c.h>  
#include <cvirte.h> 
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define BIC_ID 0
#define TRI_ID 1
#define MAX_NUM_MN 32
#define NUM_MN 10
#define NUM_MNStates 3
#define MAX_VOLTAGE 0.8

// prototypes
int Doer (double *stateMatrix,int bufferInd, int bufferLength,int numDataColumns, double samplFreq, double *motorVoltages, double *param, double *auxVar, double *user, double *exportVars);
void Izhikevich(double *neuron_state, double *neuron_input);
void Spindle(double *spindle_state, double *spindle_input);  
int UpdateMuscleLoop(double *loopState, double *mnPoolState, double *loopInput);


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
	// [8] Muscle length scale 0.1 - calibrate the encoder BICEPS
	// [9] Muscle length origin 5 - calibrate the encoder BICEPS	
	// [10] Muscle length scale 0.1 - calibrate the encoder TRICEPS
	// [11] Muscle length origin 5 - calibrate the encoder TRICEPS	

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
	
	double bicMNPool[NUM_MN*NUM_MNStates];
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
	memcpy(bicMNPool, user + NUM_MN * NUM_MNStates * BIC_ID, NUM_MNStates * NUM_MN * sizeof(double));
	
	
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
	
	UpdateMuscleLoop(bicState, bicMNPool, bicInput);	
	
	if (bicState[7] > 0) motorVoltages[bicMotorIndex] = (bicState[7]*bicInput[10] >MAX_VOLTAGE) ? MAX_VOLTAGE : bicState[7]*bicInput[10];
	else motorVoltages[bicMotorIndex] = param[6];
	
	for(int i=0;i<NUM_MN;i++)
	{
		exportVars[2 * i + NUM_MN * 2 * BIC_ID] = bicMNPool[3*i];	//mni voltage
		exportVars[2 * i + 1 +  NUM_MN * 2 * BIC_ID] = bicMNPool[3*i + 2];	//mni spikes
	}
	
	memcpy(auxVar + NUM_STATE * BIC_ID, bicState, NUM_STATE * sizeof(double));
	memcpy(user + NUM_MN * NUM_MNStates * BIC_ID, bicMNPool,  NUM_MNStates * NUM_MN * sizeof(double));
	
	//*** New Triceps
	
	int triMotorIndex = 2;
	
	double triState[NUM_STATE];	//auxvar key
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
	
	double triMNPool[NUM_MN*NUM_MNStates];
	// [0] motor neuron - voltage
	// [1] motor neuron - recovery variable
	// [2] motor neuron - binary spike
	// [0] motor neuron - voltage
	// [1] motor neuron - recovery variable
	// [2] motor neuron - binary spike
	// [0] motor neuron - voltage
	// [1] motor neuron - recovery variable
	// [2] motor neuron - binary spike
	
	double triInput[NUM_INPUT]; 
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
	
	
	memcpy(triState, auxVar + NUM_STATE * TRI_ID, NUM_STATE * sizeof(double));
	memcpy(triMNPool, user + NUM_MN * NUM_MNStates * TRI_ID, NUM_MNStates * NUM_MN * sizeof(double));
	
	
	triInput[0] = param[0] + triState[6]*param[7];
	triInput[1] = param[1];
	triInput[2] = (double) (1.0 / samplFreq);
	
	triInput[3] = param[2]; // gamma dynamic for bag 1
	triInput[4] = param[10]*(-stateMatrix[currVecInd + 1 + triMotorIndex]+param[11])+1.0; // muscle length, Lce in Loeb model
	triInput[5] = (double) (0.1 / samplFreq);
	
	triInput[6] = param[3];
	triInput[7] = param[4];
	triInput[8] = (double) (1.0 / samplFreq);
	
	triInput[9] = param[5];
	triInput[10] = param[6];
	
	UpdateMuscleLoop(triState, triMNPool, triInput);	///???? Add ID
	
	if (triState[7] > 0) motorVoltages[triMotorIndex] = (triState[7]*triInput[10] >MAX_VOLTAGE) ? MAX_VOLTAGE : triState[7]*triInput[10] ;
	else motorVoltages[triMotorIndex] = param[6];
	
	
	for(int i=0;i<NUM_MN;i++)
	{
		exportVars[2 * i + NUM_MN * 2 * TRI_ID] = triMNPool[3*i];			//mni voltage
		exportVars[2 * i + 1 + NUM_MN * 2 * TRI_ID] = triMNPool[3*i + 2];	//mni spikes
	}
	
	memcpy(auxVar + NUM_STATE * TRI_ID, triState, NUM_STATE * sizeof(double));
	memcpy(user + NUM_MN * NUM_MNStates * TRI_ID, triMNPool,  NUM_MNStates * NUM_MN * sizeof(double));
	
	
	return 0;	
}

int UpdateMuscleLoop(double *loopState, double *mnPoolState, double *loopInput)
{
	// *** Izh motoneurons
	double mnState[NUM_MNStates];
	double mnInput[3];

	mnInput[0] = loopInput[0];
	mnInput[1] = loopInput[1];
	mnInput[2] = loopInput[2];	

	loopState[2] = 0.0;
	
	if(loopInput[9]>0.01) // RESET -> initialize all motoneuron pool
	{
		for (int i = 0; i < NUM_MN; i++)
		{
			memcpy(mnState, mnPoolState+NUM_MNStates*i, NUM_MNStates*sizeof(double));
			Izhikevich(mnState, mnInput);
			
			// mnState[0] = action potential
			// mnState[1] = recovery potential
			if (mnState[2] > 0)
			// mnState[2] = 30 if spikes or 0 if not;
			{
				loopState[2] += 3.0;	//loopInput[1]/10.0;	//loopInput[2];??
			} 
			memcpy(mnPoolState+NUM_MNStates*i, mnState, NUM_MNStates*sizeof(double));
		}
		// Temporary : only Save MN#0 to loopState 
		// Goal : Save all MN status bitwise to loopState[0..2]
		//loopState[0] = ranNum;			
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
		//Initialize random number from outside 
		srand( time(NULL) );
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


void Izhikevich(double *neuron_state, double *neuron_input)
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
  
  // 
  // SetRandomSeed(0);  
  // Firing threshold randomly distributed 25.0 ~ 35.0
  double TH_RANGE = 10.0;
  double TH = 30.0 - TH_RANGE + ( 2.0 * TH_RANGE * rand() / ( RAND_MAX + 1.0 ) );
  //*exportTH = TH;

  
  
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

void Spindle(double *spindle_state, double *spindle_input)
{
	// ##############
	// ## KSR             [10.4649 10.4649 10.4649]
	// ## KPR             [0.1127 0.1623 0.1623]
	// ## B0DAMP          [0.0605 0.0822 0.0822]
	// ## BDAMP           [0.2356 -0.046 -0.069]
	// ## F0ACT           [0 0 0]
	// ## FACT            [0.0289 0.0636 0.0954]
	// ## XII             [1 0.7 0.7]
	// ## LPRN            [1 0.92 0.92]
	// ## GI              [20000 10000 10000]
	// ## GII             [20000 7250 7250]
	// ## ANONLINEAR      [0.3 0.3 0.3]
	// ## RLDFV           [0.46 0.46 0.46]
	// ## LSR0            [0.04 0.04 0.04]
	// ## LPR0            [0.76 0.76 0.76]
	// ## L2ND            [1 0.04 0.04]
	// ## TAO             [0.192 0.185 0.0001]
	// ## MASS            [0.0002 0.0002 0.0002]
	// ## FSAT            [1 0.5 1]

	const double KSR = 10.4649;
	const double KPR = 0.1127;
	const double B0DAMP=0.0605;
	const double BDAMP=0.2356;
	const double F0ACT=0.0;
	const double FACT=0.0289;
	const double XII=1.0;
	const double LPRN=1.0;
	const double GI = 20000.0;
	const double GII = 20000.0;
	const double ANONLINEAR = 0.25;	//0.3 in original
	const double RLDFV=0.46;
	const double LSR0 = 0.04;
	const double LPR0 = 0.76;
	const double L2ND = 1.00;
	const double TAO = 0.192;
	const double MASS = 0.0002;
	const double FSAT = 1.00;

	const double freq = 60.0;
    	
	
	
	double gd = spindle_input[0];
	double lce = spindle_input[1];
	double DT = spindle_input[2];
  
	double alpha = 0.0;
	double beta = 1.0;
	
	double dx0;
	double dx1;
	double dx2;
	//double fib;

    double mingd;
	double CSS;
    double sig;

	double x0_prev = spindle_state[0];
	double x1_prev = spindle_state[1];
	double x2_prev = spindle_state[2];
	double fib = spindle_state[3];

	double dx0_prev = spindle_state[4];
	double dx1_prev = spindle_state[5];
	double dx2_prev = spindle_state[6];

	double xx0, xx1, xx2, x0, x1, x2;

	x0 = x0_prev + dx0_prev * DT;
	x1 = x1_prev + dx1_prev * DT;
	x2 = x2_prev + dx2_prev * DT;

	mingd = gd*gd / (gd*gd + freq*freq);
	dx0 = (mingd - x0) / 0.149; // Tao: 0.149
	dx1 = x2;
	if (x2 < 0.0)
	CSS = -1.0;
	else 
	CSS = 1.0;
    
    CSS = (fabs(x2) == 0.0) ? 0.0 : CSS;

    sig=((x1 - RLDFV) > 0.0) ? (x1 - RLDFV) : 0.0;

	//printf("%.6f\n", lce);
	// dx2 = (1 / MASS) * (KSR * lce - (KSR + KPR) * x1 - CSS * (BDAMP * x0) * (fabs(x2)) - 0.4);
	// dx2 = (1 / MASS) * (KSR * lce - (KSR + KPR) * x1 - CSS * (BDAMP * x0) * sig * sqrt(sqrt(fabs(x2))) - 0.4);
    dx2 = (1.0 / MASS) * 
        (KSR * lce - (KSR + KPR) * x1 - (B0DAMP + BDAMP * x0) * sig * CSS * pow(fabs(x2), ANONLINEAR) - 
        (FACT * x0) - KSR*LSR0 + KPR*LPR0 );

    xx0 = x0 + DT * (dx0 + dx0_prev)/2.0;
    xx1 = x1 + DT * (dx1 + dx1_prev)/2.0;
    xx2 = x2 + DT * (dx2 + dx2_prev)/2.0;

	fib = GI * (lce - xx1 - LSR0);
	fib = (fib >= 0.0 && fib <= 100000.0) ? fib : (fib >100000.0 ? 100000.0: 0.0);

	spindle_state[0] = xx0;
	spindle_state[1] = xx1;
	spindle_state[2] = xx2;
	spindle_state[3] = fib;
	spindle_state[4] = dx0;
	spindle_state[5] = dx1;
	spindle_state[6] = dx2;
	//printf("%.6f\n", xx0);
	//printf("%.6f\n", xx2);

}

