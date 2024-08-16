#pragma once
#include "config.h"

struct model_create_info
{
    void* p_data;
    uint cb_size;
    uint stride;
};


struct DX12Model
{
    explicit DX12Model(const model_create_info& info);

    ID3D12Resource* m_buffer{};
    D3D12_VERTEX_BUFFER_VIEW m_buffer_view{};
};


