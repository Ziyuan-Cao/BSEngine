#pragma once
#include "Pre_Define.h"
#include "BasePass.h"
#include "LightPass.h"
#include "MixPass.h"
#include "ShadowPass.h"
#include "SSAOPass.h"
//#include "Auxiliary/BFrame_Resource.h"

//延迟渲染主流程
//遍历RRender_Scene绘制
//
class BDeferredRendering
{
public:
	BDeferredRendering(
		ID3D12Device* IDevice,
		ID3D12GraphicsCommandList* ICmdList);

	void Initialize(ID3D12Device* IDevice,
		ID3D12GraphicsCommandList* ICmdList);

	void Render(ID3D12Device* IDevice,
		ID3D12GraphicsCommandList* ICmdList,
		RRender_Scene* IRenderscene,
		ID3D12Resource* ORendertarget,
		D3D12_CPU_DESCRIPTOR_HANDLE ORendertargetView);

protected:

	ShadowPass* Shadowpass;
	BasePass* Basepass;
	LightPass* Lightpass;
	SSAOPass* SSAOpass;
	MixPass* Mixpass;

};