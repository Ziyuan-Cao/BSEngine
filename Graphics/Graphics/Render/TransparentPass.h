#include "Pass.h"
#include<unordered_map>

class TransparentPass : public pass
{
public:
	virtual void Draw(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList, RRender_Scene* IRenderscene) override;

private:
	std::unordered_map<std::string, ID3DBlob*> Shaders;
	std::unordered_map<std::string, ID3D12PipelineState*> PSOs;

	UINT LightIndex = 0;
	UINT GBufferIndex = 0;
private:

	virtual void VertexsAndIndexesInput() override;
	virtual void BuildHeaps(ID3D12GraphicsCommandList* ICmdList) override;
	virtual void BuildDescriptorHeaps(ID3D12Device* IDevice) override;
	virtual void CreateDescriptors(ID3D12Device* IDevice) override;
	virtual void BuildRootSignature(ID3D12Device* IDevice) override;
	virtual void BuildShadersAndInputLayout() override;
	virtual void BuildPSO(ID3D12Device* IDevice) override;

	virtual void DrawRenderItem(ID3D12GraphicsCommandList* ICmdList, const RRender_Scene::RenderItem& IRenderitem);

};