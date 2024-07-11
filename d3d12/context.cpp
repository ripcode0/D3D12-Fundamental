#include "context.h"

d3d12_context::d3d12_context(int cx, int cy, HWND hwnd) 
    : m_hwnd(hwnd)
{
    create_device();
    
    create_command_queue();

}

void d3d12_context::create_device() 
{
    uint factory_flags = 0;

#ifdef _DEBUG
    // com_ptr<ID3D12Debug> debug;
    ComPtr<ID3D12Debug> debug;
    
    if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
    {
        debug->EnableDebugLayer();

        factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    HR(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)));

    ComPtr<IDXGIAdapter> warp_adapter;
    HR(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter)));

    HR(D3D12CreateDevice(
        warp_adapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device)
    ));
}

void d3d12_context::create_command_queue() 
{
    D3D12_COMMAND_QUEUE_DESC desc{};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HR(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_command_queue)));


}
