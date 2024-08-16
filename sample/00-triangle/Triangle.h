#pragma once

#include "DX12Context.h"
#include "Scene.h"

struct Matrices
{
    float scale[16];
    float rot[16];
    float trans[16];
};

struct Colors
{
    float color[4];
};


class TriangleScene : public DX12Scene
{
public:
    TriangleScene(DX12Context* context);

    void initialize() override;

    void update_frame(float dt);

    void render_frame();

    void release() override;

    ID3D12RootSignature*    m_root_signature{};
    ID3D12DescriptorHeap*   m_srv_heap;
    
    ID3D12PipelineState*    m_pipeline;

    ID3D12Resource*             m_vbo{};
    D3D12_VERTEX_BUFFER_VIEW    m_vbo_view{};

protected:
    void create_const_buffers();
    ID3D12DescriptorHeap*   m_cbo_heap{};

    //const buffer 0 b0
    ID3D12Resource*         m_cbo{};

    //const buffer 1 b1
    ID3D12Resource*         m_cbo1{};
    
    //Datas
    Matrices matrices{};
    Colors colors{};
};
