#pragma once
#include "Pass.h"
#include "DirectX/DX_Information.h"
#include "Resource/RRender_Scene.h"
#include<string>
#include<unordered_map>

class SSAOPass : public pass
{
public:

	SSAOPass() : pass() {};

	virtual void Draw(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList, RRender_Scene* IRenderscene) override;
	virtual void OnResize(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList) override;

private:
	bool Init = false;

	std::unordered_map<std::string, ID3DBlob*> Shaders;
	std::unordered_map<std::string, ID3D12PipelineState*> PSOs;

	D3D12_VERTEX_BUFFER_VIEW VbView;

	UINT DepthIndex = 0;
	UINT NoiseIndex = 0;
	struct NoiseMap
	{
		float data[192];
	};

	D3D12_SUBRESOURCE_DATA NoiseMapGPUSUBRESOURCE;
	ID3D12Resource* NoiseMapGPU;
	ID3D12Resource* textureUploadHeap;
	D3D12_SHADER_RESOURCE_VIEW_DESC NoiseMapdescSRV;

private:

	void BuildNoiseMap(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList);

	virtual void VertexsAndIndexesInput() override;
	virtual void BuildHeaps(ID3D12GraphicsCommandList* ICmdList) override;
	virtual void BuildDescriptorHeaps(ID3D12Device* IDevice) override;
	virtual void CreateDescriptors(ID3D12Device* IDevice) override;
	virtual void BuildRootSignature(ID3D12Device* IDevice) override;
	virtual void BuildShadersAndInputLayout() override;
	virtual void BuildPSO(ID3D12Device* IDevice) override;

};