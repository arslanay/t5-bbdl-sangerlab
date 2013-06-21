#include <string>
#include <windows.h>
#include "Utilities.h"
#include <fstream>
#include <vector>
#include <assert.h>
#include	"NIDAQmx.h"
#include "okFrontPanelDLL.h"
#include <iostream>


int ReInterpret(float32 in, int32 *out)
{
    memcpy(out, &in, sizeof(int32));
    return 0;
}

int ReInterpret(int32 in, int32 *out)
{
    memcpy(out, &in, sizeof(int32));
    return 0;
}

int ReInterpret(int32 in, float32 *out)
{
    memcpy(out, &in, sizeof(float32));
    return 0;
}

FileContainer::FileContainer()
{
    // Create new swap files with specified lengths
    std::ofstream ofsRTF("C:\\tmp\\RobotToFpga.dat", std::ios::binary | std::ios::out);
    ofsRTF.seekp((1024) - 1);
    ofsRTF.write("", 1);

    std::ofstream ofsFTR("C:\\tmp\\FpgaToRobot.dat", std::ios::binary | std::ios::out);
    ofsFTR.seekp((1024) - 1);
    ofsFTR.write("", 1);

    // Create a memory-mapped file (MMF)
    // c.f.: http://www.codeproject.com/Articles/11843/Embedding-Python-in-C-C-Part-II

    hFileRobotToFpga = CreateFile((LPCTSTR) "C:\\tmp\\RobotToFpga.dat", 
        GENERIC_WRITE | GENERIC_READ, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    hFileFpgaToRobot = CreateFile((LPCTSTR) "C:\\tmp\\FpgaToRobot.dat", 
        GENERIC_WRITE | GENERIC_READ, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    //if(hFileRobotToFpga == INVALID_HANDLE_VALUE) return 1;
    //if(hFileFpgaToRobot == INVALID_HANDLE_VALUE) return 2;


    tempRobotToFpga = CreateFileMapping(hFileRobotToFpga, NULL, 
        PAGE_READWRITE, 0, 1024, "MMAPShmem");
    mmapFpgaToRobot = (char *) MapViewOfFile (tempRobotToFpga, 
        FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0); 
    tempFpgaToRobot = CreateFileMapping(hFileFpgaToRobot, NULL, 
        PAGE_READWRITE, 0, 1024, "MMAPShmem");
    mmapFpgaToRobot = (char *) MapViewOfFile (tempFpgaToRobot, 
        FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0); 

    if(NULL != mmapFpgaToRobot)
    {    
        printf("Wrapper has created a MMAP for file 'C:\\tmp\\FpgaToRobot.dat'\n");
    };

    if(NULL != mmapFpgaToRobot)
    {    
        printf("Wrapper has created a MMAP for file 'C:\\tmp\\RobotToFpga.dat'\n");
    };
};

FileContainer::~FileContainer() 
{
    //WaitForSingleObject(handle,INFINITE);
    // Clean up
    UnmapViewOfFile(mmapFpgaToRobot);
    CloseHandle(tempFpgaToRobot);
    CloseHandle(hFileFpgaToRobot);

    UnmapViewOfFile(mmapRobotToFpga);
    CloseHandle(tempRobotToFpga);
    CloseHandle(hFileRobotToFpga);  
};

int SineGen(int (&data)[1024])
{
    float F = 12.0f; // in Hz
    float BIAS = 00000.0f;
    float AMP = 60000.0f;
    float PHASE = 0.0f;
    float SAMPLING_RATE = 1024.0f;
    float dt = 1.0f / SAMPLING_RATE; // Sampling interval in seconds
    float periods = 1.0;

    auto w = F * 2 * PI * dt;
    int max_n = 1024; //floor(periods * SAMPLING_RATE / F);
    printf("max_n = %d\n", max_n);
    for (int i = 0; i < max_n; i++)
    {
        data[i] = (int) (AMP * sin(w * i + PHASE) + BIAS);
    }

    return (0);
}

SomeFpga::SomeFpga(int NUM_NEURON = 128, int SAMPLING_RATE = 1024, std::string serX = "")
{
    this->NUM_NEURON = NUM_NEURON;
    this->SAMPLING_RATE = SAMPLING_RATE;
    
    this->xem = new okCFrontPanel;

    std::cout << "Connecting to OpaKelly of serial number: " << serX << std::endl;

    this->xem->OpenBySerial(serX);
    //assert(this->xem->IsOpen());
    this->xem->LoadDefaultPLLConfiguration();

}

SomeFpga::~SomeFpga()
{
    delete this->xem;
}

float SomeFpga::ReadFpga(int getAddr)
{
    int outValLo = this->xem->GetWireOutValue(getAddr) & 0xffff ;//# length = 16-bit
    int outValHi = this->xem->GetWireOutValue(getAddr + 0x01) & 0xffff;
    int outValBit = ((outValHi << 16) + outValLo) & 0xFFFFFFFF;
    float32 outVal;
    ReInterpret((int32) outValBit, &outVal);

    return outVal;
}

int SomeFpga::SendPara(int bitVal, int trigEvent)
{
    int bitValLo = bitVal & 0xffff;
    int bitValHi = (bitVal >> 16) & 0xffff;
    this->xem->SetWireInValue(0x01, bitValLo, 0xffff);
    this->xem->SetWireInValue(0x02, bitValHi, 0xffff);
    this->xem->UpdateWireIns();            
    this->xem->ActivateTriggerIn(0x50, trigEvent)  ;
    return 0;
}