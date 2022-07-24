#include "Render/Auxiliary/BFrame_Resource.h"

BFrame_Resource::BFrame_Resource(ID3D12Device* Idevice, RRender_Sence* IRendersence)
{
    ThrowIfFailed(Idevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&CmdListAlloc)));


	//->GetMaterialNum(), Rendersence->GetLightNum(), Rendersence->GetCameraNum()
	//MaterialGPUCB = std::make_unique<BGPU_Upload_Resource<RMaterial>>(Idevice, IMaterialnum, false);
	//LightGPUCB = std::make_unique<BGPU_Upload_Resource<RLight>>(Idevice, ILightnum, false);
	//CameraGPUCB = std::make_unique<BGPU_Upload_Resource<RCamera>>(Idevice, ICameranum, false);
}

BFrame_Resource::~BFrame_Resource()
{

}