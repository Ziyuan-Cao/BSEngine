#pragma once

#include "Pre_Define.h"
#include "MathHelper.h"
#include "BGPU_Upload_Resource.h"
#include "Resource/RRender_Sence.h"

// Stores the resources needed for the CPU to build the command lists
// for a frame.  
//帧资源
//储存每帧的模型 材质 灯光 摄像机等数据
//要求反射回调刷新缓存内数据

struct BFrame_Resource
{
public:
    
    BFrame_Resource(ID3D12Device* Idevice, UINT IMaterialnum, UINT ILightnum, UINT ICameranum);
    BFrame_Resource(const BFrame_Resource& rhs) = delete;
    BFrame_Resource& operator=(const BFrame_Resource& rhs) = delete;
    ~BFrame_Resource();

    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    ID3D12CommandAllocator* CmdListAlloc;

    // We cannot update a cbuffer until the GPU is done processing the commands
    // that reference it.  So each frame needs their own cbuffers.

    //???
    //std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;

    std::unique_ptr<BGPU_Upload_Resource<RMaterial>> MaterialGPUCB = nullptr;

    std::unique_ptr<BGPU_Upload_Resource<RLight>> LightGPUCB = nullptr;

    std::unique_ptr<BGPU_Upload_Resource<RCamera>> CameraGPUCB = nullptr;

    // Fence value to mark commands up to this fence point.  This lets us
    // check if these frame resources are still in use by the GPU.
    UINT64 Fence = 0;
};