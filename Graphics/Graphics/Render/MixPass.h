#pragma once
#include "Pass.h"
//#include "ShadowMap.h"
#include "DirectX/DX_Information.h"
#include<unordered_map>

class MixPass : public pass
{
public:

	MixPass() : pass() {};

	//virtual void Update(const GameTimer& gt) override;
	virtual void Draw(ID3D12Device* IDevice,
		ID3D12GraphicsCommandList* ICmdList,
		RRender_Scene* IRenderscene,
		ID3D12Resource* ORendertarget,
		D3D12_CPU_DESCRIPTOR_HANDLE ORendertargetView);


	//std::unique_ptr<ShadowMap> mShadowMap;

private:
	ID3D12Resource* ViewCB;
	ID3D12Resource* LightCB;
	ID3D12Resource* DsTexture;

	UINT GBufferIndex = 0;
	UINT SSAOIndex = 0;
	UINT DepthIndex = 0;
	UINT ProfileIndex = 0;
	UINT LightIndex = 0;
	UINT TransparentIndex = 0;

	std::unordered_map<std::string, ID3DBlob*> Shaders;
	std::unordered_map<std::string, ID3D12PipelineState*> PSOs;

	ID3D12Resource* VB;
	D3D12_VERTEX_BUFFER_VIEW VbView;

private:

	virtual void VertexsAndIndexesInput() override;
	virtual void BuildHeaps(ID3D12GraphicsCommandList* ICmdList) override;
	virtual void BuildDescriptorHeaps(ID3D12Device* IDevice) override;
	virtual void CreateDescriptors(ID3D12Device* IDevice) override;
	virtual void BuildRootSignature(ID3D12Device* IDevice) override;
	virtual void BuildShadersAndInputLayout() override;
	virtual void BuildPSO(ID3D12Device* IDevice) override;
};