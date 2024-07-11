#pragma once

#include "config.h"

class d3d12_context
{
public:
    d3d12_context(int cx, int cy, HWND hwnd);
    void create_device();
    void create_command_queue();

    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_command_queue;

    ComPtr<IDXGISwapChain> m_swapchain;

    HWND m_hwnd;
};

