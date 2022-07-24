#include "Render/BResource_Heap.h"

void BResource_Heap::AddResource(std::string name,
    D3D12_RESOURCE_DESC & resourceDesc,
    void* data,
    size_t data_size,
    D3D12_HEAP_TYPE heapType,
    D3D12_RESOURCE_STATES resourceStates,
    D3D12_CLEAR_VALUE *clearValue)
{
    Resource mresource;
    mresource.Format = resourceDesc.Format;

    CD3DX12_HEAP_PROPERTIES Heapproperties(heapType);

    ThrowIfFailed(DXDevice->CreateCommittedResource(
        &Heapproperties,
        D3D12_HEAP_FLAG_NONE ,
        &resourceDesc,
        resourceStates,
        clearValue,
        IID_PPV_ARGS(&mresource.resource)));
    if (data)
    {
        UINT8* dataBegin;
        ThrowIfFailed(mresource.resource->Map(0, nullptr, reinterpret_cast<void**>(&dataBegin)));
        memcpy(dataBegin, data, data_size);
        mresource.resource->Unmap(0, nullptr);
    }
    Resourcepool[name] = mresource;
}

void BResource_Heap::CreateSRVDesc(const std::string& name, D3D12_SHADER_RESOURCE_VIEW_DESC desc)
{
    Resourcepool[name].SRV = desc;
}
void BResource_Heap::CreateRTVDesc(const std::string& name, D3D12_RENDER_TARGET_VIEW_DESC desc) 
{
    Resourcepool[name].RTV = desc;
}
void BResource_Heap::CreateDSVDesc(const std::string& name, D3D12_DEPTH_STENCIL_VIEW_DESC desc)
{
    Resourcepool[name].DSV = desc;
}
void BResource_Heap::CreateCBVDesc(const std::string& name, D3D12_CONSTANT_BUFFER_VIEW_DESC desc)
{
    Resourcepool[name].CBV = desc;
}
void BResource_Heap::CreateUAVDesc(const std::string& name, D3D12_UNORDERED_ACCESS_VIEW_DESC desc)
{
    Resourcepool[name].UAV = desc;
}

DXGI_FORMAT  BResource_Heap::GetFormat(const std::string& name)
{
    return Resourcepool[name].Format;
}

const D3D12_SHADER_RESOURCE_VIEW_DESC* const BResource_Heap::GetSRVDesc(const std::string& name)
{
    return &Resourcepool[name].SRV;
}
const D3D12_RENDER_TARGET_VIEW_DESC* const BResource_Heap::GetRTVDesc(const std::string& name)
{
    return &Resourcepool[name].RTV;
}
const D3D12_DEPTH_STENCIL_VIEW_DESC* const BResource_Heap::GetDSVDesc(const std::string& name)
{
    return &Resourcepool[name].DSV;
}
const D3D12_CONSTANT_BUFFER_VIEW_DESC* const BResource_Heap::GetCBVDesc(const std::string& name)
{
    return &Resourcepool[name].CBV;
}
const D3D12_UNORDERED_ACCESS_VIEW_DESC* const BResource_Heap::GetUAVDesc(const std::string& name)
{
    return &Resourcepool[name].UAV;
}

//??? 所有资源都要Resize
void BResource_Heap::Resize()
{
    if (Resourcepool.empty())
        return;
    Resourcepool.clear();
}
