#include <string>
#include <windows.h>
#include "Utilities.h"
#include <fstream>

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

    int my_array[5] = {1, 2, 3, 4, 5};
    for (int &x : my_array) {
        x *= 2;
    }
};

