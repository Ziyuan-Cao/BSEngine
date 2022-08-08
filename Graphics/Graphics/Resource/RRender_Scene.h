#pragma once 
#include "BGraphics.h"
#include "RLight.h"
#include "RMaterial.h"
#include "RTexture.h"
#include "RCamera.h"
#include "Model/RObject_Model.h"
#include "Model/RSkeleton_Model.h"
#include "Model/RStatic_Model.h"
#include <vector>

enum RENDER_SCENE_STATE
{
	CHANGE_SCENE,
	UPDATE_SCENE,
	UNCHANGE_SCENE
};


// ��ǰ����������Ⱦ����Դ
// ����ѡ���Ի��ƹ��ܺ͹�����
// 1.��̬����;�̬������
// 2.������
// 3.�ƹ���
// 4.��ͼ��
// 
class RRender_Scene : public ARender_Scene
{
	friend class BGPU_Resource_Factory;
public:

	RRender_Scene();


	virtual void UpdateMaterial() override {};

	virtual void UpdateLight() override {};

	virtual void UpdateCamera() override {};

	const UINT GetID() const { return ID; };

	struct SceneConstants
	{
		DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 ShadowTransform = MathHelper::Identity4x4();
		DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
		float cbPerObjectPad1 = 0.0f;
		DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
		DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
		float NearZ = 0.0f;
		float FarZ = 0.0f;
		float TotalTime = 0.0f;
		float DeltaTime = 0.0f;

		//light should be fixed in other buffer ?
		DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

		// Indices [0, NUM_DIR_LIGHTS) are directional lights;
		// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
		// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
		// are spot lights for a maximum of MaxLights per object.
		//RLight::LightData Lights[MaxLights];
	};

	//��Ⱦ������

	struct RenderItem
	{
		//������������
		XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
		RObject_Model* Objectmodel;
	};


	BGPU_Upload_Resource<SceneConstants>* GetSceneConstantsGPU()
	{
		return SceneconstantsGPU;
	}


	BGPU_Upload_Resource<ALight::LightData>* GetLightsGPU()
	{
		return LightGPU;
	}


	DirectX::BoundingSphere GetSceneBounds()
	{
		return SceneBounds;
	}

	const std::vector<RenderItem>& GetRenderItems() const { return Renderitems; };

protected:

	DirectX::BoundingSphere SceneBounds;

	//��Ҫ��
	BGPU_Upload_Resource<SceneConstants>* SceneconstantsGPU = nullptr;
	
	BGPU_Upload_Resource <ALight::LightData> * LightGPU = nullptr;

	//Game�й���
	std::vector<RenderItem> Renderitems;
	//���ڼ���Ƿ�����˳���
	UINT ID;
};