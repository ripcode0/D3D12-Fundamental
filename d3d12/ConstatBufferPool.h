#pragma once

#include "config.h"

struct ConstatnBuffer
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle;
    D3D12_GPU_VIRTUAL_ADDRESS   gpu_addr;
    uint8_t*                    system_memory_addr;
};


class ConstantBufferPool
{
public:
    ConstatnBuffer* m_constant_buffer_list;
    ID3D12Resource* m_resource;

    uint8_t* m_system_mem_addr;
    

};