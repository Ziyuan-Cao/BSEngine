#include "Render/BDeferredRendering.h"
#include "DirectX/DX_Information.h"

//初始化过程
BDeferredRendering::BDeferredRendering(
	ID3D12Device* IDevice,
	ID3D12GraphicsCommandList* ICmdList)
{
	DX_Information* DXInf = DX_Information::GetInstance();
	Shadowpass = new ShadowPass(DXInf->GetWClientWidth(), DXInf->GetWClientHeight());
	Basepass = new BasePass();
	Lightpass = new LightPass();
	Transparentpass = new TransparentPass();
	SSAOpass = new SSAOPass();
	Mixpass = new MixPass();

	Shadowpass->OnResize(IDevice, ICmdList);
	Basepass->OnResize(IDevice, ICmdList);
	Lightpass->OnResize(IDevice, ICmdList);
	Transparentpass->OnResize(IDevice, ICmdList);
	SSAOpass->OnResize(IDevice, ICmdList);
	Mixpass->OnResize(IDevice, ICmdList);

}

void BDeferredRendering::Initialize(ID3D12Device* IDevice,
	ID3D12GraphicsCommandList* ICmdList)
{
	Shadowpass->Initialize(IDevice, ICmdList);
	Basepass->Initialize(IDevice, ICmdList);
	Lightpass->Initialize(IDevice, ICmdList);
	Transparentpass->Initialize(IDevice, ICmdList);
	SSAOpass->Initialize(IDevice, ICmdList);
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

	Transparentpass->Draw(IDevice, ICmdList, IRenderscene);

	SSAOpass->Draw(IDevice, ICmdList, IRenderscene);

	//reflection


	Mixpass->Draw(IDevice, ICmdList, IRenderscene, ORendertarget, ORendertargetView);
}

