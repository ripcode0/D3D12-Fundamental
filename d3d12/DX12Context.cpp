#include "DX12Context.h"
#include "DX12.h"

constexpr D3D_FEATURE_LEVEL min_feature{D3D_FEATURE_LEVEL_11_0};

// ID3D12Device*               DX12Context::m_device{};
// IDXGIFactory7*              DX12Context::dxgi_factory{};
// ID3D12CommandQueue*         DX12Context::command_queue{};
// ID3D12CommandAllocator*     DX12Context::command_allocator{};
// ID3D12GraphicsCommandList*  DX12Context::command_list{};

DX12Context::~DX12Context()
{
    release();
}

typedef unsigned int uint;

void DX12Context::initialize(uint cx, uint cy, HWND hwnd)
{
    m_hwnd = hwnd;
    DWORD factory_flag = 0;

#if defined(_DEBUG)
    ID3D12Debug3* debug_controller{};
    HR(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
    debug_controller->EnableDebugLayer();

    factory_flag |= DXGI_CREATE_FACTORY_DEBUG;
    safe_release(debug_controller);
#endif

    HR(CreateDXGIFactory2(factory_flag, IID_PPV_ARGS(&m_factory)));
    
    IDXGIAdapter4* adapter = get_adapter();

    auto max_feature = get_max_feature_level(adapter); //gtx950 _11 dx12 rtx4060 12_1
    //m_device
    HR(D3D12CreateDevice(adapter, max_feature, IID_PPV_ARGS(&m_device)));
    m_device->SetName(L"Context::device");
    
#if defined(_DEBUG)
    ID3D12InfoQueue* info_queue{};
    HR(m_device->QueryInterface(IID_PPV_ARGS(&info_queue)));
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
    safe_release(info_queue);
#endif
    
    safe_release(adapter);
    
    //safe_release(m_factory);

    create_command_queue();

    create_swapchain(cx, cy);

    create_descriptor_heaps();

    create_render_targets();

    create_depth_stencil(cx, cy);

    create_fence();

    DX12::initialize(m_device);

}

void DX12Context::release()
{
    wait_for_gpu_async();

    for(uint i = 0; i < frame_buffer_count; ++i){
        safe_release(m_render_targets[i]);
    }

    safe_release(m_rtv_heap);
    safe_release(m_dsv_heap);

    safe_release(m_depth_stencil);
    safe_release(m_swapchain);

    safe_release(m_command_queue);
    safe_release(m_command_list);
    safe_release(m_command_allocator);

    safe_release(m_factory);
    safe_release(m_fence);

    CloseHandle(m_fence_event);

    safe_release(m_device);


    DX12::release();
    
    IDXGIDebug1* debug{};
    DXGI_DEBUG_RLO_FLAGS flag = (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL);
    if(SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))){
        debug->ReportLiveObjects(DXGI_DEBUG_ALL, flag);
        safe_release(debug);
    }
    

}

IDXGIAdapter4 *DX12Context::get_adapter()
{
    IDXGIAdapter4* fixed_adapter{};
    for(UINT i = 0;
        m_factory->EnumAdapterByGpuPreference(
            i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&fixed_adapter)) != DXGI_ERROR_NOT_FOUND;
        ++i)
    {
        if(SUCCEEDED(D3D12CreateDevice(fixed_adapter, min_feature, __uuidof(IDXGIAdapter4), nullptr))){
            return fixed_adapter;
        }
        safe_release(fixed_adapter);
    }
    return nullptr;
}

D3D_FEATURE_LEVEL DX12Context::get_max_feature_level(IDXGIAdapter4 *fixed_adapter)
{
    constexpr D3D_FEATURE_LEVEL featrue_levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_12_1
    };

    D3D12_FEATURE_DATA_FEATURE_LEVELS feature_level_info{};
    feature_level_info.NumFeatureLevels = _countof(featrue_levels);
    feature_level_info.pFeatureLevelsRequested = featrue_levels;

    ID3D12Device* temp_device{};
    HR(D3D12CreateDevice(fixed_adapter, min_feature, IID_PPV_ARGS(&temp_device)));

    HR(temp_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &feature_level_info, sizeof(feature_level_info)));

    safe_release(temp_device);
    return feature_level_info.MaxSupportedFeatureLevel;
}

void DX12Context::create_command_queue()
{
    D3D12_COMMAND_QUEUE_DESC cmd_queue_desc{};
    cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HR(m_device->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(&m_command_queue)));

    HR(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator)));

    HR(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator,
         nullptr, IID_PPV_ARGS(&m_command_list)));
    
    HR(m_command_list->Close());
}

void DX12Context::create_swapchain(uint cx, uint cy)
{
    safe_release(m_swapchain);

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.BufferCount = frame_buffer_count;
    desc.Width = cx;
    desc.Height = cy;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.SampleDesc.Count = 1;

    IDXGISwapChain1* swapchain{};
    HR(m_factory->CreateSwapChainForHwnd(
        m_command_queue, m_hwnd, &desc, 
        nullptr, nullptr,
        &swapchain
    ));

    HR(swapchain->QueryInterface(IID_PPV_ARGS(&m_swapchain)));

    safe_release(swapchain);

    m_backbuffer_index = m_swapchain->GetCurrentBackBufferIndex();

    m_viewport.Width = cx;
    m_viewport.Height = cy;
    m_viewport.MaxDepth = 1.f;
    m_viewport.MinDepth = 0.f;
    m_viewport.TopLeftX = 0.f;
    m_viewport.TopLeftY = 0.f;

    m_sissor.right = cx;
    m_sissor.bottom = cy;
}

void DX12Context::create_descriptor_heaps() 
{
    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
    rtv_heap_desc.NumDescriptors = frame_buffer_count;
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HR(m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&m_rtv_heap)));

    m_rtv_desc_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_DESCRIPTOR_HEAP_DESC depth_stencil_heap_desc{};
    depth_stencil_heap_desc.NumDescriptors = 1;
    depth_stencil_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    //depth_stencil_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    depth_stencil_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HR(m_device->CreateDescriptorHeap(&depth_stencil_heap_desc, IID_PPV_ARGS(&m_dsv_heap)));

    m_dsv_desc_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void DX12Context::create_render_targets() 
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtv_heap->GetCPUDescriptorHandleForHeapStart());


    for(uint i = 0; i < frame_buffer_count; i++)
    {
        HR(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_render_targets[i])));
        m_device->CreateRenderTargetView(m_render_targets[i], nullptr, rtv_handle);
        rtv_handle.Offset(1, m_rtv_desc_size);
    }
}

void DX12Context::create_depth_stencil(uint cx, uint cy) 
{
    D3D12_DEPTH_STENCIL_VIEW_DESC depth_stencil_desc{};
    depth_stencil_desc.Format = DXGI_FORMAT_D32_FLOAT;
    depth_stencil_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depth_stencil_desc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CLEAR_VALUE depth_clear_value{};
    depth_clear_value.Format = DXGI_FORMAT_D32_FLOAT;
    depth_clear_value.DepthStencil.Depth = 1.0f;
    depth_clear_value.DepthStencil.Stencil = 0.f;

    // D3D12_RESOURCE_DESC depth_desc{};
    // depth_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    // depth_desc.Alignment = 0;
    // depth_desc.Width = cx;
    // depth_desc.Height = cy;
    // depth_desc.DepthOrArraySize = 1;
    // depth_desc.MipLevels = 1;
    // depth_desc.Format = DXGI_FORMAT_R32_TYPELESS; //use stencil view : D32_FLLOAT shader_view : R32_FLOAT
    // depth_desc.SampleDesc.Count = 1;
    // depth_desc.SampleDesc.Quality = 0;
    // depth_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    // depth_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    CD3DX12_RESOURCE_DESC depth_desc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		cx,
		cy,
		1,
		1,
		DXGI_FORMAT_R32_TYPELESS,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    HR(m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depth_desc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depth_clear_value,
        IID_PPV_ARGS(&m_depth_stencil)
    ));

    m_depth_stencil->SetName(L"DXContext::m_depth_stencil");

    CD3DX12_CPU_DESCRIPTOR_HANDLE dsv_handle(m_dsv_heap->GetCPUDescriptorHandleForHeapStart());

    m_device->CreateDepthStencilView(m_depth_stencil, &depth_stencil_desc, dsv_handle);
    
}

void DX12Context::create_fence() 
{
    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));

    m_fence_value = 0;

    m_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

uint64_t DX12Context::eval_fence()
{
    m_fence_value++; //cpu 1
    m_command_queue->Signal(m_fence, m_fence_value); //0 -> 1
    return m_fence_value;
}

void DX12Context::wait_for_gpu_async() 
{
    const uint64_t expected_fence_value = m_fence_value;

    if(m_fence->GetCompletedValue() < expected_fence_value){
        m_fence->SetEventOnCompletion(expected_fence_value, m_fence_event);
        WaitForSingleObject(m_fence_event, INFINITE);
    }
}

void DX12Context::begine_frame() 
{
    m_command_allocator->Reset();
    m_command_list->Reset(m_command_allocator, nullptr);

    //this guy is the heap [0xFF000wad][0xFF000wad + heap increasesize * index][
    //0xFF000wad + heap increasesize * index]
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtv_heap->GetCPUDescriptorHandleForHeapStart(),
        m_backbuffer_index, m_rtv_desc_size);
    //make this guy change state
    m_command_list->ResourceBarrier(
        1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            m_render_targets[m_backbuffer_index],
            D3D12_RESOURCE_STATE_PRESENT, 
            D3D12_RESOURCE_STATE_RENDER_TARGET
        ));
    //so u are focing to your pre engine to 12 porting
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsv_handle(m_dsv_heap->GetCPUDescriptorHandleForHeapStart());

    m_command_list->RSSetViewports(1, &m_viewport);
    m_command_list->RSSetScissorRects(1, &m_sissor);

    const float clear_color[] = {1.f, 0.5f, 0.2f, 1.f};
    m_command_list->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);
    m_command_list->ClearDepthStencilView(dsv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

    m_command_list->OMSetRenderTargets(1, &rtv_handle, FALSE, &dsv_handle);


}

void DX12Context::end_frame() 
{
    m_command_list->ResourceBarrier(
        1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            m_render_targets[m_backbuffer_index],
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
        ));
    m_command_list->Close();

    ID3D12CommandList* cmdlists[] = {m_command_list};
    m_command_queue->ExecuteCommandLists(_countof(cmdlists), cmdlists);

    m_swapchain->Present(1, 0);

    m_backbuffer_index = m_swapchain->GetCurrentBackBufferIndex();

    //query to signal to gpu current fence value
    eval_fence();

    wait_for_gpu_async();
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Context::get_rtv_handle() const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += m_backbuffer_index * m_rtv_desc_size;
    return handle;
}
