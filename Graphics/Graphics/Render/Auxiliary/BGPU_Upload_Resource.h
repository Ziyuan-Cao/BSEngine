#pragma once

#include "DirectX/d3dx12.h"
#include "Pre_Define.h"
#include "TMathTool.h"
template<typename T>
class BGPU_Upload_Resource
{
public:
    BGPU_Upload_Resource(ID3D12Device* Idevice, UINT IelementCount, bool IisConstantBuffer) : 
        IsConstantbuffer(IisConstantBuffer)
    {
        Elementbytesize = sizeof(T);
        Memorysize = Elementbytesize * IelementCount;
        // Constant buffer elements need to be multiples of 256 bytes.
        // This is because the hardware can only view constant data 
        // at m*256 byte offsets and of n*256 byte lengths. 
        // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
        // UINT64 OffsetInBytes; // multiple of 256
        // UINT   SizeInBytes;   // multiple of 256
        // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
        if(IsConstantbuffer)
            Elementbytesize = MathHelper::CalcConstantBufferByteSize(sizeof(T));

        CD3DX12_HEAP_PROPERTIES Heapproperties(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC Resourcebuffer = CD3DX12_RESOURCE_DESC::Buffer(Elementbytesize * IelementCount);

        ThrowIfFailed(Idevice->CreateCommittedResource(
            &Heapproperties,
            D3D12_HEAP_FLAG_NONE,
            &Resourcebuffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&Uploadresource)));

        ThrowIfFailed(Uploadresource->Map(0, nullptr, reinterpret_cast<void**>(&Mappeddata)));

        // We do not need to unmap until we are done with the resource.  However, we must not write to
        // the resource while it is in use by the GPU (so we must use synchronization techniques).
    }

    BGPU_Upload_Resource(const BGPU_Upload_Resource& rhs) = delete;
    BGPU_Upload_Resource& operator=(const BGPU_Upload_Resource& rhs) = delete;
    ~BGPU_Upload_Resource()
    {
        if(Uploadresource != nullptr)
            Uploadresource->Unmap(0, nullptr);

        Mappeddata = nullptr;
    }

    ID3D12Resource* Resource()const
    {
        return Uploadresource;
    }

    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&Mappeddata[elementIndex*Elementbytesize], &data, sizeof(T));
    }
    void Copy(void* data)
    {
        memcpy(&Mappeddata[0], data, Memorysize);
    }

private:
    ID3D12Resource * Uploadresource;
    BYTE* Mappeddata = nullptr;

    UINT Elementbytesize = 0;
    UINT Memorysize = 0;
    bool IsConstantbuffer = false;
};