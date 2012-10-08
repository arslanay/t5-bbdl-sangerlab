#include <string>
#include <windows.h>
#include "Utilities.h"
#include <fstream>
#include <vector>

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