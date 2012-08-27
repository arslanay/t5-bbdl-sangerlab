#include "HapticAPI.h"
#include "FcsHapticMaster.h"

#include  <stdio.h>

// This class is actually a wrapper class to the CFcsHapticMASTER class
// This enables running HapticMASTER initialisation in threads
// because it contains a CFcsHapticMASTER object together with some
// variables needed for Master-Slave operation
// It also provides some shortcuts to CFcsHapticMASTER functions
class CHapticMaster
{
public:
	CHapticMaster(char *pName="vmd");
	~CHapticMaster();
	void Init(void);

	int SetForceGetPV();
	int GetEeForce();
	
public:
	CFcsHapticMASTER *pHapticMaster;
	FCSSTATE CurrentState;
	char Name[10];
	bool bConnected;
	bool bInitDone;
	double dPos[3];
	double dVel[3];
	double dForce[3];
	double dEeForce[3];
};