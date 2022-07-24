#include "Render/BDeferredRendering.h"

//��ʼ������
BDeferredRendering::BDeferredRendering(
	ID3D12Device* IDevice,
	ID3D12GraphicsCommandList* ICmdList)
{
	Basepass = new BasePass();
	Lightpass = new LightPass();
	Mixpass = new MixPass();

	Basepass->OnResize(IDevice, ICmdList);
	Lightpass->OnResize(IDevice, ICmdList);
	Mixpass->OnResize(IDevice, ICmdList);

}

void BDeferredRendering::Initialize(ID3D12Device* IDevice,
	ID3D12GraphicsCommandList* ICmdList)
{
	Basepass->Initialize(IDevice, ICmdList);
	Lightpass->Initialize(IDevice, ICmdList);
	Mixpass->Initialize(IDevice, ICmdList);
}


//�����ӳ���Ⱦȫ����
//
void BDeferredRendering::Render(
	ID3D12Device* IDevice,
	ID3D12GraphicsCommandList* ICmdList, 
	RRender_Scene* IRenderscene,
	ID3D12Resource* ORendertarget,
	D3D12_CPU_DESCRIPTOR_HANDLE ORendertargetView)
{
	Lightpass->Update();

	Basepass->Draw(IDevice, ICmdList, IRenderscene);
	Lightpass->Draw(IDevice, ICmdList, IRenderscene);
	Mixpass->Draw(IDevice, ICmdList, IRenderscene, ORendertarget, ORendertargetView);
}

