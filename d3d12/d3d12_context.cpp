// #include "d3d12_context.h"

// d3d12_context::d3d12_context(int cx, int cy, HWND hwnd) 
//     : m_hwnd(hwnd)
// {
//     m_viewport = {0.f, 0.f, (float)cx, (float)cy, 0.f, 1.f};
//     m_scissor = {0, 0, cx, cy};

//     create_device();

//     DX12::initialize(m_device.Get());
    
//     create_command_queue();

//     create_swapchain(cx, cy);

//     create_descriptor_heaps();
    
//     create_pipelines();

//     create_fence();

    

//     //DX12::create_resource_gpu_memory()//

    
// }

// void d3d12_context::create_device() 
// {
//     uint factory_flags = 0;

// #ifdef _DEBUG
//     // com_ptr<ID3D12Debug> debug;
//     ComPtr<ID3D12Debug> debug;
    
//     if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
//     {
//         debug->EnableDebugLayer();

//         factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
//     }
// #endif

//     ComPtr<IDXGIFactory4> factory;
//     HR(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)));

//     HR(factory.As(&m_factory));

//     ComPtr<IDXGIAdapter> warp_adapter;
//     HR(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter)));

//     HR(D3D12CreateDevice(
//         warp_adapter.Get(),
//         D3D_FEATURE_LEVEL_11_0,
//         IID_PPV_ARGS(&m_device)
//     ));
// }

// void d3d12_context::create_command_queue() 
// {
//     D3D12_COMMAND_QUEUE_DESC desc{};
//     desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//     desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

//     HR(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_command_queue)));

// }

// void d3d12_context::create_swapchain(int cx, int cy) 
// {
//     DXGI_SWAP_CHAIN_DESC1 desc{};
//     desc.BufferCount = frame_buffer_count;
//     desc.Width = cx;
//     desc.Height = cy;
//     desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
//     desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//     desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
//     desc.SampleDesc.Count = 1;

//     ComPtr<IDXGISwapChain1> swapchain;
//     HR(m_factory->CreateSwapChainForHwnd(
//         m_command_queue.Get(),
//         m_hwnd,
//         &desc,
//         nullptr,
//         nullptr,
//         &swapchain
//     ));

//     HR(m_factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));
//     HR(swapchain.As(&m_swapchain));

//     m_frame_index = m_swapchain->GetCurrentBackBufferIndex();
// }

// void d3d12_context::create_descriptor_heaps() 
// {
//     D3D12_DESCRIPTOR_HEAP_DESC rtv_desc{};
//     rtv_desc.NumDescriptors = frame_buffer_count;
//     rtv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
//     rtv_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    
//     HR(m_device->CreateDescriptorHeap(&rtv_desc, IID_PPV_ARGS(&m_rtv_heap)));

//     m_rtv_descriptor_size = m_device->GetDescriptorHandleIncrementSize(
//         D3D12_DESCRIPTOR_HEAP_TYPE_RTV
//     );

//     D3D12_DESCRIPTOR_HEAP_DESC cbv_desc{};
//     cbv_desc.NumDescriptors = 1;
//     cbv_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//     cbv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//     HR(m_device->CreateDescriptorHeap(&cbv_desc, IID_PPV_ARGS(&m_cbv_heap)));

//     CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtv_heap->GetCPUDescriptorHandleForHeapStart());

//     for(uint i = 0; i < frame_buffer_count; i++){
//         HR(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_render_targets[i])));
//         m_device->CreateRenderTargetView(m_render_targets[i].Get(), nullptr, rtv_handle);
//         rtv_handle.Offset(1, m_rtv_descriptor_size);
//     }
//     HR(m_device->CreateCommandAllocator(
//         D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator)));
//     m_command_queue->SetName(L"alloc");
// }

// void d3d12_context::create_pipelines() 
// {
//     D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data{};

//     feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

//     HRESULT supported = m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE,
//      &feature_data, sizeof(feature_data));
    
//     if(FAILED(supported)){
//         feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
//     }

//     CD3DX12_DESCRIPTOR_RANGE1 ranges[1]{};
//     CD3DX12_ROOT_PARAMETER1 root_params[1]{};

//     ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 
//         D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
//     root_params[0].InitAsDescriptorTable(
//         1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX
//     );
    
//     //allow input layout vertex shader only
//     D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags =
//      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
//      D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
//      D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
//      D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
//      D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

//      CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc{};

//      root_signature_desc.Init_1_1(
//         _countof(root_params),root_params, 0, nullptr,
//         root_signature_flags
//      );

//      ComPtr<ID3DBlob> signature{};
//      ComPtr<ID3DBlob> error{};

//      HR(D3DX12SerializeVersionedRootSignature(
//         &root_signature_desc, feature_data.HighestVersion, &signature, &error
//      ));

//      HR(m_device->CreateRootSignature(0, signature->GetBufferPointer(),
//         signature->GetBufferSize(), IID_PPV_ARGS(&m_root_signature)
//      ));

//     ComPtr<ID3DBlob> vert;
//     ComPtr<ID3DBlob> pixel;

//     //shaderbuffer

//     UINT flag = 0;
// #if defined(_DEBUG)
//     flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
// #endif

//     HR(D3DCompileFromFile(L"vs.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", flag, 0, &vert, nullptr));
//     HR(D3DCompileFromFile(L"ps.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", flag, 0, &pixel, nullptr));
    
//     D3D12_INPUT_ELEMENT_DESC input_layout[] = {
//         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//         {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
//     };

//     D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_desc{};
//     pipeline_desc.InputLayout = {input_layout, _countof(input_layout)};
//     pipeline_desc.pRootSignature = m_root_signature.Get();
//     pipeline_desc.VS = CD3DX12_SHADER_BYTECODE(vert.Get());
//     pipeline_desc.PS = CD3DX12_SHADER_BYTECODE(pixel.Get());
//     pipeline_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
//     pipeline_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); //disable blendstate by defualt
//     pipeline_desc.DepthStencilState.DepthEnable = false;
//     pipeline_desc.DepthStencilState.StencilEnable = false;
//     pipeline_desc.SampleMask = UINT_MAX;
//     pipeline_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//     pipeline_desc.NumRenderTargets = 1;
//     pipeline_desc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
//     pipeline_desc.SampleDesc.Count = 1;

//     HR(m_device->CreateGraphicsPipelineState(&pipeline_desc, IID_PPV_ARGS(&m_pipeline)));

//     HR(m_device->CreateCommandList(0,
//         D3D12_COMMAND_LIST_TYPE_DIRECT,
//         m_command_allocator.Get(), m_pipeline.Get(),
//         IID_PPV_ARGS(&m_commandlist)
//     ));

//     HR(m_commandlist->Close());

//     VertexPC vertices[] = {
//         {{0.f, 0.5f, 0.f}, {1.f, 0.f, 0.f, 1.f}},
//         {{0.5f, -0.5f, 0.f}, {1.f, 0.f, 0.f, 1.f}},
//         {{-0.5f, -0.5f, 0.f}, {1.f, 0.f, 0.f, 1.f}}
//     };

//     //Create Vertex Buffer
//     // HR(m_device->CreateCommittedResource(
//     //     &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
//     //     D3D12_HEAP_FLAG_NONE,
//     //     &CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
//     //     D3D12_RESOURCE_STATE_GENERIC_READ,
//     //     nullptr,
//     //     IID_PPV_ARGS(&m_vertex_buffer)
//     // ));

//     // //we do not intend for Range
//     // unsigned char* data_begin{};
//     // D3D12_RANGE read_range{};
//     // read_range.Begin = 0;
//     // read_range.End = 0;


//     // m_vertex_buffer->Map(0, &read_range, (void**)&data_begin);
//     // memcpy(data_begin, vertices, sizeof(vertices));
//     // m_vertex_buffer->Unmap(0, nullptr);

//     DX12::create_resource_gpu_memory(vertices, sizeof(VertexPC) * ARRAYSIZE(vertices),
//         m_vertex_buffer.GetAddressOf()
//     );


//     m_vertex_buffer_view.BufferLocation = m_vertex_buffer->GetGPUVirtualAddress();
//     m_vertex_buffer_view.StrideInBytes = sizeof(VertexPC);
//     m_vertex_buffer_view.SizeInBytes = sizeof(vertices);

// }

// void d3d12_context::create_fence() 
// {
//     HR(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, 
//         IID_PPV_ARGS(&m_fence)));

//     m_fence_value = 1;

//     m_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);

//     wait_for_async();
// }

// void d3d12_context::wait_for_async() 
// {
//     const UINT64 fence = m_fence_value;
//     HR(m_command_queue->Signal(m_fence.Get(), fence));

//     m_fence_value++;

//     if(m_fence->GetCompletedValue() < fence){
//         HR(m_fence->SetEventOnCompletion(fence, m_fence_event));
//         WaitForSingleObject(m_fence_event, INFINITE);
//     }

//     m_frame_index = m_swapchain->GetCurrentBackBufferIndex();
// }

// void d3d12_context::render() 
// {
//     HR(m_command_allocator->Reset());

//     m_commandlist->Reset(m_command_allocator.Get(), m_pipeline.Get());

//     m_commandlist->SetGraphicsRootSignature(m_root_signature.Get());

//     ID3D12DescriptorHeap* pp_heaps[] = { m_cbv_heap.Get()};
//     m_commandlist->SetDescriptorHeaps(_countof(pp_heaps), pp_heaps);

//     m_commandlist->RSSetViewports(1, &m_viewport);

//     m_commandlist->RSSetScissorRects(1, &m_scissor);

//     m_commandlist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//         m_render_targets[m_frame_index].Get(),
//         D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
//     ));

//     CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(
//         m_rtv_heap->GetCPUDescriptorHandleForHeapStart(),
//         m_frame_index, m_rtv_descriptor_size
//     );

//     m_commandlist->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);

//     const float clear_color[] = {0.1,0.2,0.3,1.f};
//     m_commandlist->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);
//     m_commandlist->IASetVertexBuffers(0, 1, &m_vertex_buffer_view);
//     m_commandlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//     m_commandlist->DrawInstanced(3, 1, 0, 0);

//     m_commandlist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//         m_render_targets[m_frame_index].Get(),
//         D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
//     ));

//     m_commandlist->Close();

//     ID3D12CommandList* cmdlist[] = {m_commandlist.Get()};
//     m_command_queue->ExecuteCommandLists(_countof(cmdlist), cmdlist);

//     m_swapchain->Present(1, 0);

//     Sleep(300);

//     wait_for_async();
//     printf("current frame buffer index %d\n", m_frame_index);

// }
