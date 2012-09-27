#include <windows.h>

#ifndef CMN_UtilityHeader
#define CMN_UtilityHeader

#define     FPGA_BIT_FILENAME         "C:/nerf_sangerlab/projects/one_joint_robot/one_joint_robot_xem6010.bit"



#define		PI 3.14159265
#define		EPS 0.0000001
#define		NUM_THREADS 2
#define     NUM_AUXVAR 4 // { muscleLen, muscleVel, muscleForce, muscleEmg}
#define		EMG_SAMPLING_RATE 1000 // samples/s

/* Motor States for FSM*/
#define     MOTOR_STATE_INIT 0
#define     MOTOR_STATE_WINDING_UP 1
#define     MOTOR_STATE_OPEN_LOOP 2
#define     MOTOR_STATE_CLOSED_LOOP 3
#define     MOTOR_STATE_SHUTTING_DOWN 4

/* Safety Configurations */
#define		MAX_VOLT 8.0
#define		SAFE_MOTOR_VOLTAGE 0.9
#define		ZERO_MOTOR_VOLTAGE 0.0
#define     NUM_MOTOR 2
#define     NUM_FPGA 2

/* FPGA Trigger events */
const int    DATA_EVT_CLKRATE = 0;
const int    DATA_EVT_LCE = 8;
const int    DATA_EVT_VEL = 9;

/* PRACTICE FLAGS */


#define		PthreadMutexLock pthread_mutex_lock 
#define		PthreadMutexUnlock pthread_mutex_unlock 

#define		CHANNEL_NUM 2

#define		MAX_SAMPLE_NUM 10 * EMG_SAMPLING_RATE // 10 sec = ? Samples



class FileContainer 
{
    public:
        FileContainer();

        ~FileContainer();
    
    private:
        HANDLE tempRobotToFpga;
        HANDLE hFileRobotToFpga;
        HANDLE tempFpgaToRobot;
        HANDLE hFileFpgaToRobot;
        char* mmapFpgaToRobot;
        char* mmapRobotToFpga;    
}; // Semi-colon is REQUIRED!

#endif // CMN_UtilityHeader