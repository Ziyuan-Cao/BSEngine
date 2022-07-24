#pragma once
#include "Pre_Define.h"
#include <map>
#include <string>

using namespace DirectX;
using namespace DirectX::PackedVector;

enum DESC_TYPE_FLAG
{
	SRV,
	RTV,
	DSV,
	CBV,
	UAV
};
/*
*1.构建对应（SRV,CBV,DSV,RTV）描述符堆（PASS自己的）
*2.往描述符堆塞资源的描述符 （RTCLASS的）
*3.构建根签名时塞输入数据(SRV,CBV) (深度图也需要构建对应的SRV才能作为输入数据）（PASS自己的）
*4.构建PSO时设定输出数据（RTV）,可设定多张 （PASS自己的）
*5.	SetDescriptorHeaps（SRVHeap）
*	SetGraphicsRootSignature(RootSignature)
*	SetPipelineState(PSO)
*6.每个资源需要ResourceBarrier（），对应的状态需要实现循环转换 （PASS自己的）
*7.	SetGraphicsRootConstantBufferView()根签名定义时对应顺序的输入资源
*	SetGraphicsRootDescriptorTable()
*/
struct Resource
{
	ID3D12Resource* resource;
	/*int RTV_index = -1;
	int SRV_index = -1;
	int DSV_index = -1;
	int CBV_index = -1;
	int UAV_index = -1;*/
    D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
    D3D12_RENDER_TARGET_VIEW_DESC RTV;
    D3D12_DEPTH_STENCIL_VIEW_DESC DSV;
    D3D12_CONSTANT_BUFFER_VIEW_DESC CBV;
    D3D12_UNORDERED_ACCESS_VIEW_DESC UAV;

    DXGI_FORMAT Format;
};

class BResource_Heap
{
private:
	ID3D12Device* DXDevice;
	std::map<std::string, Resource> Resourcepool;
	/*std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> mSRVDescs;
	std::vector<D3D12_RENDER_TARGET_VIEW_DESC> mRTVDescs;
	std::vector<D3D12_DEPTH_STENCIL_VIEW_DESC> mDSVDescs;
	std::vector<D3D12_CONSTANT_BUFFER_VIEW_DESC> mCBVDescs;
	std::vector<D3D12_UNORDERED_ACCESS_VIEW_DESC> mUAVDescs;
    */
public:
	BResource_Heap(ID3D12Device*  d3dDevice) : DXDevice(d3dDevice) {};
    
    void Resize();

	void AddResource(std::string name,
		D3D12_RESOURCE_DESC& resourceDesc,
		void* data = nullptr,
		size_t data_size = 0,
		D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATES resourceStates = D3D12_RESOURCE_STATE_COMMON,
		D3D12_CLEAR_VALUE* clearValue = nullptr);

	//template<typename  T>
	//void CreateDesc(const std::string& name, T desc, DESC_TYPE_FLAG flag);
    //template<typename  T>
    //T GetDesc(const std::string & name, DESC_TYPE_FLAG flag);

    void CreateSRVDesc(const std::string& name, D3D12_SHADER_RESOURCE_VIEW_DESC desc);
    void CreateRTVDesc(const std::string& name, D3D12_RENDER_TARGET_VIEW_DESC desc);
    void CreateDSVDesc(const std::string& name, D3D12_DEPTH_STENCIL_VIEW_DESC desc);
    void CreateCBVDesc(const std::string& name, D3D12_CONSTANT_BUFFER_VIEW_DESC desc);
    void CreateUAVDesc(const std::string& name, D3D12_UNORDERED_ACCESS_VIEW_DESC desc);

	
    const D3D12_SHADER_RESOURCE_VIEW_DESC* const GetSRVDesc(const std::string& name);
    const D3D12_RENDER_TARGET_VIEW_DESC* const GetRTVDesc(const std::string& name);
    const D3D12_DEPTH_STENCIL_VIEW_DESC* const GetDSVDesc(const std::string& name);
    const D3D12_CONSTANT_BUFFER_VIEW_DESC* const GetCBVDesc(const std::string& name);
    const D3D12_UNORDERED_ACCESS_VIEW_DESC* const GetUAVDesc(const std::string& name);

    DXGI_FORMAT  GetFormat(const std::string& name);
	ID3D12Resource* GetResource(std::string name) const
	{
		return Resourcepool.find(name)->second.resource;
	}
};


/*template<typename T>
void BResource_Heap::CreateDesc(const std::string& name, T desc, DESC_TYPE_FLAG flag)
{
    switch (flag)
    {
    case SRV:
        Resourcemap[name].SRV = desc;
        //Resourcemap[name].SRV_index = mSRVDescs.size();
        //mSRVDescs.push_back(desc);
        break;
    case RTV:
        Resourcemap[name].RTV = desc;
        //Resourcemap[name].RTV_index = mRTVDescs.size();
        //mRTVDescs.push_back(desc);
        break;
    case DSV:
        Resourcemap[name].DSV = desc;
        //Resourcemap[name].DSV_index = mDSVDescs.size();
        //mDSVDescs.push_back(desc);
        break;
    case CBV:
        Resourcemap[name].CBV = desc;
        //Resourcemap[name].CBV_index = mCBVDescs.size();
        //mCBVDescs.push_back(desc);
        break;
    case UAV:
        Resourcemap[name].UAV = desc;
        //Resourcemap[name].UAV_index = mUAVDescs.size();
        //mUAVDescs.push_back(desc);
        break;
    }
}

/*flag:
* 0 for SRV
* 1 for RTV
* 2 for DSV
* 3 for CBV
* 4 for UAV
*/
/*
template<typename T>
T  BResource_Heap::GetDesc(const std::string& name, DESC_TYPE_FLAG flag)
{
    Resource R = Resourcemap.find(name)->second;
    
    switch (flag)
    {
    case SRV:
        return R.SRV;
        //return mSRVDescs[Resourcemap.find(name)->second.SRV_index];
        break;
    case RTV:
        return R.RTV;
        //return mRTVDescs[Resourcemap.find(name)->second.RTV_index];
        break;
    case DSV:
        return R.DSV;
        //return mDSVDescs[Resourcemap.find(name)->second.DSV_index];
        break;
    case CBV:
        return R.CBV;
        //return mCBVDescs[Resourcemap.find(name)->second.CBV_index];
        break;
    case UAV:
        return R.UAV;
        //return mUAVDescs[Resourcemap.find(name)->second.UAV_index];
        break;
    }
    return nullptr;
}*/