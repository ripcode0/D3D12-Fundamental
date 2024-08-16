#include "Triangle.h"
#include "DX12.h"

TriangleScene::TriangleScene(DX12Context *context)
    : DX12Scene(context)
{

}

void TriangleScene::initialize()
{
    CD3DX12_DESCRIPTOR_RANGE range[2] = {};
    range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0); //b0 : const buffer reg b1
    //range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); //b0 : const buffer reg
    range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); //t0 : shader resource
    

    CD3DX12_ROOT_PARAMETER root_param[1] = {};
    root_param->InitAsDescriptorTable(_countof(range), range, D3D12_SHADER_VISIBILITY_ALL);

    //default sampler
    // D3D12_STATIC_SAMPLER_DESC sampler_desc{};
    // sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    // sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    // sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    // sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    // sampler_desc.MipLODBias = 0.0f;
    // sampler_desc.MaxAnisotropy = 16.f;
    CD3DX12_STATIC_SAMPLER_DESC sampler_desc{};
    sampler_desc.Init(D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    D3D12_ROOT_SIGNATURE_DESC root_desc{};
    root_desc.NumParameters = _countof(root_param);
    root_desc.pParameters = root_param;
    root_desc.NumStaticSamplers = 1;
    root_desc.pStaticSamplers = &sampler_desc;
    root_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* root_blob{};
    ID3DBlob* err_blob{};

    HR(D3D12SerializeRootSignature(&root_desc, D3D_ROOT_SIGNATURE_VERSION_1, 
        &root_blob, &err_blob));

    
    HR(m_device->CreateRootSignature(
        0, 
        root_blob->GetBufferPointer(),
        root_blob->GetBufferSize(),
        IID_PPV_ARGS(&m_root_signature)));

    safe_release(root_blob);
    safe_release(err_blob);


    D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc{};
    srv_heap_desc.NumDescriptors = 1;
    srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    m_device->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&m_srv_heap));

    
    // D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc{};
    // srv_heap_desc.NumDescriptors = 1;
    // srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    // srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    //m_device->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&m_cbv_heap));

    ID3DBlob* vs_blob{};
    ID3DBlob* ps_blob{};
    
    DX12::compile_from_file("vs.hlsl", "VSMain", "vs_5_0",&vs_blob);
    DX12::compile_from_file("ps.hlsl", "PSMain", "ps_5_0",&ps_blob);

    D3D12_INPUT_ELEMENT_DESC input_layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
    

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_desc{};
    pipeline_desc.InputLayout = { input_layout, _countof(input_layout)};
    pipeline_desc.VS = {vs_blob->GetBufferPointer(), vs_blob->GetBufferSize()};
    pipeline_desc.PS = {ps_blob->GetBufferPointer(), ps_blob->GetBufferSize()};
    pipeline_desc.NumRenderTargets = frame_buffer_count;
    pipeline_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipeline_desc.SampleDesc.Count = 1;
    pipeline_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipeline_desc.pRootSignature = m_root_signature;
    pipeline_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipeline_desc.SampleMask = UINT_MAX;
    pipeline_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pipeline_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pipeline_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    

    HR(m_device->CreateGraphicsPipelineState(&pipeline_desc, IID_PPV_ARGS(&m_pipeline)));

    float vertices[] = {
        -1.f, -1.f, 0.f, 1.f, 0.f, 0.f, 1.f,
         0.f,  1.f, 0.f, 1.f, 0.f, 0.f, 1.f,
         1.f, -1.f, 0.f, 1.f, 0.f, 0.f, 1.f,
    }; 

    DX12::create_resource_gpu_memory(vertices, sizeof(float) * _countof(vertices), &m_vbo);
    m_vbo_view.BufferLocation = m_vbo->GetGPUVirtualAddress();
    m_vbo_view.SizeInBytes = sizeof(float) * _countof(vertices);
    m_vbo_view.StrideInBytes = sizeof(float) * 7;

    create_const_buffers();

}

void TriangleScene::update_frame(float dt) 
{
    static uint frame = 0;
    float f = std::sinf(frame * 0.03f);

    matrices = {
        {   0.5f, 0.0f, 0.0f, 0.f,
            0.0f, 0.5f, 0.0f, 0.f,
            0.0f, 0.0f, 0.5f, 0.f,
            0.0f, 0.0f, 0.0f, 1.f},

        {   cosf(f), -sinf(f),  0.0f, 0.f,
            sinf(f),  cosf(f),  0.0f, 0.f,
               0.0f,     0.0f,  1.0f, 0.f,
               0.0f,     0.0f,  0.0f, 1.f},

        {   1.0f, 0.0f, 0.0f, 0.f,
            0.0f, 1.0f, 0.0f, 0.f,
            0.0f, 0.0f, 1.0f, 0.f,
            sinf(f), 0.0f, 0.0f, 1.f}
    };

    void* mapped_data{};
    //CD3DX12_RANGE range(0,0);
    D3D12_RANGE range{};
    m_cbo->Map(0, &range, &mapped_data);
    memcpy(mapped_data, &matrices, sizeof(matrices));
    m_cbo->Unmap(0, nullptr);

    colors = { cosf(f), sinf(f), sinf(f), 1.f};

    mapped_data ={};
    m_cbo1->Map(0, &range, &mapped_data);
    memcpy(mapped_data, &colors, sizeof(colors));
    m_cbo1->Unmap(0, nullptr);

    frame++;
}

void TriangleScene::render_frame() 
{
    m_command_list->SetPipelineState(m_pipeline);
    m_command_list->SetGraphicsRootSignature(m_root_signature);
    m_command_list->IASetVertexBuffers(0, 1, &m_vbo_view);
    m_command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //cbo
    ID3D12DescriptorHeap* heap_list[] = {m_cbo_heap};
    m_command_list->SetDescriptorHeaps(_countof(heap_list), heap_list);
    m_command_list->SetGraphicsRootDescriptorTable(0, m_cbo_heap->GetGPUDescriptorHandleForHeapStart());


    m_command_list->DrawInstanced(3, 1, 0, 0);
}

void TriangleScene::release() 
{
    safe_release(m_root_signature);
    safe_release(m_srv_heap);
    safe_release(m_pipeline);
    safe_release(m_vbo);
    safe_release(m_cbo);
    safe_release(m_cbo1);
    safe_release(m_cbo_heap);
}

void TriangleScene::create_const_buffers() 
{
    matrices = {
        {   
            0.5f, 0.0f, 0.0f, 0.f,
            0.0f, 0.5f, 0.0f, 0.f,
            0.0f, 0.0f, 0.5f, 0.f,
            0.0f, 0.0f, 0.0f, 1.f},

        {   1.0f,    0.0f,     0.0f, 0.f,
            0.0f,    1.0f,     0.0f, 0.f,
            0.0f,    0.0f,     1.0f, 0.f,
            0.0f,    0.0f,     0.0f, 1.f},

        {   1.0f, 0.0f, 0.0f, 0.f,
            0.0f, 1.0f, 0.0f, 0.f,
            0.0f, 0.0f, 1.0f, 0.f,
            0.0f, 0.0f, 0.0f, 1.f},
    };
    DX12::create_constant_buffer(
        &matrices, sizeof(Matrices), &m_cbo
    );

    colors = {
        1,1,0,1
    };
    DX12::create_constant_buffer(
        &colors, sizeof(colors), &m_cbo1
    );


    //we'll using 2 constatn buffers
    D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
    heap_desc.NumDescriptors = 2;
    heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    HR(m_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_cbo_heap)));

    //b0
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbo_view_desc{};
    cbo_view_desc.BufferLocation = m_cbo->GetGPUVirtualAddress();
    cbo_view_desc.SizeInBytes = DX12::align_256byte_size(sizeof(Matrices));

    D3D12_CPU_DESCRIPTOR_HANDLE cbo_handle = m_cbo_heap->GetCPUDescriptorHandleForHeapStart();

    m_device->CreateConstantBufferView(&cbo_view_desc, cbo_handle);

    //b1
    cbo_view_desc = {};
    cbo_view_desc.BufferLocation = m_cbo1->GetGPUVirtualAddress();
    cbo_view_desc.SizeInBytes = DX12::align_256byte_size(sizeof(Colors));

    //increase handle ptr for the next one
    cbo_handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    m_device->CreateConstantBufferView(&cbo_view_desc, cbo_handle);
}
