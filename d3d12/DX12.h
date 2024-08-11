#pragma once

#include "config.h"

struct raw_buffer
{
    ~raw_buffer(){ safe_release(resource);}
    ID3D12Resource* resource;
};

struct vertex_buffer_create_info
{
    void* pData;

};


struct vertex_buffer : raw_buffer
{
    vertex_buffer(const vertex_buffer_create_info* info){
        //DX12::create_resource_gpu_memory(info->pData, 100, &resource);
    }
    D3D12_VERTEX_BUFFER_VIEW view;
};



struct DX12
{
    static void initialize(ID3D12Device* p_device);
    static void release();
    // static void fence();
    // static void wait_for_gpu_fence();

    static HRESULT create_resource_system_memory(
        D3D12_RESOURCE_STATES dest_state,
        void* srcs_data,
        uint  srcs_size,
        _Out_ ID3D12Resource** pp_resources
    );
    
    static HRESULT create_resource_gpu_memory(
        void* src_data, uint srcs_size,
        _Out_ ID3D12Resource** pp_resource
    );

    static HRESULT create_constant_buffer(
        void* src_data, 
        uint src_size,
        _Out_ ID3D12Resource** pp_resource  
    );

    static void compile_from_file(
        const char* file,
        const char* entry,
        const char* version,
        _Out_ ID3DBlob** pp_blob
        );

private:
    static void fence();
    static void wait_for_gpu_fence();

    static ID3D12Device* device; //globar

    static ID3D12CommandQueue* cmdqueue; //globar
    static ID3D12CommandAllocator* cmdallocator;
    static ID3D12GraphicsCommandList* cmdlist; //for resources

    static UINT64 sub_fence_value;
    static HANDLE sub_fence_event;
    static ID3D12Fence* sub_fence;
};


