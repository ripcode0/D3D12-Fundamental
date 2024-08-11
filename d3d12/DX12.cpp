#include "DX12.h"
#include "config.h"

ID3D12Device* DX12::device{};
//only for resources
ID3D12CommandQueue*     DX12::cmdqueue{};
ID3D12CommandAllocator* DX12::cmdallocator{};
ID3D12GraphicsCommandList*      DX12::cmdlist{};

UINT64        DX12::sub_fence_value = 0;
HANDLE        DX12::sub_fence_event = nullptr;
ID3D12Fence*  DX12::sub_fence       = nullptr;

void DX12::initialize(ID3D12Device *p_device)
{
    device = p_device;

    //create the command queue

    D3D12_COMMAND_QUEUE_DESC desc{};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HR(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cmdqueue)));

    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdallocator));

    device->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT, cmdallocator, nullptr, IID_PPV_ARGS(&cmdlist));

    HR(cmdlist->Close());

    //create the sub fence

    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&sub_fence));
    set_name(sub_fence, "DX::sub_fence");

    sub_fence_value = 0;

    sub_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);

}

void DX12::release() 
{
    safe_release(cmdlist);
    safe_release(cmdallocator);
    safe_release(cmdqueue);

    CloseHandle(sub_fence_event);
    sub_fence_event = nullptr;

    safe_release(sub_fence);
}

void DX12::fence() 
{
    sub_fence_value++;
    HR(cmdqueue->Signal(sub_fence, sub_fence_value));
}

void DX12::wait_for_gpu_fence() 
{
    const UINT64 expected_fence_value = sub_fence_value;

    if(sub_fence->GetCompletedValue() < expected_fence_value){
      sub_fence->SetEventOnCompletion(expected_fence_value, sub_fence_event);
      ::WaitForSingleObject(sub_fence_event, INFINITE);
    }
}

HRESULT DX12::create_resource_system_memory(
    D3D12_RESOURCE_STATES dest_state, void *p_data,
    uint cb_size, ID3D12Resource **pp_resources)
{
    if (*pp_resources)
        return E_INVALIDARG;

    HRESULT res = S_OK;

    // create resource on system memory
    CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resource_desc(CD3DX12_RESOURCE_DESC::Buffer(cb_size));

    res = device->CreateCommittedResource(
        &heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(pp_resources));

    ID3D12Resource *cpu_resource = *pp_resources;
    CD3DX12_RANGE range{0, 0};
    void *p_begin_data = nullptr;
    res = cpu_resource->Map(0, &range, &p_begin_data);
    memcpy(p_begin_data, p_data, cb_size);
    cpu_resource->Unmap(0, nullptr);

    return res;
}

HRESULT DX12::create_resource_gpu_memory(
    void *src_data,
    uint srcs_size,
    ID3D12Resource **pp_resource)
{
    ID3D12Resource* buffer{};

    CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_DEFAULT);

    CD3DX12_RESOURCE_DESC desc(CD3DX12_RESOURCE_DESC::Buffer(srcs_size));
    
    HR(device->CreateCommittedResource(
      &heap_props,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS(&buffer)
    ));

    heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    ID3D12Resource* upload_buffer{};

    HR(device->CreateCommittedResource(
      &heap_props,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&upload_buffer)
    ));
    
    uint8_t* p_data{};
    CD3DX12_RANGE range(0,0);
    HR(upload_buffer->Map(0, &range, (void**)&p_data));
    memcpy(p_data, src_data, srcs_size);
    upload_buffer->Unmap(0, nullptr);

    HR(cmdallocator->Reset());

    HR(cmdlist->Reset(cmdallocator, nullptr));

    cmdlist->CopyBufferRegion(buffer, 0, upload_buffer, 0, srcs_size);    
    cmdlist->ResourceBarrier(1, 
      &CD3DX12_RESOURCE_BARRIER::Transition(buffer,
      D3D12_RESOURCE_STATE_COPY_DEST,   
      D3D12_RESOURCE_STATE_GENERIC_READ));  
  
    cmdlist->Close();

    ID3D12CommandList* cmd_lists[] = {cmdlist};
    cmdqueue->ExecuteCommandLists(_countof(cmd_lists), cmd_lists);

    fence();

    wait_for_gpu_fence();

    *pp_resource = buffer;

    safe_release(upload_buffer);
    //dx12 making thread safe
    return S_OK;
}

HRESULT DX12::create_constant_buffer(
    void *src_data, 
    uint src_size,   
    ID3D12Resource **pp_resource)
{

    return E_NOTIMPL;
}

#include <cwchar>
#include <cstring>

void DX12::compile_from_file(
    const char *file,
    const char *entry,
    const char* version,
    ID3DBlob **pp_blob)
{
    const size_t len = std::strlen(file) + 1;

    wchar_t wide_file[128];

    std::mbstowcs(wide_file, file, len);

    UINT flag{};
#if defined(_DEBUG)
    flag |= D3DCOMPILE_DEBUG;
#endif
    ID3DBlob* err_blob{};
    HRESULT res = ::D3DCompileFromFile(
        wide_file,
        nullptr,
        nullptr, 
        entry,
        version,
        flag,
        0,
        pp_blob,
        &err_blob
    );

    if(FAILED(res)){
        if(err_blob){
            
            MessageBoxA(NULL, (char*)err_blob->GetBufferPointer(), "Compile Error", MB_OK);
            safe_release(err_blob);
        }
    }
    safe_release(err_blob);
}
