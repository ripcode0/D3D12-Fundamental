#include "d3d12_context.h"

d3d12_context::d3d12_context(int cx, int cy, HWND hwnd) 
    : m_hwnd(hwnd)
{
    //i just research 
    create_device();
    
    create_command_queue();

    create_swapchain(cx, cy);
    
    create_pipelines();
}

void d3d12_context::create_device() 
{
    uint factory_flags = 0;

#ifdef _DEBUG
    // com_ptr<ID3D12Debug> debug;
    ComPtr<ID3D12Debug> debug;
    
    if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
    {
        debug->EnableDebugLayer();

        factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    HR(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)));

    HR(factory.As(&m_factory));

    ComPtr<IDXGIAdapter> warp_adapter;
    HR(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter)));

    HR(D3D12CreateDevice(
        warp_adapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device)
    ));
}

void d3d12_context::create_command_queue() 
{
    D3D12_COMMAND_QUEUE_DESC desc{};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HR(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_command_queue)));

}

void d3d12_context::create_swapchain(int cx, int cy) 
{
    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.BufferCount = frame_buffer_count;
    desc.Width = cx;
    desc.Height = cy;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapchain;
    HR(m_factory->CreateSwapChainForHwnd(
        m_command_queue.Get(),
        m_hwnd,
        &desc,
        nullptr,
        nullptr,
        &swapchain
    ));

    HR(m_factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));
    HR(swapchain.As(&m_swapchain));

    m_frame_index = m_swapchain->GetCurrentBackBufferIndex();
}

void d3d12_context::create_descriptor_heaps() 
{
    D3D12_DESCRIPTOR_HEAP_DESC rtv_desc{};
    rtv_desc.NumDescriptors = frame_buffer_count;
    rtv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    
    HR(m_device->CreateDescriptorHeap(&rtv_desc, IID_PPV_ARGS(&m_rtv_heap)));

    m_rtv_descriptor_size = m_device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV
    );

    D3D12_DESCRIPTOR_HEAP_DESC cbv_desc{};
    cbv_desc.NumDescriptors = 1;
    cbv_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    HR(m_device->CreateDescriptorHeap(&cbv_desc, IID_PPV_ARGS(&m_cbv_heap)));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtv_heap->GetCPUDescriptorHandleForHeapStart());

    for(uint i = 0; i < frame_buffer_count; i++){
        HR(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_render_targets[i])));
        m_device->CreateRenderTargetView(m_render_targets[i].Get(), nullptr, rtv_handle);
        rtv_handle.Offset(1, m_rtv_descriptor_size);
    }
    HR(m_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator)));
}

void d3d12_context::create_pipelines() 
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data{};

    feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    HRESULT supported = m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE,
     &feature_data, sizeof(feature_data));
    
    if(FAILED(supported)){
        feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    CD3DX12_DESCRIPTOR_RANGE1 ranges[1]{};
    CD3DX12_ROOT_PARAMETER1 root_params[1]{};

    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 
        D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    root_params[0].InitAsDescriptorTable(
        1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX
    );
    
    //allow input layout vertex shader only
    D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags =
     D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
     D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
     D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
     D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
     D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

     CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc{};

     root_signature_desc.Init_1_1(
        _countof(root_params),root_params, 0, nullptr,
        root_signature_flags
     );

     ComPtr<ID3DBlob> signature{};
     ComPtr<ID3DBlob> error{};

     HR(D3DX12SerializeVersionedRootSignature(
        &root_signature_desc, feature_data.HighestVersion, &signature, &error
     ));

     HR(m_device->CreateRootSignature(0, signature->GetBufferPointer(),
        signature->GetBufferSize(), IID_PPV_ARGS(&m_root_signature)
     ));


}
