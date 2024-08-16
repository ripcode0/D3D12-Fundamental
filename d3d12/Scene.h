#pragma once

class Scene
{
public:
    Scene() = default;

    virtual void initialize() = 0;
    virtual void update_frame(float dt) = 0;
    virtual void render_frame() = 0;
    virtual void release() = 0;
};

class DX12Context;
class ID3D12GraphicsCommandList;
class ID3D12Device;

class DX12Scene : public Scene
{
public:
    DX12Scene(DX12Context* context);

    virtual void initialize() {};
    virtual void update_frame(float dt) {};
    virtual void render_frame() {};
    virtual void release() {};

    DX12Context* m_context;
    ID3D12Device* m_device;
    ID3D12GraphicsCommandList* m_command_list;
};

