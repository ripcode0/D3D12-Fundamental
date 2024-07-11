#pragma once

#include "config.h"

#define frame_buffer_count 2


class d3d12_context
{
public:
    d3d12_context(int cx, int cy, HWND hwnd);

    void create_device();
    void create_command_queue();
    void create_swapchain(int cx, int cy);
    void create_descriptor_heaps();

    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_command_queue;

    ComPtr<IDXGISwapChain3> m_swapchain;
    
    ComPtr<IDXGIFactory4> m_factory;

    ComPtr<ID3D12DescriptorHeap> m_rtv_heap;
    ComPtr<ID3D12DescriptorHeap> m_cbv_heap;

    ComPtr<ID3D12Resource> m_render_targets[frame_buffer_count];

    HWND m_hwnd;
    uint m_frame_index;

};

