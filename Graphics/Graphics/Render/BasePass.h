#pragma once
#include "Pass.h"
#include "DirectX/DX_Information.h"
#include "Resource/RRender_Scene.h"
#include<string>
#include<unordered_map>

class BasePass : public pass
{
public:

	BasePass() : pass() {};

	//void UpdateObjCB(ObjectConstants objConstants);//BasePass π‹¿Ì ”Ω«

	virtual void Draw(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList, RRender_Scene* IRenderscene) override;
	virtual void OnResize(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList) override;

	//void LoadTexture();
	
private:


	std::unordered_map<std::string, ID3DBlob *> Shaders;
	std::unordered_map<std::string, ID3D12PipelineState *> PSOs;

	//UINT mSkyTexHeapIndex = 0;
	//UINT mNullCubeSrvIndex = 0;
	//UINT mShadowMapIndex = 0;
	CD3DX12_GPU_DESCRIPTOR_HANDLE NullSrv;

	bool Init = true;
private:
	void ReloadDescriptors(ID3D12Device* IDevice);

	virtual void VertexsAndIndexesInput() override;
	virtual void BuildHeaps(ID3D12GraphicsCommandList* ICmdList) override;
	virtual void BuildDescriptorHeaps(ID3D12Device* IDevice) override;
	virtual void CreateDescriptors(ID3D12Device* IDevice) override;
	virtual void BuildRootSignature(ID3D12Device* IDevice) override;
	virtual void BuildShadersAndInputLayout() override;
	virtual void BuildPSO(ID3D12Device* IDevice) override;


	virtual void DrawRenderItem(ID3D12GraphicsCommandList* ICmdList, const RRender_Scene::RenderItem& IRenderitem);

};