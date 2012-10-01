#include <windows.h>
#include <functional>

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
const float		MAX_VOLT = 8.0;
const double		SAFE_MOTOR_VOLTAGE = 0.9;
const double		ZERO_MOTOR_VOLTAGE = 0.0;
const int    NUM_MOTOR = 2;
const int    NUM_FPGA = 2;

/* FPGA Trigger events */
const int    DATA_EVT_CLKRATE = 0;
const int    DATA_EVT_LCE = 8;
const int    DATA_EVT_VEL = 9;

/* PRACTICE FLAGS */


#define		PthreadMutexLock pthread_mutex_lock 
#define		PthreadMutexUnlock pthread_mutex_unlock 

#define		CHANNEL_NUM 2



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


#endif // CMN_UtilityHeader

