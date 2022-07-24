#pragma once
#include "Pass.h"
#include "DirectX/DX_Information.h"
#include "Resource/RRender_Scene.h"
#include<string>
#include<unordered_map>
//#include "ShadowMap.h"

class LightPass : public pass
{
public:

	LightPass() : pass() {};

	virtual void Update();
	virtual void Draw(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList, RRender_Scene* IRenderscene) override;

	//virtual void Initialize() override;


	//std::unique_ptr<ShadowMap> mShadowMap;

private:

	RCamera::CameraData* Cameradata;

	DirectX::BoundingSphere mSceneBounds;

	//ComPtr<ID3D12Resource> mViewCB;
	//ComPtr<ID3D12Resource> mLightCB;
	//ComPtr<ID3D12Resource> mDsTexture;

	UINT mDepthIndex = 0;

	std::unordered_map<std::string, ID3DBlob*> Shaders;
	std::unordered_map<std::string, ID3D12PipelineState*> PSOs;

	ID3D12Resource *  VB;
	D3D12_VERTEX_BUFFER_VIEW VbView;

private:
	void CreateCB();
	void UpdateConstantBuffer();

	virtual void VertexsAndIndexesInput() override;
	virtual void BuildHeaps(ID3D12GraphicsCommandList* ICmdList) override;
	virtual void BuildDescriptorHeaps(ID3D12Device* IDevice) override;
	virtual void CreateDescriptors(ID3D12Device* IDevice) override;
	virtual void BuildRootSignature(ID3D12Device* IDevice) override;
	virtual void BuildShadersAndInputLayout() override;
	virtual void BuildPSO(ID3D12Device* IDevice) override;
};