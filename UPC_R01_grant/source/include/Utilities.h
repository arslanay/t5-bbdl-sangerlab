#include <windows.h>
#include <functional>
#include <vector>
#include	"NIDAQmx.h"
#include "okFrontPanelDLL.h"

#ifndef CMN_UtilityHeader
#define CMN_UtilityHeader

#define     FPGA_BIT_FILENAME         "C:/nerf_sangerlab/projects/one_joint_robot_all_in1/one_joint_robot_all_in1_xem6010.bit"
const int   NUM_NEURON = 128;
const int   SAMPLING_RATE = 1024;

#define		PI 3.14159265
#define		EPS 0.0000001
#define		NUM_THREADS 3
#define     NUM_AUXVAR 4 // { muscleLen, muscleVel, muscleForce, muscleEmg}
#define		EMG_SAMPLING_RATE 1000 // samples/s

/* Motor States for FSM*/
#define     MOTOR_STATE_INIT 0
#define     MOTOR_STATE_WINDING_UP 1
#define     MOTOR_STATE_OPEN_LOOP 2
#define     MOTOR_STATE_CLOSED_LOOP 3
#define     MOTOR_STATE_SHUTTING_DOWN 4

/* Safety Configurations */
const float		MAX_VOLT = 6.0;
const double		SAFE_MOTOR_VOLTAGE = 0.6;
const double		ZERO_MOTOR_VOLTAGE = 0.0;
const int    NUM_MOTOR = 2;
const int    NUM_FPGA = 2;
const int    NUM_MUSCLE = 2;

/* FPGA Trigger events */
const int    DATA_EVT_CLKRATE = 0;
//const int    DATA_EVT_LCE = 9;
const int    DATA_EVT_GAMMA_DYN = 4;
const int    DATA_EVT_GAMMA_STA = 5;
//const int    DATA_EVT_VEL = 9;
const int    DATA_EVT_LCEVEL = 9;
//const int    DATA_EVT_M1_VOL = 11;
//const int    DATA_EVT_M1_DYS = 8;

/* FPGA Button events */
const int    BUTTON_RESET = 0;
const int    BUTTON_INPUT_FROM_TRIG = 1;
const int    BUTTON_RESET_SIM = 2;

/* PRACTICE FLAGS */


#define		PthreadMutexLock pthread_mutex_lock 
#define		PthreadMutexUnlock pthread_mutex_unlock 

#define		CHANNEL_NUM 2

int ReInterpret(float32 in, int32 *out);

int ReInterpret(int32 in, int32 *out);

int ReInterpret(int32 in, float32 *out);

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


/*  MINOS added: ScopeGuard class for memory-leak prevention
Usage: ON_SCOPE_EXIT(steps_to_take) */
class ScopeGuard
{
public:
    explicit ScopeGuard(std::function<void()> onExitScope)
        : onExitScope_(onExitScope), dismissed_(false)
    { }

    ~ScopeGuard()
    {
        if(!dismissed_)
        {
            onExitScope_();
        }
    }

    void Dismiss()
    {
        dismissed_ = true;
    }

private:
    std::function<void()> onExitScope_;
    bool dismissed_;

private: // noncopyable
    ScopeGuard(ScopeGuard const&);
    ScopeGuard& operator=(ScopeGuard const&);
};

#define SCOPEGUARD_LINENAME_CAT(name, line) name##line
#define SCOPEGUARD_LINENAME(name, line) SCOPEGUARD_LINENAME_CAT(name, line)
#define ON_SCOPE_EXIT(callback) ScopeGuard SCOPEGUARD_LINENAME(EXIT, __LINE__)(callback)

int SineGen(int (&data)[1024]);




class SomeFpga 
{
    public:
        SomeFpga(int , int , std::string);
        ~SomeFpga();
        int SendPara(int bitVal, int trigEvent);
        int SendButton(bool bitVal, int buttonEvt);
        int ReadFpga(BYTE getAddr, char *, float *);
        int ReadFpga(BYTE getAddr, char *,int *);
        //int ReadFpga(int getAddr);
        okCFrontPanel *xem;
        int WriteFpgaLceVel(int32, int32, int32, int32, int32);    
    private:
        int NUM_NEURON;
        int SAMPLING_RATE;
        char serX[50];
        


}; // Semi-colon is REQUIRED!



#endif // CMN_UtilityHeader
