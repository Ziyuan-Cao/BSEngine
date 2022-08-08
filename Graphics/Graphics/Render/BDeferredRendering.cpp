#include "Render/BDeferredRendering.h"

//初始化过程
BDeferredRendering::BDeferredRendering(
	ID3D12Device* IDevice,
	ID3D12GraphicsCommandList* ICmdList)
{
	Basepass = new BasePass();
	Lightpass = new LightPass();
	Mixpass = new MixPass();
	Shadowpass = new ShadowPass();

	Shadowpass->OnResize(IDevice, ICmdList);
	Basepass->OnResize(IDevice, ICmdList);
	Lightpass->OnResize(IDevice, ICmdList);
	Mixpass->OnResize(IDevice, ICmdList);

}

void BDeferredRendering::Initialize(ID3D12Device* IDevice,
	ID3D12GraphicsCommandList* ICmdList)
{
	Shadowpass->Initialize(IDevice, ICmdList);
	Basepass->Initialize(IDevice, ICmdList);
	Lightpass->Initialize(IDevice, ICmdList);
	Mixpass->Initialize(IDevice, ICmdList);
}


//安排延迟渲染全流程
//
void BDeferredRendering::Render(
	ID3D12Device* IDevice,
	ID3D12GraphicsCommandList* ICmdList, 
	RRender_Scene* IRenderscene,
	ID3D12Resource* ORendertarget,
	D3D12_CPU_DESCRIPTOR_HANDLE ORendertargetView)
{
	Shadowpass->UpdateShadowConstant(IRenderscene, (RLight*)IRenderscene->Lightgroup[0]);

	Shadowpass->Draw(IDevice, ICmdList, IRenderscene);

	Basepass->Draw(IDevice, ICmdList, IRenderscene);
	
	Lightpass->Draw(IDevice, ICmdList, IRenderscene);

	//reflection

	//SSAO

	//

	Mixpass->Draw(IDevice, ICmdList, IRenderscene, ORendertarget, ORendertargetView);
}

