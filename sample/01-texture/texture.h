#pragma once 

#include "DX12Context.h"
#include "Scene.h"
#include "config.h"

class TextureScene : public DX12Scene
{
public:
    TextureScene(DX12Context* context);

    void initialize() override;
    void update_frame(float dt) override;
    void render_frame() override;
    void release() override;

    ID3D12Resource* m_vertex_buffer;
    ID3D12Resource* m_index_buffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertex_view;
    D3D12_INDEX_BUFFER_VIEW m_index_buffer_view;

    ID3D12RootSignature* m_root_signature;
    ID3D12PipelineState* m_pipeline;

protected:
    void create_texture();

    ID3D12Resource* m_texture{};
    ID3D12DescriptorHeap* m_texture_heap{};
    

};