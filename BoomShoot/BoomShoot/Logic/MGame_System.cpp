#include "MGame_System.h"
#include "LInput_Process.h"
#include "Rendering/BGraphics.h"
#pragma comment(lib,"Graphics.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
		
	//Debug
	wchar_t* path = new wchar_t[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, path);
	printf("%s\n", path);

	//Renderer
	ARenderer_Factory Rendererfactory;
	ARenderer* Renderer;
	Renderer = Rendererfactory.CreateRenderer();
	Renderer->RenderInitialize(hInstance,800,600);

	//Renderscene
	ARender_Scene_Factory Renderscenefactory;
	ARender_Scene *Ars;
	Ars = Renderscenefactory.CreatRenderScene();

	//Object
	{
		//AObject_Model* AObjectmodel;
		//AResource_Factory Resourcefactory;
		//AObjectmodel = Resourcefactory.CreateSkeletonModel();
		//Resourcefactory.LoadFbx(AObjectmodel, L"FBXResouce\\FBXa\\");
		////Material
		//int Matnums = AObjectmodel->Materialnums;
		//std::vector<AMaterial*> Materialgroup = {};
		//Materialgroup.assign(Matnums, nullptr);
		//for (int i = 0; i < Matnums; i++)
		//{
		//	Materialgroup[i] = Resourcefactory.CreateMaterial();
		//}
		//Resourcefactory.AddMaterial(AObjectmodel, Materialgroup);
		////ObjCB
		//AObjectmodel->Transform[0] = 0.0f;
		//AObjectmodel->Transform[1] = 0.0f;
		//AObjectmodel->Transform[2] = -50.0f;
		//AObjectmodel->Rotation[0] = 3.15f;
		//AObjectmodel->Rotation[1] = 0.0f;
		//AObjectmodel->Rotation[2] = 0.0f;
		//AObjectmodel->Scale[0] = 0.04f;
		//AObjectmodel->Scale[1] = 0.04f;
		//AObjectmodel->Scale[2] = 0.04f;
		//Ars->Skeletongroup.push_back(AObjectmodel);
	}

	{
		AObject_Model* AObjectmodel;
		AResource_Factory Resourcefactory;
		AObjectmodel = Resourcefactory.CreateStaticModel();
		std::vector<ATexture*> Texturegroup = {};
		Resourcefactory.LoadObject(AObjectmodel, Texturegroup, L"FBXResouce\\FBXr\\");

		//Material
		int Matnums = AObjectmodel->Materialnums;
		std::vector<AMaterial*> Materialgroup = {};
		Materialgroup.assign(Matnums, nullptr);
		for (int i = 0; i < Matnums; i++)
		{
			Materialgroup[i] = Resourcefactory.CreateMaterial();
			Materialgroup[i]->SetLightColor({ 0.09f *i,0.4f, 0.4f });
			std::vector<ATexture*> Texturegroupbuffer;
			Texturegroupbuffer.push_back(Texturegroup[i]);
			Resourcefactory.RelateTexturetoMaterial(Materialgroup[i], Texturegroupbuffer);
		}
		Resourcefactory.AddMaterial(AObjectmodel, Materialgroup);
		//ObjCB
		AObjectmodel->Transform[0] = -70.0f;
		AObjectmodel->Transform[1] = 0.0f;
		AObjectmodel->Transform[2] = 0.0f;
		AObjectmodel->Rotation[0] = 3.15f;
		AObjectmodel->Rotation[1] = 0.0f;
		AObjectmodel->Rotation[2] = 1.5f;
		AObjectmodel->Scale[0] = 0.1f;
		AObjectmodel->Scale[1] = 0.1f;
		AObjectmodel->Scale[2] = 0.1f;
		Ars->Skeletongroup.push_back(AObjectmodel);
	}

	{
		AObject_Model* AObjectmodel;
		AResource_Factory Resourcefactory;
		AObjectmodel = Resourcefactory.CreateStaticModel();
		AObjectmodel->caseShadow = false;
		Resourcefactory.LoadFbx(AObjectmodel, L"FBXResouce\\FBXPlane\\");
		//Material
		int Matnums = AObjectmodel->Materialnums;
		std::vector<AMaterial*> Materialgroup = {};
		Materialgroup.assign(Matnums, nullptr);
		for (int i = 0; i < Matnums; i++)
		{
			Materialgroup[i] = Resourcefactory.CreateMaterial();
			Materialgroup[i]->SetLightColor({ 0.4 ,0.4, 0.4 });

		}
		Resourcefactory.AddMaterial(AObjectmodel, Materialgroup);
		//ObjCB
		AObjectmodel->Transform[0] = 0.001f;
		AObjectmodel->Transform[1] = 0.0f;
		AObjectmodel->Transform[2] = 0.0f;
		AObjectmodel->Rotation[0] = 0.0001f;
		AObjectmodel->Rotation[1] = 0.0f;
		AObjectmodel->Rotation[2] = 0.0f;
		AObjectmodel->Scale[0] = 500.0f;
		AObjectmodel->Scale[1] = 500.0f;
		AObjectmodel->Scale[2] = 500.0f;
		Ars->Skeletongroup.push_back(AObjectmodel);
	}

	{
		AObject_Model* AObjectmodel;
		AResource_Factory Resourcefactory;
		AObjectmodel = Resourcefactory.CreateStaticModel();
		AObjectmodel->caseShadow = false;
		std::vector<ATexture*> Texturegroup = {};
		Resourcefactory.LoadObject(AObjectmodel, Texturegroup, L"FBXResouce\\FBXPlaneWater\\");
		Texturegroup[0]->Type = ATexture::TEXTURE_TYPE::WATERFLOW_TEXTURE;
		Texturegroup[1]->Type = ATexture::TEXTURE_TYPE::WATERVIE_TEXTURE;

		//Material
		int Matnums = AObjectmodel->Materialnums;
		std::vector<AMaterial*> Materialgroup = {};
		Materialgroup.assign(Matnums, nullptr);
		for (int i = 0; i < Matnums; i++)
		{
			Materialgroup[i] = Resourcefactory.CreateMaterial();
			Materialgroup[i]->SetLightColor({ 0.4 ,0.4, 0.4 });
			std::vector<ATexture*> Texturegroupbuffer;
			Texturegroupbuffer.push_back(Texturegroup[0]);
			Texturegroupbuffer.push_back(Texturegroup[1]);
			Resourcefactory.RelateTexturetoMaterial(Materialgroup[i], Texturegroupbuffer);

		}
		Resourcefactory.AddMaterial(AObjectmodel, Materialgroup);
		//ObjCB
		AObjectmodel->Transform[0] = 0.001f;
		AObjectmodel->Transform[1] = 0.001f;
		AObjectmodel->Transform[2] = 0.0f;
		AObjectmodel->Rotation[0] = 0.0001f;
		AObjectmodel->Rotation[1] = 0.0f;
		AObjectmodel->Rotation[2] = 0.0f;
		AObjectmodel->Scale[0] = 100.0f;
		AObjectmodel->Scale[1] = 100.0f;
		AObjectmodel->Scale[2] = 100.0f;
		AObjectmodel->isWater = true;
		Ars->Skeletongroup.push_back(AObjectmodel);
	}

	//Lights
	{
		ALight* light;
		AResource_Factory Resourcefactory;

		light = Resourcefactory.CreateLight();
		light->Lightdata.Direction = { 0.57735f, -0.57735f, 0.57735f };
		//light->Lightdata.Direction = { 0.8f, -0.8f, 0.8f };
		light->Lightdata.Strength = { 0.9f, 0.8f, 0.7f };
		Ars->Lightgroup.push_back(light);

		light = Resourcefactory.CreateLight();
		light->Lightdata.Direction = { -0.57735f, -0.57735f, 0.57735f };
		light->Lightdata.Strength = { 0.4f, 0.4f, 0.4f };
		Ars->Lightgroup.push_back(light);

		light = Resourcefactory.CreateLight();
		light->Lightdata.Direction = { 0.0f, -0.707f, -0.707f };
		light->Lightdata.Strength = { 0.2f, 0.2f, 0.2f };
		Ars->Lightgroup.push_back(light);
	}

	//Camera
	{
		ACamera* Camera;
		AResource_Factory Resourcefactory;
		Camera = Resourcefactory.CreateCamera();
		Camera->SetPosition(0.0f, 2.0f, -15.0f);
		Ars->Cameragroup.push_back(Camera);
	}

	//DebugControl
	LInput_Process Inputcontrol;
	Inputcontrol.SetDebug(true);
	
	return Renderer->Render(Ars, Inputcontrol.GetDebugFlag());

	

}

/// <summary>
/// Enterance process
/// </summary>
void MGame_System::EnteranceSenceProcess()
{
	// 1.读取入口场景
	// 2.读取角色数据
	// 3.展现游戏标题
	// 4.移动(循环)
	// 5.进入武器库(跳转下一流程)
	// 6.进入游戏关卡(跳转下一流程)
	// 7.退出,出口(跳转下一流程)
	// 8.暂停(跳转至UI流程)

}

//Gun store process
void MGame_System::StoreProcess()
{
	// 读取武器库UI
	// 1.鼠标选择或移动(循环)
	// 2.查看武器信息(跳转至UI流程)
	// 3.暂停(跳转至UI流程)
	// 4.离开(跳转下一流程)
	// 5.退出(跳转下一流程)

}


//Sence process
void MGame_System::GameSenceProcess()
{
	// 1.读取场景和第一个关卡
	// 2.移动(循环)
	// 3.伤害计算(循环)
	// 4.进入下一关卡(循环)
	// 5.出口,死亡结算(跳转至UI流程)
	// 6.暂停(跳转至UI流程)
	// 7.退出(跳转下一流程)

}

//Exit process
void MGame_System::ExitGame()
{

}