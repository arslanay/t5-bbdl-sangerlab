
using namespace std;

//------------------------------------------------------------------------
// DESTester.CPP
//
// This is the C++ source file for the FPGA-based DES encryptor/decryptor.
// This source calls the Opal Kelly FrontPanel C++ API to perform all
// communication with the device including:
//  - PLL Configuration
//  - Spartan-3 configuration file download
//  - Data transfer for the DES block.
//
// The general operation is as follows:
// 1. The device PLL is configured to generate appropriate clocks for
//    the hardware we have implemented.
// 2. A Xilinx configuration file is downloaded to the FPGA to configure
//    it with our hardware.  This file was generated using the Xilinx
//    tools.
// 3. Hardware setup is performed:
//    a. A reset signal is sent using a WireIn.
//    b. The DES key (64-bits) is set using 8 separate WireIns.
//    c. Another WireIn is set to control the DES 'decrypt' input.
//    d. The input and output files are opened.
//    e. The transfer block RAM address pointer is reset using a TriggerIn.
// 4. The DES algorithm is continually run on data from the input file
//    until the input is exhausted.  Results are stored in the output file:
//    a. A block of 2048 bytes is read from the input.
//    b. This block is sent to the FPGA using okCFrontPanel::WriteToPipeIn(...)
//    c. A TriggerIn is activated to start the DES state machine.
//    d. A block of 2048 bytes is read from the FPGA using the method
//       okCXEM::ReadFromPipeOut(...)
//    e. The block is written to the output file.
//    f. This sequence is repeated until the input file is exhausted.
//
//
// Copyright (c) 2004-2009
// Opal Kelly Incorporated
// $Rev: 1010 $ $Date: 2011-10-08 17:49:29 -0700 (Sat, 08 Oct 2011) $
//------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>

#include "okFrontPanelDLL.h"

/*
*	Name:			reflex_single_muscle

*	Author:			C. Minos Niu

*	Email:			minos.niu AT sangerlab.net

*/

#include	<fstream>
#include	<math.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<time.h>
#include	"NIDAQmx.h"
#include	"PxiDAQ.h"
#include	"Utilities.h"
#include	"okFrontPanelDLL.h"


#define DES_FPGA_CONFIGURATION_FILE   "des.bit"
#if defined(_WIN32)
  #define strncpy strncpy_s
  #define sscanf  sscanf_s
#endif

bool
performDES(okCFrontPanel *xem, unsigned char key[8],
		   char *infilename, char *outfilename, bool decrypt)
{
	unsigned char buf[2048];
	long len;
	int i, j;
	std::ifstream f_in;
	std::ofstream f_out;
	int got;


	// Assert then deassert RESET.
	xem->SetWireInValue(0x10, 0xff, 0x01);
	xem->UpdateWireIns();
	xem->SetWireInValue(0x10, 0x00, 0x01);
	xem->UpdateWireIns();

	// Set the DES key value (WireIns).
	for (i=0; i<8; i++)
		xem->SetWireInValue(0x0f-i, key[i], 0xff);

	// Set the encrypt/decrypt bit (WireIn).
	if (decrypt)
		xem->SetWireInValue(0x10, 0xff, 0x10);
	else
		xem->SetWireInValue(0x10, 0x00, 0x10);

	xem->UpdateWireIns();

	// Open our input and output files.
	f_in.open(infilename, std::ios::binary);
	if (false == f_in.is_open()) {
		printf("Error: Input file could not be opened.\n");
		return(false);
	}
	f_out.open(outfilename, std::ios::binary);
	if (false == f_out.is_open()) {
		printf("Error: Output file could not be opened.\n");
		return(false);
	}

	// Reset the RAM address pointer.
	xem->ActivateTriggerIn(0x41, 0);
	
	// This block works through the file in 2048-byte chunks, writing
	// each one to the FPGA and initiating the DES processing.  It then
	// reads out the results and stores them in the output file.
	while (1) {
		// Read a block from the file.
		if (f_in.eof())
			break;
		f_in.read((char *)buf, 2048);

		// Zero-pad to the end of a block.
		got = (int)f_in.gcount();
		if (got == 0)
			break;
		if (got < 2048)
			for (i=got; i<2048; buf[i++] = 0);

		// Write a block of data.
		xem->WriteToPipeIn(0x80, 2048, buf);

		// Perform DES on the block.
		xem->ActivateTriggerIn(0x40, 0);

		// Wait for the TriggerOut indicating DONE.
		for (j=0; j<10; j++) {
			xem->UpdateTriggerOuts();
			if (xem->IsTriggered(0x60, 1))
				break;
		}
		if (j==10) {
			printf("DES did not complete properly.\n");
			return(false);
		}

		// Read the resulting RAM block.
		len = 2048;
		xem->ReadFromPipeOut(0xA0, len, buf);

		// Write the block to the output file.
		f_out.write((char *)buf, 2048);
	}

	f_in.close();
	f_out.close();

	return(true);
}


okCFrontPanel *
initializeFPGA()
{
	okCFrontPanel *dev;

	// Open the first XEM - try all board types.
	dev = new okCFrontPanel;
	if (okCFrontPanel::NoError != dev->OpenBySerial()) {
		delete dev;
		printf("Device could not be opened.  Is one connected?\n");
		return(NULL);
	}
	
	printf("Found a device: %s\n", dev->GetBoardModelString(dev->GetBoardModel()).c_str());

	dev->LoadDefaultPLLConfiguration();	

	// Get some general information about the XEM.
	std::string str;
	printf("Device firmware version: %d.%d\n", dev->GetDeviceMajorVersion(), dev->GetDeviceMinorVersion());
	str = dev->GetSerialNumber();
	printf("Device serial number: %s\n", str.c_str());
	str = dev->GetDeviceID();
	printf("Device device ID: %s\n", str.c_str());

	// Download the configuration file.
	if (okCFrontPanel::NoError != dev->ConfigureFPGA(DES_FPGA_CONFIGURATION_FILE)) {
		printf("FPGA configuration failed.\n");
		delete dev;
		return(NULL);
	}

	// Check for FrontPanel support in the FPGA configuration.
	if (dev->IsFrontPanelEnabled())
		printf("FrontPanel support is enabled.\n");
	else
		printf("FrontPanel support is not enabled.\n");

	return(dev);
}


static void
printUsage(char *progname)
{
	printf("Usage: %s [d|e] key infile outfile\n", progname);
	printf("   [d|e]   - d to decrypt the input file.  e to encrypt it.\n");
	printf("   key     - 64-bit hexadecimal string used for the key.\n");
	printf("   infile  - an input file to encrypt/decrypt.\n");
	printf("   outfile - destination output file.\n");
}


int
main(int argc, char *argv[])
{
	bool decrypt;
	char infilename[128];
	char outfilename[128];
	char keystr[32];
	unsigned char key[8];
	char dll_date[32], dll_time[32];
	int i, j;

	printf("---- Opal Kelly ---- FPGA-DES Application v1.0 ----\n");
	if (FALSE == okFrontPanelDLL_LoadLib(NULL)) {
		printf("FrontPanel DLL could not be loaded.\n");
		return(-1);
	}
	okFrontPanelDLL_GetVersion(dll_date, dll_time);
	printf("FrontPanel DLL loaded.  Built: %s  %s\n", dll_date, dll_time);


	if (argc != 5) {
		printUsage(argv[0]);
		return(-1);
	}

	strncpy(keystr, argv[2], 16);
	strncpy(infilename, argv[3], 128);
	strncpy(outfilename, argv[4], 128);

	if (argv[1][0] == 'd') {
		decrypt = true;
	} else if (argv[1][0] == 'e') {
		decrypt = false;
	} else {
		printUsage(argv[0]);
		return(-1);
	}

	if (strlen(argv[2]) < 16) {
		printUsage(argv[0]);
		return(-1);
	}

	for (i=0; i<8; i++) {
		sscanf(keystr+i*2, "%02x", &j);
		key[i] = j;
	}

	// Initialize the FPGA with our configuration bitfile.
	okCFrontPanel *xem;
	xem = initializeFPGA();
	if (NULL == xem) {
		printf("FPGA could not be initialized.\n");
		return(-1);
	}

	// Now perform the encryption/decryption process.
	if (performDES(xem, key, infilename, outfilename, decrypt) == false) {
		printf("DES process failed.\n");
		return(-1);
	} else {
		printf("DES process succeeded!\n");
	}

	return(0);
}
