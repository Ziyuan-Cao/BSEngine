#pragma once
#include "BGPU_Resource_Factory.h"
#include "Resource/RRender_Scene.h"
#include <vector>

using namespace DirectX;
using namespace DirectX::PackedVector;

class pass
{
public:
	pass(){ };
	pass(const pass& rhs) = delete;
	pass& operator=(const pass& rhs) = delete;
	~pass() {};

	virtual void Draw(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList, RRender_Scene* IRenderscene) {};

	virtual void Initialize(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList)
	{
		BuildRootSignature(IDevice);
		BuildShadersAndInputLayout();
		BuildPSO(IDevice);
	};

	virtual void OnResize(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList)
	{
		BuildDescriptorHeaps(IDevice);
		VertexsAndIndexesInput();
		BuildHeaps(ICmdList);
		CreateDescriptors(IDevice);
	};

protected:
	ID3D12DescriptorHeap* SRVHeap = nullptr;
	ID3D12DescriptorHeap* RTVHeap = nullptr;
	ID3D12DescriptorHeap* DSVHeap = nullptr;
	ID3D12DescriptorHeap* CBVHeap = nullptr;
	ID3D12DescriptorHeap* UAVHeap = nullptr;
	ID3D12RootSignature* RootSignature = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

private:

	

	virtual void VertexsAndIndexesInput() = 0;
	virtual void BuildHeaps(ID3D12GraphicsCommandList* ICmdList) = 0;
	virtual void BuildDescriptorHeaps(ID3D12Device* IDevice) = 0;
	virtual void CreateDescriptors(ID3D12Device* IDevice) = 0;
	virtual void BuildRootSignature(ID3D12Device* IDevice) = 0;
	virtual void BuildShadersAndInputLayout() = 0;
	virtual void BuildPSO(ID3D12Device* IDevice) = 0;
};
