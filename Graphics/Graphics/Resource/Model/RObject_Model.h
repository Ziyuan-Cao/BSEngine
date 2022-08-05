#pragma once 
#include "BGraphics.h"
#include "Resource/RMaterial.h"
#include "Render/Auxiliary/BGPU_Upload_Resource.h"
#include<vector>

using namespace DirectX;
using namespace DirectX::PackedVector;
//��������Ҫ�;�����ʷ������
class RObject_Model : public AObject_Model
{
public:

	RObject_Model() 
	{
		
	};
//VISIBLE

	//�ֲ�����ϵ����Ͳ��ʺ�
	struct ObjectConstant
	{
		DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
		UINT     MaterialIndex;
		UINT     ObjPad0;
		UINT     ObjPad1;
		UINT     ObjPad2;
	};

	//���������CPU����
	struct CPUMeshData
	{
		using uint16 = std::uint16_t;
		using uint32 = std::uint32_t;

		std::vector<uint16> Indices16;
		BoundingBox Bounds;
		std::vector<GVertex> Vertices = {};
		std::vector<std::int32_t> Indices = {};
		//std::vector<int> TriangleMtlIndex;
		XMVECTOR VMax;
		XMVECTOR VMin;

		std::vector<ObjectConstant>  Objectconstants = {};
		std::vector<RMaterial*> Materialgroup = {};//������
		//std::vector<RTexture*> Texturegroup;//��ͼ

		
		std::vector<int> Materialsindex = {};//��������ʺ�
		//std::vector<int> Verteicesoffset = {};//�����嶥��ƫ��
		std::vector<int> Indeicesoffset = {};//����������ƫ��
		//std::vector<int> mDiffuseTextures_index;
		//std::vector<int> mNormalTextures_index;
		int Materialnums = 0;
	} CPUMeshdata;


//DLL
	//������GPU����
	struct GPUMeshData
	{
		friend class BGPU_Resource_Factory;
	private:
		//���������
		ID3DBlob* VertexbufferCPU = nullptr;
		ID3DBlob* IndexbufferCPU = nullptr;

		ID3D12Resource* VertexbufferGPU = nullptr;
		ID3D12Resource* IndexbufferGPU = nullptr;

		ID3D12Resource* Vertexbufferuploader = nullptr;
		ID3D12Resource* Indexbufferuploader = nullptr;


		UINT Vertexbytestride = 0;
		UINT Vertexbufferbytesize = 0;
		DXGI_FORMAT Indexformat = DXGI_FORMAT_R16_UINT;
		UINT Indexbufferbytesize = 0;

		//��ͼ��
		//ID3D12DescriptorHeap* TextureSRV;

	public:
		//ע�⹫����
		UINT Indexcount = 0;
		

		const D3D12_VERTEX_BUFFER_VIEW GetGPUVertexBufferView()const
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = VertexbufferGPU->GetGPUVirtualAddress();
			vbv.StrideInBytes = Vertexbytestride;
			vbv.SizeInBytes = Vertexbufferbytesize;

			return vbv;
		}

		const D3D12_INDEX_BUFFER_VIEW GetGPUIndexBufferView()const
		{
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = IndexbufferGPU->GetGPUVirtualAddress();
			ibv.Format = Indexformat;
			ibv.SizeInBytes = Indexbufferbytesize;

			return ibv;
		}

		// We can free this memory after we finish upload to the GPU.
		void DisposeGPUUploaders()
		{
			Vertexbufferuploader = nullptr;
			Indexbufferuploader = nullptr;
		}

	};

	BGPU_Upload_Resource<AMaterial::MaterialData>* GetMaterialConstantsGPU()
	{
		return MaterialconstantsGPU;
	}

	BGPU_Upload_Resource<RObject_Model::ObjectConstant>* GetObjectConstantsGPU()
	{
		return ObjectconstantsGPU;
	}

	void SetPrimitiveType(int IPolygonType)
	{
		switch (IPolygonType)
		{
		case 4:
			Primitivetype = D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
			break;
		default:
			Primitivetype = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		}
	}

	D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveType()
	{
		return Primitivetype;
	}

	const std::vector<GPUMeshData>& GetGPUGeometries()const
	{
		return GPUGeometries;
	}

	const UINT GetGeometriesnumber() const
	{
		return Geometriesnumber;
	}

protected:

	//���������干��һ�����㻺��
	ID3DBlob* VertexbufferCPU = nullptr;
	ID3D12Resource* VertexbufferGPU = nullptr;
	ID3D12Resource* Vertexbufferuploader = nullptr;
	UINT Vertexbytestride = 0;
	UINT Vertexbufferbytesize = 0;

	//���ʶ�
	BGPU_Upload_Resource<AMaterial::MaterialData>* MaterialconstantsGPU = nullptr;
	//����������
	BGPU_Upload_Resource<RObject_Model::ObjectConstant>* ObjectconstantsGPU = nullptr;
	//������
	std::vector<GPUMeshData> GPUGeometries;
	UINT Geometriesnumber = 0;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY Primitivetype = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	friend class BGPU_Resource_Factory;
};


