#include "texture.h"
#include "DX12.h"


TextureScene::TextureScene(DX12Context *context)
    : DX12Scene(context)
{
}

void TextureScene::initialize() 
{
    CD3DX12_DESCRIPTOR_RANGE range[1] = {};
    //range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0); //b0 : const buffer reg b1
    //range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); //b0 : const buffer reg
    range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); //t0 : shader resource
    //range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 1); //t0 : shader resource
    

    CD3DX12_ROOT_PARAMETER root_param[1] = {};
    root_param->InitAsDescriptorTable(_countof(range), range, D3D12_SHADER_VISIBILITY_ALL);

    D3D12_STATIC_SAMPLER_DESC sampler_desc = {};
	//sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

	sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler_desc.MipLODBias = 0.0f;
	sampler_desc.MaxAnisotropy = 16;
	sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler_desc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	sampler_desc.MinLOD = -FLT_MAX;
	sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
	sampler_desc.ShaderRegister = 0;
	sampler_desc.RegisterSpace = 0;
	sampler_desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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


    ID3DBlob* vs_blob{};
    ID3DBlob* ps_blob{};
    
    DX12::compile_from_file("texture_vs.hlsl", "vs_main", "vs_5_0",&vs_blob);
    DX12::compile_from_file("texture_ps.hlsl", "ps_main", "ps_5_0",&ps_blob);

    D3D12_INPUT_ELEMENT_DESC input_layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
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
        -0.5f, -0.5f, 0.f,  0.f, 1.f,
        -0.5f,  0.5f, 0.f,  0.f, 0.f,
         0.5f,  0.5f, 0.f,  1.f, 0.f,
         0.5f, -0.5f, 0.f,  1.f, 1.f
    };

    uint indices[] = {
        0,1,2, 0,2,3
    };

    DX12::create_resource_gpu_memory(vertices, sizeof(vertices), &m_vertex_buffer);
    DX12::create_resource_gpu_memory(indices, sizeof(indices), &m_index_buffer);

    m_vertex_view.BufferLocation = m_vertex_buffer->GetGPUVirtualAddress();
    m_vertex_view.SizeInBytes = sizeof(vertices);
    m_vertex_view.StrideInBytes = sizeof(float) * 5;

    m_index_buffer_view.BufferLocation = m_index_buffer->GetGPUVirtualAddress();
    m_index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
    m_index_buffer_view.SizeInBytes = sizeof(indices);

    create_texture();
    
}

void TextureScene::update_frame(float dt) 
{

}

void TextureScene::render_frame() 
{
    m_command_list->SetPipelineState(m_pipeline);
    m_command_list->SetGraphicsRootSignature(m_root_signature);

    m_command_list->SetDescriptorHeaps(1, &m_texture_heap);

    D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = m_texture_heap->GetGPUDescriptorHandleForHeapStart();

    m_command_list->SetGraphicsRootDescriptorTable(0, gpu_handle);

    m_command_list->IASetIndexBuffer(&m_index_buffer_view);
    m_command_list->IASetVertexBuffers(0, 1, &m_vertex_view);
    m_command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    m_command_list->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void TextureScene::release() 
{

}

void TextureScene::create_texture() 
{
    //DX12::load_resouce_gpu_texture("../data/checker.png", &m_texture);
    DX12::load_resouce_gpu_texture("../data/style.jpg", &m_texture);

    D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
    heap_desc.NodeMask = 0;
    heap_desc.NumDescriptors =1;
    heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    HR(m_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_texture_heap)));

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;

    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle = m_texture_heap->GetCPUDescriptorHandleForHeapStart();

    m_device->CreateShaderResourceView(m_texture, &srv_desc, srv_handle);
}
