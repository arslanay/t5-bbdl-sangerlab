// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

/* Shows how to use both command line and config file. */

#include <boost/program_options.hpp>
namespace po = boost::program_options;


#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include "Utilities.h"
#include	"ReadParadigm.h"
using namespace std;

// A helper function to simplify the main part.
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
	copy(v.begin(), v.end(), ostream_iterator<T>(cout, " ")); 
	return os;
}



int 
ReadParadigm(ParaType& pendingPara)
{
	po::variables_map paraOptions;
	try 
	{
		string	rawSequence;

		// Declare a group of options that will be 
		// allowed only on command line

		cout << endl << "Enter Paradigm Name (*.ini):";
		cin >> paraName;	// Use fgets to accommodate all characters,
		string taskFile("C:\\HapticMASTER\\People\\Mark\\Protocol\\" 
						+ paraName + ".ini");

		po::options_description paraDesc("Paradigm Descriptions");
		paraDesc.add_options()
			("Task.Initial X", 
				po::value<double>(&pendingPara.xInit)
				->default_value(0.0))
			("Task.Initial Y", 
				po::value<double>(&pendingPara.yInit)
				->default_value(0.0))    
			("Task.Initial Z", 
				po::value<double>(&pendingPara.zInit)
				->default_value(0.0))
			("Task.Target X", 
				po::value<double>(&pendingPara.xTarget)
				->default_value(0.0))    
			("Task.Target Y",
				po::value<double>(&pendingPara.yTarget)
				->default_value(0.0))
			("Task.Target Z",
				po::value<double>(&pendingPara.zTarget)
				->default_value(0.0))    
			("Task.Middle X", 
				po::value<double>(&pendingPara.xMiddle)
				->default_value(0.0))    
			("Task.Middle Y",
				po::value<double>(&pendingPara.yMiddle)
				->default_value(0.0))
			("Task.Middle Z",
				po::value<double>(&pendingPara.zMiddle)
				->default_value(0.0))    
			("Task.Middle Target",
				po::value<int>(&pendingPara.middleSwitch)
				->default_value(0))    
			("Task.Delay Secondary Motor",
				po::value<double>(&pendingPara.delayM)
				->default_value(1.0))
			("Task.Inertia",
				po::value<double>(&pendingPara.loadCoef[0])
				->default_value(0.0))    
			("Task.Damping",
				po::value<double>(&pendingPara.loadCoef[1])
				->default_value(0.0))
			("Task.Stiffness",
				po::value<double>(&pendingPara.loadCoef[2])
				->default_value(0.0))    
			("Task.Stiffness Origin",
				po::value<double>(&pendingPara.elasticOrigin)
				->default_value(0.0))    
			("Task.Trial Number",
				po::value<int>(&pendingPara.trialNum)
				->default_value(1))
			("Task.IPa",
				po::value<int>(&pendingPara.IP[0])
				->default_value(0))
			("Task.IPb",
				po::value<int>(&pendingPara.IP[1])
				->default_value(0))
			("Task.IPc",
				po::value<int>(&pendingPara.IP[2])
				->default_value(0))
			("Task.IPd",
				po::value<int>(&pendingPara.IP[3])
				->default_value(0))
			("Task.Ptb Type Number",
				po::value<int>(&pendingPara.ptbTypeNum)
				->default_value(1))
			("Task.Sequence",
				po::value<string>(&rawSequence))
			("Task.Trigger Mode",
				po::value<int>(&pendingPara.trigMode)
				->default_value(0))
			("Task.Trigger Threshold",
				po::value<double>(&pendingPara.trigThreshold)
				->default_value(10000.00))
			("Task.Trigger Delay",
				po::value<double>(&pendingPara.trigDelay)
				->default_value(0.0));
		;

		// Declare a group of options that will be 
		// allowed both on command line and in
		// config file

		ifstream ifs(taskFile.c_str());
		store(parse_config_file(ifs, paraDesc), paraOptions);
		notify(paraOptions);

		for (int i = 0; i < pendingPara.trialNum; i++)
		{
			int t = rawSequence.find(":");
			istringstream iss(rawSequence.substr(0, t));
			iss >> pendingPara.sequence[i];
			rawSequence = rawSequence.substr(t + 1, rawSequence.length() - 1);
		}// for parsing rawSequence[]

		cout << pendingPara.sequence << endl;
		return 0;
	}
	catch(exception& e)
	{
		cout << e.what() << "\n";
		return 1;
	}    
	return 0;
}

int
ReadDispPara(void)
{
	FILE* dispHandle;
	string dispFile("");
	dispFile = "C:/HapticMASTER/People/Mark/Protocol/"
		+ paraName + "_display.ini";
	fopen_s(&dispHandle, ToChar(dispFile),"r");

	if (dispHandle == NULL)
	{
		printf("\nError reading _display.ini file!\n");
		return 0;
	}
	else
	{
		printf("\nDisplay configuration file found!\n");
	}

	for (int ii = 0; ii < MAX_PRACTICE_NUM; ii++)
	{
		fscanf_s(dispHandle, "%d %d %d %d %d %d\n", 
			&cursorExpContext[ii][FLG_EVENT_STOP],
			&cursorExpContext[ii][FLG_EVENT_INITPOS],
			&cursorExpContext[ii][FLG_EVENT_PREP],
			&cursorExpContext[ii][FLG_EVENT_PTB],
			&cursorExpContext[ii][FLG_EVENT_FREEMOVE],
			&cursorExpContext[ii][FLG_EVENT_REACH]);
		fscanf_s(dispHandle, "%d %d %d %d %d %d\n", 
			&targetExpContext[ii][FLG_EVENT_STOP],
			&targetExpContext[ii][FLG_EVENT_INITPOS],
			&targetExpContext[ii][FLG_EVENT_PREP],
			&targetExpContext[ii][FLG_EVENT_PTB],
			&targetExpContext[ii][FLG_EVENT_FREEMOVE],
			&targetExpContext[ii][FLG_EVENT_REACH]);
		fscanf_s(dispHandle, "%d %d %d %d %d %d\n", 
			&iniposExpContext[ii][FLG_EVENT_STOP],
			&iniposExpContext[ii][FLG_EVENT_INITPOS],
			&iniposExpContext[ii][FLG_EVENT_PREP],
			&iniposExpContext[ii][FLG_EVENT_PTB],
			&iniposExpContext[ii][FLG_EVENT_FREEMOVE],
			&iniposExpContext[ii][FLG_EVENT_REACH]);
	}
	for (int ii = 0; ii < MAX_PRACTICE_NUM; ii++)
	{
		fscanf_s(dispHandle, "%d %d %d %d %d %d\n", 
			&cursorPtbContext[ii][FLG_EVENT_STOP],
			&cursorPtbContext[ii][FLG_EVENT_INITPOS],
			&cursorPtbContext[ii][FLG_EVENT_PREP],
			&cursorPtbContext[ii][FLG_EVENT_PTB],
			&cursorPtbContext[ii][FLG_EVENT_FREEMOVE],
			&cursorPtbContext[ii][FLG_EVENT_REACH]);
		fscanf_s(dispHandle, "%d %d %d %d %d %d\n", 
			&targetPtbContext[ii][FLG_EVENT_STOP],
			&targetPtbContext[ii][FLG_EVENT_INITPOS],
			&targetPtbContext[ii][FLG_EVENT_PREP],
			&targetPtbContext[ii][FLG_EVENT_PTB],
			&targetPtbContext[ii][FLG_EVENT_FREEMOVE],
			&targetPtbContext[ii][FLG_EVENT_REACH]);
		fscanf_s(dispHandle, "%d %d %d %d %d %d\n", 
			&iniposPtbContext[ii][FLG_EVENT_STOP],
			&iniposPtbContext[ii][FLG_EVENT_INITPOS],
			&iniposPtbContext[ii][FLG_EVENT_PREP],
			&iniposPtbContext[ii][FLG_EVENT_PTB],
			&iniposPtbContext[ii][FLG_EVENT_FREEMOVE],
			&iniposPtbContext[ii][FLG_EVENT_REACH]);
	}
	if (dispHandle != NULL)
	{
		fclose(dispHandle);
	}
	return 0;
}

