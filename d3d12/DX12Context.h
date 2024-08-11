#pragma once

#include "config.h"
#define frame_buffer_count 2

struct DX12Context;
struct Context
{
    Context(uint cx, uint cy, void* hwnd)
        : m_hwnd((HWND)hwnd) {}
    HWND m_hwnd;
};


struct DX12Context
{
    DX12Context() = default;
    ~DX12Context();
    void initialize(uint cx, uint cy, HWND hwnd);
    void release();

    IDXGIAdapter4* get_adapter();
    D3D_FEATURE_LEVEL get_max_feature_level(IDXGIAdapter4* fixed_adapter);
    ID3D12Device*               m_device{};
    IDXGIFactory7*              m_factory{};
    ID3D12CommandQueue*         m_command_queue{};
    ID3D12CommandAllocator*     m_command_allocator{};
    ID3D12GraphicsCommandList*  m_command_list{};

    ID3D12DescriptorHeap* m_rtv_heap{};
    ID3D12DescriptorHeap* m_dsv_heap{};
    uint m_rtv_desc_size{0};
    uint m_dsv_desc_size{0};

    IDXGISwapChain3* m_swapchain{};
    ID3D12Resource*  m_render_targets[frame_buffer_count]{};
    ID3D12Resource*  m_depth_stencil{};
    uint32_t m_backbuffer_index{0};


    ID3D12Fence* m_fence{};
    uint64_t m_fence_value{0};
    HANDLE m_fence_event{};
    HWND m_hwnd{};
    D3D12_VIEWPORT m_viewport{};
    D3D12_RECT m_sissor{};
protected:
    void create_command_queue();
    void create_swapchain(uint cx, uint cy);
    void create_descriptor_heaps();
    void create_render_targets();
    void create_depth_stencil(uint cx, uint cy);
    void create_fence();
    
public:
    uint64_t eval_fence();
    void wait_for_gpu_async();

    void begine_frame();
    void end_frame();

    ID3D12GraphicsCommandList* get_command_list() { return m_command_list; };
    ID3D12Device* get_device() const { return m_device; }
    D3D12_CPU_DESCRIPTOR_HANDLE get_rtv_handle() const;
};
