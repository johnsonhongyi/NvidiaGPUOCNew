// NvidiaGPUOCNew.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

/*
            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
*/

//http://hk.zeddal.com/2018/03/08/oc-nvidia-with-hidden-api-c/
//https://1vwjbxf1wko0yhnr.wordpress.com/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

typedef unsigned long NvU32;

typedef struct {
    NvU32   version;
    NvU32   ClockType : 2;
    NvU32   reserved : 22;
    NvU32   reserved1 : 8;
    struct {
        NvU32   bIsPresent : 1;
        NvU32   reserved : 31;
        NvU32   frequency;
    }domain[32];
} NV_GPU_CLOCK_FREQUENCIES_V2;

typedef struct {
    int value;
    struct {
        int   mindelta;
        int   maxdelta;
    } valueRange;
} NV_GPU_PERF_PSTATES20_PARAM_DELTA;

typedef struct {
    NvU32   domainId;
    NvU32   typeId;
    NvU32   bIsEditable : 1;
    NvU32   reserved : 31;
    NV_GPU_PERF_PSTATES20_PARAM_DELTA   freqDelta_kHz;
    union {
        struct {
            NvU32   freq_kHz;
        } single;
        struct {
            NvU32   minFreq_kHz;
            NvU32   maxFreq_kHz;
            NvU32   domainId;
            NvU32   minVoltage_uV;
            NvU32   maxVoltage_uV;
        } range;
    } data;
} NV_GPU_PSTATE20_CLOCK_ENTRY_V1;

typedef struct {
    NvU32   domainId;
    NvU32   bIsEditable : 1;
    NvU32   reserved : 31;
    NvU32   volt_uV;
    int     voltDelta_uV;
} NV_GPU_PSTATE20_BASE_VOLTAGE_ENTRY_V1;

typedef struct {
    NvU32   version;
    NvU32   bIsEditable : 1;
    NvU32   reserved : 31;
    NvU32   numPstates;
    NvU32   numClocks;
    NvU32   numBaseVoltages;
    struct {
        NvU32                                   pstateId;
        NvU32                                   bIsEditable : 1;
        NvU32                                   reserved : 31;
        NV_GPU_PSTATE20_CLOCK_ENTRY_V1          clocks[8];
        NV_GPU_PSTATE20_BASE_VOLTAGE_ENTRY_V1   baseVoltages[4];
    } pstates[16];
} NV_GPU_PERF_PSTATES20_INFO_V1;

typedef void* (*NvAPI_QueryInterface_t)(unsigned int offset);
typedef int (*NvAPI_Initialize_t)();
typedef int (*NvAPI_Unload_t)();
typedef int (*NvAPI_EnumPhysicalGPUs_t)(int** handles, int* count);
typedef int (*NvAPI_GPU_GetSystemType_t)(int* handle, int* systype);
typedef int (*NvAPI_GPU_GetFullName_t)(int* handle, char* sysname);
typedef int (*NvAPI_GPU_GetPhysicalFrameBufferSize_t)(int* handle, int* memsize);
typedef int (*NvAPI_GPU_GetRamType_t)(int* handle, int* memtype);
typedef int (*NvAPI_GPU_GetVbiosVersionString_t)(int* handle, char* biosname);
typedef int (*NvAPI_GPU_GetAllClockFrequencies_t)(int* handle, NV_GPU_PERF_PSTATES20_INFO_V1* pstates_info);
typedef int (*NvAPI_GPU_GetPstates20_t)(int* handle, NV_GPU_PERF_PSTATES20_INFO_V1* pstates_info);
typedef int (*NvAPI_GPU_SetPstates20_t)(int* handle, int* pstates_info);

//johnson
//typedef int (*NvAPI_GPU_GetPerfClocks_t)(int* handle, NV_GPU_CLOCK_FREQUENCIES_V2* perfClocks_info);
//_NvAPI_GPU_GetPerfClocks    1EA54A3Bh
//_NvAPI_GPU_SetPerfClocks    7BCF4ACh
//_NvAPI_GPU_GetPixelClockRange   66AF10B7h
//_NvAPI_GPU_SetPixelClockRange   5AC7F8E5h
//_NvAPI_GPU_GetAllClockFrequencies   0DCB616C3h
NvAPI_QueryInterface_t NvQueryInterface = 0;
NvAPI_Initialize_t NvInit = 0;
NvAPI_Unload_t NvUnload = 0;
NvAPI_EnumPhysicalGPUs_t NvEnumGPUs = 0;
NvAPI_GPU_GetSystemType_t NvGetSysType = 0;
NvAPI_GPU_GetFullName_t NvGetName = 0;
NvAPI_GPU_GetPhysicalFrameBufferSize_t NvGetMemSize = 0;
NvAPI_GPU_GetRamType_t NvGetMemType = 0;
NvAPI_GPU_GetVbiosVersionString_t NvGetBiosName = 0;
NvAPI_GPU_GetAllClockFrequencies_t NvGetFreq = 0;
NvAPI_GPU_GetPstates20_t NvGetPstates = 0;
NvAPI_GPU_SetPstates20_t NvSetPstates = 0;

//johnson
NvAPI_GPU_GetAllClockFrequencies_t NvGetClockFreq = 0;
//NvAPI_GPU_GetPerfClocks_t NvGetPerfClocks = 0;
//NvAPI_GPU_SetPerfClocks_t NvSetPerfClocks = 0;

int main(int argc, char** argv)
{
    int nGPU = 0, userfreq = 0, systype = 0, memsize = 0, memtype = 0;
    int* hdlGPU[64] = { 0 }, * buf = 0;
    char sysname[64] = { 0 }, biosname[64] = { 0 };
    NV_GPU_PERF_PSTATES20_INFO_V1 pstates_info;
    pstates_info.version = 0x11c94;

    NvQueryInterface = (void*)GetProcAddress(LoadLibrary("nvapi.dll"), "nvapi_QueryInterface");
    NvInit = NvQueryInterface(0x0150E828);
    NvUnload = NvQueryInterface(0xD22BDD7E);
    NvEnumGPUs = NvQueryInterface(0xE5AC921F);
    NvGetSysType = NvQueryInterface(0xBAAABFCC);
    NvGetName = NvQueryInterface(0xCEEE8E9F);
    NvGetMemSize = NvQueryInterface(0x46FBEB03);
    NvGetMemType = NvQueryInterface(0x57F7CAAC);
    NvGetBiosName = NvQueryInterface(0xA561FD7D);
    NvGetFreq = NvQueryInterface(0xDCB616C3);
    NvGetPstates = NvQueryInterface(0x6FF81213);
    NvSetPstates = NvQueryInterface(0x0F4DAE6B);
    
    //johnson

    NV_GPU_PERF_PSTATES20_INFO_V1 getClockFreqinfo;
    getClockFreqinfo.version = 0x11c94;
    NvGetClockFreq = NvQueryInterface(0x0DCB616C3);
  /*  NV_GPU_CLOCK_FREQUENCIES_V2 perfClocks_info;
    perfClocks_info.version = 0x11c94;
    NvGetPerfClocks = NvQueryInterface(0x1EA54A3B);*/


    NvInit();
    NvEnumGPUs(hdlGPU, &nGPU);
    NvGetSysType(hdlGPU[0], &systype);
    NvGetName(hdlGPU[0], sysname);
    NvGetMemSize(hdlGPU[0], &memsize);
    NvGetMemType(hdlGPU[0], &memtype);
    NvGetBiosName(hdlGPU[0], biosname);
    NvGetPstates(hdlGPU[0], &pstates_info);

    NvGetPerfClocks(hdlGPU[0], &getClockFreqinfo);
    switch (systype) {
    case 1:     printf("\nType: Laptop\n"); break;
    case 2:     printf("\nType: Desktop\n"); break;
    default:    printf("\nType: Unknown\n"); break;
    }
    printf("Name: %s\n", sysname);
    printf("VRAM: %dMB GDDR%d\n", memsize / 1024, memtype <= 7 ? 3 : 5);
    printf("BIOS: %s\n", biosname);
    printf("\nNowClock: %d GPUMax: %dMHz\n", (int)((pstates_info.pstates[0].clocks[0]).data.single.freq_kHz) / 1000,(int)((pstates_info.pstates[0].clocks[0]).data.range.maxFreq_kHz) / 1000);
    printf("RAM: %dMHz\n", (int)((pstates_info.pstates[0].clocks[1]).data.single.freq_kHz) / 1000);
    printf("\nCurrent GPU OC: %dMHz\n", (int)((pstates_info.pstates[0].clocks[0]).freqDelta_kHz.value) / 1000);
    printf("Current RAM OC: %dMHz\n", (int)((pstates_info.pstates[0].clocks[1]).freqDelta_kHz.value) / 1000);

    if (argc > 1) {
        userfreq = atoi(argv[1]) * 1000;
        if (-250000 <= userfreq && userfreq <= 250000) {
            buf = malloc(0x1c94);
            memset(buf, 0, 0x1c94);
            buf[0] = 0x11c94; buf[2] = 1; buf[3] = 1;
            buf[10] = userfreq;
            NvSetPstates(hdlGPU[0], buf) ? printf("\nGPU OC failed!\n") : printf("\nGPU OC OK: %d MHz\n", userfreq / 1000);
            free(buf);
        }
        else {
            printf("\nGPU Frequency not in safe range (-250MHz to +250MHz).\n");
            return 1;
        }
    }
    if (argc > 2) {
        userfreq = atoi(argv[2]) * 1000;
        if (-250000 <= userfreq && userfreq <= 250000) {
            buf = malloc(0x1c94);
            memset(buf, 0, 0x1c94);
            buf[0] = 0x11c94; buf[2] = 1; buf[3] = 1;
            buf[7] = 4; buf[10] = memtype <= 7 ? userfreq : userfreq * 2;
            NvSetPstates(hdlGPU[0], buf) ? printf("VRAM OC failed!\n") : printf("RAM OC OK: %d MHz\n", userfreq / 1000);
            free(buf);
        }
        else {
            printf("\nRAM Frequency not in safe range (-250MHz to +250MHz).\n");
            return 1;
        }
    }
    NvUnload();
    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
