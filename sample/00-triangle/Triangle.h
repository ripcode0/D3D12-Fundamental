#pragma once

#include "DX12Context.h"
#include "Scene.h"

class TriangleScene : public DX12Scene
{
public:
    TriangleScene(DX12Context* context);

    void initialize() override;

    void update_frame(float dt);

    void render_frame();

    void release() override;

    // DX12Context* m_context;
    // ID3D12Device* m_device;
    // ID3D12GraphicsCommandList* m_command_list;

    ID3D12RootSignature* m_root_signature{};
    ID3D12DescriptorHeap* m_srv_heap;
    ID3D12DescriptorHeap* m_cbv_heap;

    ID3D12PipelineState* m_pipeline;

    ID3D12Resource* m_vbo{};
    D3D12_VERTEX_BUFFER_VIEW m_vbo_view{};
};
