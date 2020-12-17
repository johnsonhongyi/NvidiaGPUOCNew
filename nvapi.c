int main()
{
    NvAPI_Status ret = NVAPI_OK;
    ret = NvAPI_Initialize();

    if (ret != NVAPI_OK)
    {
        printf("NvAPI_Initialize() failed = 0x%x", ret);
        return 1; // Initialization failed
    }

    NvPhysicalGpuHandle hPhysicalGpu[NVAPI_MAX_PHYSICAL_GPUS];

    for (NvU32 PhysicalGpuIndex = 0; PhysicalGpuIndex < NVAPI_MAX_PHYSICAL_GPUS; PhysicalGpuIndex++)
    {
        hPhysicalGpu[PhysicalGpuIndex] = 0;
    }

    ret = NvAPI_EnumPhysicalGPUs(hPhysicalGpu, &physicalGpuCount);
    if (ret != NVAPI_OK)
    {
        printf("Cannot enumerate GPUs in the system...\n");
        getchar();
        exit(1);
    }

    NV_GPU_THERMAL_SETTINGS currentTemp;//获取温度的数据结构
    currentTemp.version = NV_GPU_THERMAL_SETTINGS_VER;//一定要设置，不然调用获取温度函数时候会出错
    for (NvU32 GpuIndex = 0; GpuIndex < physicalGpuCount; GpuIndex++)
    {
        ret = NvAPI_GPU_GetThermalSettings(hPhysicalGpu[PhysicalGpuIndex], NVAPI_THERMAL_TARGET_ALL, ¤tTemp);//获取温度
        if (ret == NVAPI_OK)
        {
            printf("current temperature=%d\n", currentTemp.sensor[0].currentTemp);
        }

    }

    return 0;
}