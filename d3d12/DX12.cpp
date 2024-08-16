#include "DX12.h"
#include "config.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ID3D12Device* DX12::device{};
//only for resources
ID3D12CommandQueue*     DX12::cmdqueue{};
ID3D12CommandAllocator* DX12::cmdallocator{};
ID3D12GraphicsCommandList*      DX12::cmdlist{};

UINT64        DX12::sub_fence_value = 0;
HANDLE        DX12::sub_fence_event = nullptr;
ID3D12Fence*  DX12::sub_fence       = nullptr;

void DX12::initialize(ID3D12Device *p_device)
{
    device = p_device;

    //create the command queue

    D3D12_COMMAND_QUEUE_DESC desc{};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HR(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cmdqueue)));

    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdallocator));

    device->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT, cmdallocator, nullptr, IID_PPV_ARGS(&cmdlist));

    HR(cmdlist->Close());

    //create the sub fence

    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&sub_fence));
    set_name(sub_fence, "DX::sub_fence");

    sub_fence_value = 0;

    sub_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);

}

void DX12::release() 
{
    safe_release(cmdlist);
    safe_release(cmdallocator);
    safe_release(cmdqueue);

    CloseHandle(sub_fence_event);
    sub_fence_event = nullptr;

    safe_release(sub_fence);
}

void DX12::fence() 
{
    sub_fence_value++;
    HR(cmdqueue->Signal(sub_fence, sub_fence_value));
}

void DX12::wait_for_gpu_fence() 
{
    const UINT64 expected_fence_value = sub_fence_value;

    if(sub_fence->GetCompletedValue() < expected_fence_value){
      sub_fence->SetEventOnCompletion(expected_fence_value, sub_fence_event);
      ::WaitForSingleObject(sub_fence_event, INFINITE);
    }
}

HRESULT DX12::create_resource_system_memory(
    D3D12_RESOURCE_STATES dest_state, void *p_data,
    uint cb_size, ID3D12Resource **pp_resources)
{
    if (*pp_resources)
        return E_INVALIDARG;

    HRESULT res = S_OK;

    // create resource on system memory
    CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resource_desc(CD3DX12_RESOURCE_DESC::Buffer(cb_size));

    res = device->CreateCommittedResource(
        &heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(pp_resources));

    ID3D12Resource *cpu_resource = *pp_resources;
    CD3DX12_RANGE range{0, 0};
    void *p_begin_data = nullptr;
    res = cpu_resource->Map(0, &range, &p_begin_data);
    memcpy(p_begin_data, p_data, cb_size);
    cpu_resource->Unmap(0, nullptr);

    return res;
}

HRESULT DX12::create_resource_gpu_memory(
    void *src_data,
    uint srcs_size,
    ID3D12Resource **pp_resource)
{
    ID3D12Resource* buffer{};

    CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_DEFAULT);

    CD3DX12_RESOURCE_DESC desc(CD3DX12_RESOURCE_DESC::Buffer(srcs_size));
    
    HR(device->CreateCommittedResource(
      &heap_props,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS(&buffer)
    ));

    heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    ID3D12Resource* upload_buffer{};

    HR(device->CreateCommittedResource(
      &heap_props,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&upload_buffer)
    ));
    
    uint8_t* p_data{};
    CD3DX12_RANGE range(0,0);
    HR(upload_buffer->Map(0, &range, (void**)&p_data));
    memcpy(p_data, src_data, srcs_size);
    upload_buffer->Unmap(0, nullptr);

    HR(cmdallocator->Reset());

    HR(cmdlist->Reset(cmdallocator, nullptr));

    cmdlist->CopyBufferRegion(buffer, 0, upload_buffer, 0, srcs_size);    
    cmdlist->ResourceBarrier(1, 
      &CD3DX12_RESOURCE_BARRIER::Transition(buffer,
      D3D12_RESOURCE_STATE_COPY_DEST,   
      D3D12_RESOURCE_STATE_GENERIC_READ));  
  
    cmdlist->Close();

    ID3D12CommandList* cmd_lists[] = {cmdlist};
    cmdqueue->ExecuteCommandLists(_countof(cmd_lists), cmd_lists);

    fence();

    wait_for_gpu_fence();

    *pp_resource = buffer;

    safe_release(upload_buffer);
    //dx12 making thread safe
    return S_OK;
}

HRESULT DX12::create_constant_buffer(
    void* p_data, 
    uint cb_size,   
    ID3D12Resource **pp_resource)
{
    uint fixed_buffer_size = (cb_size + 255) & ~255;

    ID3D12Resource* constant_buffer{};
    CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(fixed_buffer_size);

    HR(device->CreateCommittedResource(
        &heap_props,
        D3D12_HEAP_FLAG_NONE,
        &buffer_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&constant_buffer)
    ));

    void* p_mapped_data{};
    CD3DX12_RANGE read_range(0,0);
    constant_buffer->Map(0, &read_range, &p_mapped_data);
    memcpy(p_mapped_data, p_data, cb_size);
    constant_buffer->Unmap(0, nullptr);


    *pp_resource = constant_buffer;

    return S_OK;
}

#include <cwchar>
#include <cstring>

void DX12::compile_from_file(
    const char *file,
    const char *entry,
    const char* version,
    ID3DBlob **pp_blob)
{
    const size_t len = std::strlen(file) + 1;

    wchar_t wide_file[128];

    std::mbstowcs(wide_file, file, len);

    UINT flag{};
#if defined(_DEBUG)
    flag |= D3DCOMPILE_DEBUG;
#endif
    ID3DBlob* err_blob{};
    HRESULT res = ::D3DCompileFromFile(
        wide_file,
        nullptr,
        nullptr, 
        entry,
        version,
        flag,
        0,
        pp_blob,
        &err_blob
    );

    if(FAILED(res)){
        if(err_blob){
            
            MessageBoxA(NULL, (char*)err_blob->GetBufferPointer(), "Compile Error", MB_OK);
            safe_release(err_blob);
        }
    }
    safe_release(err_blob);
}

HRESULT DX12::load_resouce_gpu_texture(const char *file, ID3D12Resource **pp_resource)
{
    int width = 0;
    int height = 0;
    int channels = 0;

    if(GetFileAttributesA(file) == INVALID_FILE_ATTRIBUTES){
        return ERROR_FILE_NOT_FOUND;
    }

    uint8_t* data = stbi_load(file, &width, &height, &channels, STBI_rgb_alpha);
    if(!data) return E_OUTOFMEMORY;

    ID3D12Resource* texture_resources{};
    ID3D12Resource* upload_resources{};

    D3D12_RESOURCE_DESC texture_desc = {};
	texture_desc.MipLevels = 1;
	texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// ex) DXGI_FORMAT_R8G8B8A8_UNORM, etc...
	texture_desc.Width = width;
	texture_desc.Height = height;
	texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	texture_desc.DepthOrArraySize = 1;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texture_desc,
		D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texture_resources))))
	{
		__debugbreak();
	}


	D3D12_RESOURCE_DESC Desc = texture_resources->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint;
	UINT	Rows = 0;
	UINT64	RowSize = 0;
	UINT64	TotalBytes = 0;

	device->GetCopyableFootprints(&Desc, 0, 1, 0, &Footprint, &Rows, &RowSize, &TotalBytes);

	BYTE*	pMappedPtr = nullptr;
	CD3DX12_RANGE writeRange(0, 0);

	UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture_resources, 0, 1);

	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&upload_resources))))
	{
		__debugbreak();
	}
	HRESULT hr = upload_resources->Map(0, &writeRange,
		reinterpret_cast<void**>(&pMappedPtr));
	if (FAILED(hr))
		__debugbreak();
	const BYTE* pSrc = data;
	BYTE* pDest = pMappedPtr;
	for (UINT y = 0; y < height; y++)
	{
		memcpy(pDest, pSrc, width * 4);
		pSrc += (width * 4);
		pDest += Footprint.Footprint.RowPitch;			
	}
	
	upload_resources->Unmap(0, nullptr);

	update_texture_for_write(texture_resources, upload_resources);

	safe_release(upload_resources)
		
	
	*pp_resource = texture_resources;

    return S_OK;
}

void DX12::update_texture_for_write(ID3D12Resource *dest_resource, ID3D12Resource *srcs_resource)
{
    const DWORD MAX_SUB_RESOURCE_NUM = 32;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint[MAX_SUB_RESOURCE_NUM] = {};
	UINT	Rows[MAX_SUB_RESOURCE_NUM] = {};
	UINT64	RowSize[MAX_SUB_RESOURCE_NUM] = {};
	UINT64	TotalBytes = 0;

	D3D12_RESOURCE_DESC Desc = dest_resource->GetDesc();
	if (Desc.MipLevels > (UINT)_countof(Footprint))
		__debugbreak();

	device->GetCopyableFootprints(&Desc, 0, Desc.MipLevels, 0, Footprint, Rows, RowSize, &TotalBytes);

	if (FAILED(cmdallocator->Reset()))
		__debugbreak();

	if (FAILED(cmdlist->Reset(cmdallocator, nullptr)))
		__debugbreak();

	cmdlist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dest_resource,
     D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
	for (DWORD i = 0; i < Desc.MipLevels; i++)
	{

		D3D12_TEXTURE_COPY_LOCATION	destLocation = {};
		destLocation.PlacedFootprint = Footprint[i];
		destLocation.pResource = dest_resource;
		destLocation.SubresourceIndex = i;
		destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION	srcLocation = {};
		srcLocation.PlacedFootprint = Footprint[i];
		srcLocation.pResource = srcs_resource;
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

		cmdlist->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);
	}
	cmdlist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dest_resource,
     D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));
	cmdlist->Close();

	ID3D12CommandList* ppCommandLists[] = { cmdlist };
	cmdqueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	fence();
	wait_for_gpu_fence();
}

// void DX12::load_resouce_gpu_texture(
//     const char *file, 
//     ID3D12Resource **pp_resource)
// {
//     int width = 0;
//     int height = 0;
//     int channels = 0;
//     uint8_t* data = stbi_load(file, &width, &height, &channels, STBI_rgb_alpha);

//     if(!data) return;

//     D3D12_RESOURCE_DESC texture_desc{};
//     texture_desc.MipLevels = 1;
//     texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//     texture_desc.Width = width;
//     texture_desc.Height = height;
//     texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
//     texture_desc.DepthOrArraySize = 1;
//     texture_desc.SampleDesc.Count = 1;
//     texture_desc.SampleDesc.Quality = 0;
//     texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

//     CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_UPLOAD);
//     CD3DX12_RESOURCE_DESC texture_resource_desc = CD3DX12_RESOURCE_DESC::Tex2D(
//         texture_desc.Format, width, height, 
//         texture_desc.DepthOrArraySize, texture_desc.MipLevels
//     );

//     ID3D12Resource* texture{};
//     HRESULT res = device->CreateCommittedResource(
//         &heap_props,
//         D3D12_HEAP_FLAG_NONE,
//         &texture_resource_desc,
//         D3D12_RESOURCE_STATE_COPY_DEST,
//         nullptr,
//         IID_PPV_ARGS(&texture)
//     );

//     if(FAILED(res)) {
//         stbi_image_free(data);
//         return;
//     }

//     ID3D12Resource* upload_heap_resource{};
//     const uint64_t upload_buffer_size = GetRequiredIntermediateSize(texture, 0, 1);
//     CD3DX12_HEAP_PROPERTIES upload_heap_props(D3D12_HEAP_TYPE_UPLOAD);
//     CD3DX12_RESOURCE_DESC upload_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(upload_buffer_size);

//     res = device->CreateCommittedResource(
//         &upload_heap_props,
//         D3D12_HEAP_FLAG_NONE,
//         &upload_buffer_desc,
//         D3D12_RESOURCE_STATE_GENERIC_READ,
//         nullptr,
//         IID_PPV_ARGS(&upload_heap_resource)
//     );

//     if(FAILED(res)) {
//         stbi_image_free(data);
//         return;
//     }

//     D3D12_SUBRESOURCE_DATA texture_data{};
//     texture_data.pData = data;
//     texture_data.RowPitch = width * 4;
//     texture_data.SlicePitch = texture_data.RowPitch * height;

//     HR(cmdallocator->Reset());

//     HR(cmdlist->Reset(cmdallocator, nullptr));

//     UpdateSubresources(cmdlist, texture, upload_heap_resource, 0,0,1,&texture_data);

//     cmdlist->ResourceBarrier(1, 
//       &CD3DX12_RESOURCE_BARRIER::Transition(texture,
//       D3D12_RESOURCE_STATE_COPY_DEST,   
//       D3D12_RESOURCE_STATE_GENERIC_READ));  
  
//     cmdlist->Close();

//     ID3D12CommandList* cmd_lists[] = {cmdlist};
//     cmdqueue->ExecuteCommandLists(_countof(cmd_lists), cmd_lists);

//     fence();

//     wait_for_gpu_fence();

//     stbi_image_free(data);

//     *pp_resource = texture;
//     safe_release(upload_heap_resource);

// }

uint DX12::align_256byte_size(uint size)
{
    return (size + 255) & ~255;
}
