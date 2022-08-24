#pragma once

#include "Base/Tool/TMathTool.h"
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <Windows.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX;

#ifdef  DLL_GRAPHICS_API
#else
#define DLL_GRAPHICS_API _declspec(dllimport)
#endif


//提供资源类型，读写功能
//提供渲染类和方法

#ifdef __cplusplus
extern "C" 
{
#endif

	class AResource
	{
	public:
		std::wstring ID{};
		std::wstring Name{};
		std::wstring Filepath{};
	};

	class AMaterial : public AResource
	{
	public:
		struct MaterialData
		{
			DirectX::XMFLOAT3 Lightcolor = { 1.0f, 1.0f, 1.0f };
			DirectX::XMFLOAT3 Specularcolor = { 1.0f, 1.0f, 1.0f };
			DirectX::XMFLOAT3 Shadowcolor = { 0.5f, 0.5f, 0.5f };
			DirectX::XMFLOAT3 Profilecolor = { 0.0f, 0.0f, 0.0f };
			float Thick = 0.f;
			float Shadowsmooth = 0.1f;
			float Shadowsoft = 0.1f;
			float Specularsmooth = 0.1f;
			float Specularsoft = 0.1f;

			float Depthstrength = 0.5f;
			float Depthdistance = 3.0f;
			float Normalstrength = 10.0f;
			float Normaldistance = 5.0f;

			// Used in texture mapping.
			DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();

			UINT     DiffuseMapIndex = 0;
			UINT     NormalMapIndex = 0;
			UINT	 LayerIndex = 0;
		};

		struct WaterData
		{
			DirectX::XMFLOAT2 windDir = { -0.5, -0.6 };
			float windSpeed = 1.0;
			float waveSteepness = 0.72;
			float waveTiling = 1.0;
			DirectX::XMFLOAT4 waveAmplitude = { 0.13, 0.3, 0.1, 0.05 };
			float waveAmplitudeFactor = 0.25;
			DirectX::XMFLOAT4 wavesIntensity = { 5, 3, 2, 1.3 };
			DirectX::XMFLOAT4 wavesNoise = { 0.15, 0.32, 0.15, 0.15 };
			float _heightIntensity = 0.3;
			float textureTiling = 3.5;
		};

	protected:
		MaterialData Materialdata;

	public:
		
		void SetLightColor(DirectX::XMFLOAT3 ILightcolor) 
		{
			Materialdata.Lightcolor = ILightcolor;
		}

	};

	class  ALight : public AResource
	{
	public:
		struct LightData
		{
			DirectX::XMFLOAT3 Strength = { 10.0f, 10.0f, 10.0f };
			float FalloffStart = 1.0f;                          // point/spot light only
			DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
			float FalloffEnd = 10.0f;                           // point/spot light only
			DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
			float SpotPower = 64.0f;                            // spot light only
		};

		LightData Lightdata;
	};

	class ACamera : public AResource
	{
	public:
		virtual void SetPosition(float x, float y, float z) = 0;
		virtual void RotateY(float angle) = 0;
	};



	class ATexture : public AResource
	{

	public:
		enum TEXTURE_TYPE
		{
			COLOR_TEXTURE, 
			NORMAL_TEXTURE,
			WATERFLOW_TEXTURE,
			WATERVIE_TEXTURE,
			SPECULAR_TEXTURE // Useless
		} Type = COLOR_TEXTURE;
	};

	class AObject_Model : public AResource
	{
	public:
		bool Visible = true;

		bool isGPUInit = false;
		//Debug
		bool isTransparent = false;
		bool isWater = false;
		bool isStatic = true;
		bool hasAnimation = false;
		bool caseShadow = true;
		int Materialnums = 0;

		//坐标系矩阵
		float Transform[3] = { 0,0,0 };
		float Rotation[3] = { 0,0,0 };
		float Scale[3] = { 0,0,0 };
	};

	class AStatic_Model : public AObject_Model
	{

	};

	class ASkeleton_Model : public AObject_Model
	{

	};


	class DLL_GRAPHICS_API AResource_Factory
	{
	public:
		//A真正读取为R类型
		bool LoadData();

		bool LoadFbx(AObject_Model* IOObjectmodel, std::wstring IDictionary);

		AObject_Model* CreateSkeletonModel();

		AObject_Model* CreateStaticModel();

		ATexture* CreateTexture();

		ACamera* CreateCamera();

		ALight* CreateLight();

		AMaterial* CreateMaterial();

		bool AddMaterial(AObject_Model* IOObjectmodel, std::vector<AMaterial*>& IMaterial);
	
		bool RelateTexturetoMaterial(AMaterial* IMaterial, std::vector<ATexture*>& ITextures);

		bool LoadTexture(ATexture*& IOTexture, std::wstring IDictionary);

		bool LoadTextures(std::vector<ATexture*>& IOTextures, std::wstring IDictionary);

		bool LoadObject(AObject_Model* IOObjectmodel, std::vector<ATexture*>& IOTextures, std::wstring IDictionary);

	private:
		bool LoadHierarchy();
	};

	class DLL_GRAPHICS_API ARender_Scene
	{
	public:

		//材质刷新
		virtual void UpdateMaterial() = 0;

		virtual void UpdateLight() = 0;

		virtual void UpdateCamera() = 0;

		
		std::vector<ALight*> Lightgroup;
		std::vector<AObject_Model*> Staticgroup;
		std::vector<AObject_Model*> Skeletongroup;
		std::vector<ACamera*> Cameragroup;

		const std::vector<AObject_Model*>* GetSkeletonGroup() const { return &Skeletongroup; };
		const std::vector<AObject_Model*>* GetStaticGroup() const { return &Staticgroup; };

	};

	class DLL_GRAPHICS_API ARender_Scene_Factory
	{
	public:
		ARender_Scene* CreatRenderScene();
	};

	class ARenderer
	{
	public:
		//virtual void CreateWindows() = 0;
		//进入程序调用
		virtual void RenderInitialize() = 0;
		virtual void RenderInitialize(HINSTANCE IHINSTANCE, UINT IWidth, UINT IHeight) = 0;
		virtual void RenderInitialize(HWND IHWND, UINT IWidth, UINT IHeight) = 0;

		//每帧调用
		virtual int Render(ARender_Scene* IRenderscene, bool IsDebug = true) = 0;
		//释放关卡资源
		virtual void UnLoadRenderingResource() = 0;

		virtual void UpdateGPUResource() = 0;

	};

	class DLL_GRAPHICS_API ARenderer_Factory
	{

	public:
		 ARenderer* CreateRenderer();
	};

	class TTimer
	{
	public:
		TTimer();

		void Reset(); // Call before message loop
		void Start(); //启动
		void Stop();  //暂停
		void Tick();  //每帧调用刷新当前时间

		//距离上一次Reset（）的时间，秒为单位
		float TotalTime()const;
		//距离上一帧的时间，秒为单位
		float DeltaTime()const;



	private:
		__int64 Basetime;//起始时间
		__int64 Pausedtime;//总停止时间
		__int64 Stoptime;//上一次停顿时间
		__int64 Prevtime;//上一帧时间
		__int64 Currtime;//当前帧时间

		double Secondspercount;//cpu刷新时间（秒为单位）
		double Deltatime;//距离上一帧的时间，秒为单位

		bool Stopped;
	};



#ifdef __cplusplus
}
#endif