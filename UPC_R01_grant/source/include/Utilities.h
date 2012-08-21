#ifndef CMN_UtilityHeader
#define CMN_UtilityHeader

#define		BG_INERTIA 3.0 //background inertia 3.0kg
#define		PI 3.14159265
#define		EPS 0.0000001
#define		PosX 0
#define		PosY 1
#define		PosZ 2
#define		NUM_THREADS 2
#define     NUM_AUXVAR 4 // { force0, force1, pos0, pos1}
#define		TRAJ_TIME 2.0
#define		FzColorFact 0.8
#define		ForceBufLeng 500
#define		REACH_TARGET_DIST 0.01
#define		REACH_TARGET_VEL 0.1
#define		REACH_TARGET_DELAY 0.5
#define		DISPLAY_POS_TH 0.02
#define		DISPLAY_VEL_TH 0.05
#define		EMG_SAMPLING_RATE 1000 // samples/s
#define		INIT_HEIGHT -0.05
#define		SERVO_SAMPLING_RATE 150 // in Hz, Sampling Constants for Servoing

/* Motor States for FSM*/
#define MOTOR_STATE_INIT 0
#define MOTOR_STATE_WINDING_UP 1
#define MOTOR_STATE_OPEN_LOOP 2
#define MOTOR_STATE_CLOSED_LOOP 3
#define MOTOR_STATE_SHUTTING_DOWN 4

/* Safety Configurations */
#define		MAX_VOLT 3.0
#define		SAFE_MOTOR_VOLTAGE 0.4
#define		ZERO_MOTOR_VOLTAGE 0.0
#define     NUM_MOTOR 2
#define     NUM_FPGA 2

/* FPGA Trigger events */
const int    DATA_EVT_CLKRATE = 0;


/* EVENT FLAGS */
#define		MAX_PTB_NUM 6
#define		MAX_PRACTICE_NUM 3
#define		EVENT_NUM 6
const int	FLG_EVENT_STOP = 0;
const int	FLG_EVENT_INITPOS = 1;
const int	FLG_EVENT_PREP = 2;
const int	FLG_EVENT_PTB = 3;
const int	FLG_EVENT_FREEMOVE = 4;
const int	FLG_EVENT_REACH = 5;
#define		VISUAL_GAIN 0.02

/* PRACTICE FLAGS */

#define		PRACTICE_NUM 3
#define		FLG_PRACTICE_OFF 0
#define		FLG_PRACTICE_TARGET 1
#define		FLG_PRACTICE_BOTH 2



#define		TARGET_ON 1
#define		TARGET_OFF 0
#define		TARGET_JUMP -1
#define		TARGET_TH -2
#define		CURSOR_ON 1
#define		CURSOR_OFF 0
#define		CURSOR_TH -1

#define		PthreadMutexLock pthread_mutex_lock 
#define		PthreadMutexUnlock pthread_mutex_unlock 

#define		CHANNEL_NUM 2

#define		MAX_SAMPLE_NUM 10 * EMG_SAMPLING_RATE // 10 sec = ? Samples
#include <string>

extern std::string paraName;

/*-------------------------------------------------------------------------*/
/*
  @brief	ParaType type

  This struct contains all the paradigm parameters.
*/
/*-------------------------------------------------------------------------*/
typedef	struct	
_ParaType_ 
{
	int trialNum;

	double xInit;
	double yInit;
	double zInit;

	double xTarget; 
	double yTarget;
	double zTarget;

	double xMiddle;
	double yMiddle;
	double zMiddle;

	int		middleSwitch;

	double loadCoef[3], elasticOrigin;

	int trigMode;
	double trigThreshold;
	double trigDelay;
	double	delayM;
	int		ptbTypeNum;
	int		IP[4];




	int		sequence[1000];
} ParaType;


char* 
ToChar(std::string srcStr);

#endif // CMN_UtilityHeader