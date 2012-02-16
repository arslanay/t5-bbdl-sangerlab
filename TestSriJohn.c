#include <ansi_c.h>  
#include <cvirte.h> 
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define NUM_STATE 21
#define NUM_INPUT 15
#define BIC_ID 0
#define TRI_ID 1
#define MAX_NUM_MN 32
#define MAX_NUM_SN 32
#define NUM_MN 10
#define NUM_SN 10
#define NUM_SYN 10
#define NUM_EXTRAS 4
#define NUM_MN_STATES 3
#define NUM_SN_STATES 3
#define NUM_SYNAPSE_STATES 1
#define MAX_VOLTAGE 1.3
#define CYCLES_OF_INT 10

// prototypes
int Doer (double *stateMatrix,int bufferInd, int bufferLength,int numDataColumns, double samplFreq, double *motorVoltages, double *param, double *auxVar, double *user, double *exportVars);
void Izhikevich(double *neuron_state, double *neuron_input);
void Synapse(double *neuron_state, double *neuron_input);
void Spindle(double *spindle_state, double *spindle_input);  
int UpdateMuscleLoop(double *loopState, double *mnPoolState, double *snPoolState, double *synPoolState, double *loopParam);


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
	int currVecInd = bufferInd*numDataColumns;
	
    // param key
	   
    double bicAlphaBg = param[0];   // [0] biceps motor neuron - background alpha-MN input current
    double triAlpha = param[1];     // [1] triceps motor neuron - background alpha-MN input current 
    double spikeSize = param[2];    // [2] motor neuron - digital spike size
	double gammaDyn = param[3];     // [3] spindle - gamma dynamic	
	double musTime = param[4];      // [4] muscle fiber - time constant
	double musPeakForce = param[5]; // [5] muscle fiber - peak force	
	double reset = param[6];        // [6] RESET the spindle state variables 1
	double bag1FRtoI = param[7];    // [7] Scaling factor 0.05 bag1FiringRate -> izhCurrent(I)
	double lenScaleBic = param[8];  // [8] Muscle length scale 0.1 - calibrate the encoder BICEPS
	double lenScaleTri = param[9];  // [9] Muscle length origin 5 - calibrate the encoder BICEPS	
	double gammaSta = param[10];    // [10] Muscle length scale 0.1 - calibrate the encoder TRICEPS
    double bag2FRtoI = param[11];   // [11] Scaling factor 0.05 bag2FiringRate -> izhCurrent(I)
	double initMNVolt = param[12];  // [12] motor neuron - voltage INIT
	double initMNRecov = param[13]; // [13] motor neuron - recovery variable INIT
	double initMNSpike = param[14]; // [14] motor neuron - binary spike INIT
    
    int currMusInd;
    int currMusUserInd;
    
	int bicMotorIndex = 1;
	//int triMotorIndex = 0;	//Use a different motor, 
	
	double bicState[NUM_STATE];	//auxvar key
    // [0] LenOrigBic DEPRECATED
    // [1] LCE	    
    // [2] Accumulative MN spikes from the pool, within 1 cycle
    // [3] muscle fiber - current force
    // [4] muscle fiber - previous force
    // [5] bag1 state - firing constant x0
    // [6] bag1 state - polar region length x1
    // [7] bag1 state - polar region velocity x2
    // [8] bag1 state - bag1 firing rate		GAMMA FIRING RATE?
    // [9] dx0
    // [10] dx1
    // [11]	dx2	
    // [12] bag2 state - firing constant x3
    // [13] bag2 state - polar region length x4
    // [14] bag2 state - polar region velocity x5
    // [15] bag2 state - bag2 firing rate		GAMMA FIRING RATE?
    // [16] dx3
    // [17] dx4
    // [18]	dx5	
    // [19] Accumulative SN spikes from the pool, within 1 cycle
    // [20] output from Synapse (current)
    
    double bicMNPoolStates[NUM_MN * NUM_MN_STATES];
	// [0] motor neuron - voltage
	// [1] motor neuron - recovery variable
	// [2] motor neuron - binary spike
    // ...
	
	double bicSNPoolStates[NUM_SN * NUM_SN_STATES];
	// [0] sensory neuron - voltage
	// [1] sensory neuron - recovery variable
	// [2] sensory neuron - binary spike
    // ...
	
    double bicSynPoolStates[NUM_SYN * NUM_SYNAPSE_STATES];
	// [0] synapse model input
	// ...
    
	double bicParam[NUM_INPUT]; 
	// [0]	Current to the alpha_MN pool
	// [1]	Digital spike size
	// [2]	dt - Integration time interval 0.01s
	// [3]	Gamma dynamic for bag I
    // [4]	Gamma static for bag II
    // [5]  Drive to the Muscle Fiber
	// [6]	Muscle Fiber time constant C
	// [7]	Muscle Fiber Peak Force P
	// [8]	RESET
    // [9]  motor neuron - voltage INIT
	// [10] motor neuron - recovery variable INIT
	// [11] motor neuron - binary spike INIT
	// [12] presynaptic drive - Spike Count
	// [13]	Current to the SN pool
	// [14]	Muscle length
    
	memcpy(bicState, auxVar + NUM_STATE * BIC_ID, NUM_STATE * sizeof(double));
	        
    double bicLenOrig = bicState[0];
    //double bicLce = bicState[1];
    double bicNumMNSpikes = bicState[2];
    double bicCurrForce = bicState[3];
    double bicPrevForce = bicState[4];
    double bicBag1FConst = bicState[5];
    double bicBag1Len = bicState[6];
    double bicBag1Vel = bicState[7];
    double bicBag1FRate = bicState[8];
    double bicDx0 = bicState[9];
    double bicDx1 = bicState[10];
    double bicDx2 = bicState[11];
    double bicBag2FConst = bicState[12];
    double bicBag2Len = bicState[13];
    double bicBag2Vel = bicState[14];
    double bicBag2FRate = bicState[15];
    double bicDx3 = bicState[16];
    double bicDx4 = bicState[17];
    double bicDx5 = bicState[18];
    double bicNumSNSpikes = bicState[19];
    double bicSynOut = bicState[20];
        
    currMusUserInd =  (NUM_MN * NUM_MN_STATES + NUM_SN * NUM_SN_STATES + NUM_SYN * NUM_SYNAPSE_STATES) * BIC_ID;
    
    memcpy(bicMNPoolStates, user + currMusUserInd, NUM_MN_STATES * NUM_MN * sizeof(double));
    
    currMusUserInd =  currMusUserInd + NUM_MN * NUM_MN_STATES;
    
    memcpy(bicSNPoolStates, user + currMusUserInd, NUM_SN_STATES * NUM_SN * sizeof(double));
    
    currMusUserInd =  currMusUserInd + NUM_SN * NUM_SN_STATES;
    
    memcpy(bicSynPoolStates, user + currMusUserInd, NUM_SYNAPSE_STATES * NUM_SYN * sizeof(double));
        
    
    bicParam[0] = bicAlphaBg + bicSynOut;
	bicParam[1] = spikeSize;
	bicParam[2] = (double) (1.0 / samplFreq);    
    
	bicParam[3] = gammaDyn; // gamma dynamic for bag 1
    bicParam[4] = gammaSta;
    bicParam[14] = lenScaleBic*(-stateMatrix[currVecInd + 1 + bicMotorIndex] + bicLenOrig)+1.0;

    bicParam[5] = 30.0 / (double) NUM_MN * bicNumMNSpikes;  //Drive        
	bicParam[6] = musTime;
	bicParam[7] = musPeakForce;
	bicParam[8] = reset;
    
    bicParam[9] = initMNVolt;
    bicParam[10] = initMNRecov;
    bicParam[11] = initMNSpike;
	
    bicParam[12] = bicNumSNSpikes;  //Presynaptic Spike Count
    bicParam[13] = (bicBag1FRate - 60.0) * bag1FRtoI + (bicBag2FRate - 40.0) * bag2FRtoI;
	    
	    
    //REZERO THE SYSTEM    
    if(reset > 0.01)
    {
    	//bicState[1] = lenScaleBic*(-stateMatrix[currVecInd + 1 + bicMotorIndex] + bicLenOrig)+1.0; // muscle length, Lce in Loeb model
    }
    else
    {
        //bicState[1] = 1.0; // muscle length, Lce in Loeb model
        bicState[0] = stateMatrix[currVecInd + 1 + bicMotorIndex];   //bicLenOrig
        //Initialize random number from outside 
		srand( time(NULL) );
    }
    
	UpdateMuscleLoop(bicState, bicMNPoolStates, bicSNPoolStates, bicSynPoolStates, bicParam);	
	
	if (bicCurrForce > 0) motorVoltages[bicMotorIndex] = (bicCurrForce > MAX_VOLTAGE) ? MAX_VOLTAGE : bicCurrForce * 0.3;
	else motorVoltages[bicMotorIndex] = 0.3;
	
    //Export   
    
    bicBag1FRate = bicState[8];
    bicBag2FRate = bicState[12];
    //bicLce = bicState[1];
    
    currMusInd =  (NUM_MN * 2 + NUM_SN * 2 + NUM_EXTRAS) * BIC_ID; // This is 0 for biceps
    
    for(int i = 0 ;i < NUM_MN ; i++)
	{
		exportVars[currMusInd + 2 * i] = bicMNPoolStates[3*i];	//mni voltage
		exportVars[currMusInd + 2 * i + 1] = bicMNPoolStates[3*i + 2];	//mni spikes
	}
    
    currMusInd = currMusInd + NUM_MN * 2;
    
    for(int i = 0 ; i< NUM_SN ; i++)
	{
		exportVars[currMusInd + 2 * i] = bicSNPoolStates[3*i];	//mni voltage
		exportVars[currMusInd + 2 * i + 1] = bicSNPoolStates[3*i + 2];	//mni spikes
	}
    
    currMusInd = currMusInd + NUM_SN * 2;
        
    exportVars[currMusInd] = bicBag1FRate;
    exportVars[currMusInd + 1] = bicBag2FRate;
    exportVars[currMusInd + 2] = bicParam[14];
    exportVars[currMusInd + 3] = bicAlphaBg;
    
	memcpy(auxVar + NUM_STATE * BIC_ID, bicState, NUM_STATE * sizeof(double));
	
    currMusUserInd =  (NUM_MN * NUM_MN_STATES + NUM_SN * NUM_SN_STATES + NUM_SYN * NUM_SYNAPSE_STATES) * BIC_ID;
    
    memcpy(user + currMusUserInd, bicMNPoolStates, NUM_MN_STATES * NUM_MN * sizeof(double));
    
    currMusUserInd =  currMusUserInd + NUM_MN * NUM_MN_STATES;
    
    memcpy(user + currMusUserInd, bicSNPoolStates, NUM_SN_STATES * NUM_SN * sizeof(double));
    
    currMusUserInd =  currMusUserInd + NUM_SN * NUM_SN_STATES;
    
    memcpy(user + currMusUserInd, bicSynPoolStates, NUM_SYNAPSE_STATES * NUM_SYN * sizeof(double));
    
    return 0;	
}

int UpdateMuscleLoop(double *loopState, double *mnPoolState, double *snPoolState, double *synPoolState, double *loopParam)
{
    // PARAM LOAD
     
    double digSpikeSize = loopParam[1]; //????
    double timeInterval = loopParam[2];
   
    double reset = loopParam[8];
    
    // *** Izh alpha motoneuron pool :: I (nanoAmp) -> spike

    double iniMNVolt = loopParam[9];
    double iniMNRecov = loopParam[10];
    double iniMNSpike = loopParam[11];
 
	double mnState[NUM_MN_STATES];
	double mnInput[3];
 
	mnInput[0] = loopParam[0]; // current input to alpha
	mnInput[1] = digSpikeSize; // spike ? mnInput[1] : 0.0
	mnInput[2] = timeInterval; // dt
	
    double mnSpikeCount = 0.0;
    
	if(reset > 0.01) // RESET -> initialize all motoneuron pool
	{
		for (int i = 0; i < NUM_MN; i++)
		{
			memcpy(mnState, mnPoolState + NUM_MN_STATES * i, NUM_MN_STATES * sizeof(double));
			Izhikevich(mnState, mnInput);
			
			if (mnState[2] > 0)
			{
				mnSpikeCount += 1.0;	//the number of spikes
			} 
			memcpy(mnPoolState + NUM_MN_STATES * i, mnState, NUM_MN_STATES * sizeof(double));
		}
	}
	else
	{	
		for (int i = 0; i < NUM_MN; i++)
		{
			mnPoolState[3*i] = iniMNVolt;
			mnPoolState[3*i+1] = iniMNRecov;
			mnPoolState[3*i+2] = iniMNSpike; 		
		}	
		
	}

    
	// *** Muscle Fiber:: spikes (accumulated) -> Force

    double musDrive = loopParam[5];    
    double musTimeConst = loopParam[6];    
    double musPeakForce = loopParam[7]; 
	
    double nowF = loopState[3];
    double preF = loopState[4];
    
    double C = musTimeConst;
	double P = musPeakForce;
	double h = timeInterval;    //???? CHECK it out
    
		
	double fiberState[3];
	fiberState[0] = C/(exp(1)*P*h*h)-1/(exp(1)*P*h);         // define the discretization of the ODE 
	fiberState[1] = -2*C/(exp(1)*P*h*h)+1/(exp(1)*P*C);
	fiberState[2] = C/(exp(1)*P*h*h)+1/(exp(1)*P*h);
	double u;
	double f;	
		
	u = musDrive/h;
	f = (u- fiberState[0]* preF - fiberState[1] * nowF)/fiberState[2];
	
    preF = nowF;
    nowF = f;
    
    
	// *** Spindle :: (Lce, Gd, Gs) -> Ia firing rate (pps)
 	double spindleState[14];
	
	double spindleInput[4];

	spindleInput[0] = loopParam[3]; //gd
	spindleInput[1] = loopParam[14]; //lce 
	spindleInput[2] = timeInterval / (double) CYCLES_OF_INT; //a tenth of dt
	spindleInput[3] = loopParam[4];; //gs	
    
    //Bag I
	spindleState[0] = loopState[3]; // x0
	spindleState[1] = loopState[4]; // x1
	spindleState[2] = loopState[5]; // x2
	spindleState[3] = loopState[6]; // Ia firing rate
	spindleState[4] = loopState[10]; // dx0
	spindleState[5] = loopState[11]; // dx1
	spindleState[6] = loopState[12]; // dx2
    
    //Bag II
	spindleState[7] = loopState[13]; // x3
	spindleState[8] = loopState[14]; // x4
	spindleState[9] = loopState[15]; // x5
	spindleState[10] = loopState[16]; // IIa firing rate
	spindleState[11] = loopState[17]; // dx3
	spindleState[12] = loopState[18]; // dx4
	spindleState[13] = loopState[19]; // dx5    
	
	for (int j = 0; j < 10; j++) 
	{
		Spindle(spindleState, spindleInput);
	}   
    
	// *** Ia Sensory Neuron :: Ia postsyn current -> Ia_spike
	double snState[NUM_SN_STATES];
	double snInput[3];
    
	snInput[0] = loopParam[13]; // current input to sensory neuron
	snInput[1] = digSpikeSize;  // spike size 
	snInput[2] = timeInterval; // dt
	
    //TODO ADD THE OTHER VAR
    double snSpikeCount = 0.0;
    
	if(reset > 0.01) // RESET -> initialize all motoneuron pool
	{
		for (int i = 0; i < NUM_SN; i++)
		{
			memcpy(snState, snPoolState + NUM_SN_STATES * i, NUM_SN_STATES * sizeof(double));
			Izhikevich(snState, snInput);
			
			if (snState[2] > 0)
			{
				snSpikeCount += 1.0;	//the number of spikes
			} 
			memcpy(snPoolState + NUM_SN_STATES * i, snState, NUM_SN_STATES* sizeof(double));
		}
	}
	else
	{	
		for (int i = 0; i < NUM_SN; i++)
		{
			snPoolState[3*i] = iniMNVolt;
			snPoolState[3*i+1] = iniMNRecov;
			snPoolState[3*i+2] = iniMNSpike; 		
		}	
		
	}
    
    // *** synapse :: Ia_firingrate -> Ia_postsyn current
    double synState[NUM_SYNAPSE_STATES]; 
    double synInput[1];
    
	if(reset > 0.01) // RESET -> initialize all sensory pool
	{
		for (int i = 0; i < NUM_SYN; i++)
		{
			memcpy(synState, synPoolState + NUM_SYNAPSE_STATES * i, NUM_SYNAPSE_STATES*sizeof(double));
            synInput[0]= snPoolState[NUM_SN_STATES * i + 2];    
            
			Synapse(synState, synInput);
			memcpy(synPoolState + NUM_SYNAPSE_STATES * i, mnState, NUM_SYNAPSE_STATES*sizeof(double));
		}
	}    
    
	if(reset > 0.01)
	{
        loopState[2] = mnSpikeCount;	//the number of spikes

        loopState[3] = nowF;  // 
		loopState[4] = preF ; // 
		
        //Bag I
        
		loopState[5] = spindleState[0] ; // x0
		loopState[6] = spindleState[1] ;// x1
		loopState[7] = spindleState[2] ; // x2
		loopState[8] = spindleState[3] ;// Ia firing rate
		loopState[9] = spindleState[4] ; //d x0
		loopState[10] = spindleState[5] ;// dx1
		loopState[11] = spindleState[6] ; // dx2
        
        //Bag II
        
        loopState[12] = spindleState[7] ; // x3
        loopState[13] = spindleState[8] ; // x4
        loopState[14] = spindleState[9] ; // x5
        loopState[15] = spindleState[10]; // IIa firing rate
        loopState[16] = spindleState[11]; // dx3
        loopState[17] = spindleState[12]; // dx4
        loopState[18] = spindleState[13]; // dx5
                
        loopState[19] = snSpikeCount;	//the number of spikes
        loopState[20] = synState[0];	//synapse output

	}
	else    //?????
	{
        loopState[2] = 0.0;	//the number of spikes

        loopState[3] = 0.0;  // 
		loopState[4] = 0.0 ; // 
		
        //Bag I
        
		loopState[5] = 0.0 ; // x0
		loopState[6] = 0.9579 ;// x1
		loopState[7] = 0.0 ; // x2
		loopState[8] = 0.0 ;// Ia firing rate
		loopState[9] = 0.0 ; //d x0
		loopState[10] = 0.0 ;// dx1
		loopState[11] = 0.0 ; // dx2
        
        //Bag II
        
        loopState[12] = 0.0 ; // x3
        loopState[13] = 0.9579 ; // x4
        loopState[14] = 0.0 ; // x5
        loopState[15] = 0.0; // IIa firing rate
        loopState[16] = 0.0; // dx3
        loopState[17] = 0.0; // dx4
        loopState[18] = 0.0; // dx5
                
        loopState[19] = 0.0;
        loopState[20] = 0.0;	//synapse output
        
	}    
    
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

  
  
  if (vv >= TH) // if spikes    
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

    //Declarations for Bag I
    
	double KSR = 10.4649;
	double KPR = 0.1127;
	double B0DAMP=0.0605;
	double BDAMP=0.2356;
	double F0ACT=0.0;
	double FACT=0.0289;
	double XII=1.0;
	double LPRN=1.0;
	double GI = 20000.0;
	double GII = 20000.0;
	double ANONLINEAR = 0.25;	//0.3 in original
	double RLDFV=0.46;
	double LSR0 = 0.04;
	double LPR0 = 0.76;
	double L2ND = 1.00;
	double TAO = 0.192;
	double MASS = 0.0002;
	double FSAT = 1.00;

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
    
 
    
    //*** BAG I
    
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
        (KSR * lce - 
            (KSR + KPR) * x1 - 
            (B0DAMP + BDAMP * x0) * sig * CSS * pow(fabs(x2), ANONLINEAR) - 
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
    
    
    //Declarations for Bag II
    
    double x3_prev = spindle_state[7];
	double x4_prev = spindle_state[8];
	double x5_prev = spindle_state[9];
    
    double fib_bag2 = spindle_state[10];
    
    double dx3_prev = spindle_state[11];
	double dx4_prev = spindle_state[12];
	double dx5_prev = spindle_state[13];
    
    double gs = spindle_input[3];   //Change this in Update
	
    double dx3;
	double dx4;
	double dx5;
    
    double mings;
    double CSS_bag2;
    double sig_bag2;
        
    double xx3, xx4, xx5, x3, x4, x5;    
    //*** BAG II
    
    KSR = 10.4649;
	KPR = 0.1623;
	B0DAMP =0.0822;
	BDAMP =-0.046;
	F0ACT =0.0;
	FACT =0.0636;
	XII =0.7;
	LPRN =0.92;
	GI = 10000.0;
	GII = 7250.0;
	ANONLINEAR = 0.3;	//0.3 in original
	RLDFV=0.46;
	LSR0 = 0.04;
	LPR0 = 0.76;
	L2ND = 0.04;
	TAO = 0.185;
	MASS = 0.0002;
	FSAT = 0.50;
    
    
	x3 = x3_prev + dx3_prev * DT;
	x4 = x4_prev + dx4_prev * DT;
	x5 = x5_prev + dx5_prev * DT;
    
    mings = gs*gs / (gs*gs + freq*freq);
	
    dx3 = (mings - x3)/ 0.205;
    
    dx4 = x5;
    
    if (x5 < 0.0)
	CSS_bag2 = -1.0;
	else 
	CSS_bag2 = 1.0;
    
    sig_bag2 = x4 - RLDFV;
    sig_bag2 = (sig_bag2 > 0) ? sig_bag2 : 0.0 ;
    
    dx5 = (1.0 / MASS) * 
        (KSR * lce - 
            (KSR + KPR) * x4 -
            (B0DAMP + BDAMP * x3) * sig_bag2 * CSS_bag2 * pow(fabs(x5), ANONLINEAR) -
            (FACT * x3) -
            KSR*LSR0 + KPR*LPR0 );
    
    xx3 = x3 + DT * (dx3 + dx3_prev)/2.0;
    xx4 = x4 + DT * (dx4 + dx4_prev)/2.0;
    xx5 = x5 + DT * (dx5 + dx5_prev)/2.0;

    fib_bag2 = GI * (lce - xx4 - LSR0);
	fib_bag2 = (fib_bag2 >= 0.0 && fib_bag2 <= 100000.0) ? fib_bag2 : (fib_bag2 >100000.0 ? 100000.0: 0.0);

    spindle_state[7] = xx3;
	spindle_state[8] = xx4;
	spindle_state[9] = xx5;
	spindle_state[10] = fib_bag2;
	spindle_state[11] = dx3;
	spindle_state[12] = dx4;
	spindle_state[13] = dx5;   
    
}

//Chain

void Synapse(double *neuron_state, double *neuron_input)
{

}


