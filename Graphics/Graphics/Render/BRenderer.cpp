#ifdef  DLL_GRAPHICS_API
#else
#define DLL_GRAPHICS_API _declspec(dllexport)
#endif
#include "Render/BRenderer.h"

void DebugOnMouseDown(WPARAM btnState, int x, int y);
void DebugOnMouseUp(WPARAM btnState, int x, int y);
void DebugOnMouseMove(WPARAM btnState, int x, int y);

void BRenderer::CreateWindows(HINSTANCE IHINSTANCE, UINT IWidth, UINT IHeight)
{
	WindowInstance = BWindow::GetInstance();
	WindowInstance->InitMainWindow(IHINSTANCE, IWidth, IHeight);
	WindowInstance->SetConctrolFunction(DebugOnMouseDown, DebugOnMouseUp, DebugOnMouseMove);
}

void  BRenderer::RenderInitialize()
{
	//!
	//CreateWindows();

	HWND Hwnd = WindowInstance->GetHWND();
	int width = WindowInstance->GetWidth();
	int height = WindowInstance->GetHeight();
	DXInstance = new DX_App(Hwnd);
	DXInstance->Initialize(width, height);

	//�����Ⱦ��Ϣ
	DXInfinstance = new DX_Information(DXInstance->GetDevice(), DXInstance->GetCommandList());

	DFInstance = new BDeferredRendering(DXInstance->GetDevice(), DXInstance->GetCommandList());

	Timer.Reset();
}

//���ڳ�ʼ��
void BRenderer::RenderInitialize(HINSTANCE IHINSTANCE, UINT IWidth, UINT IHeight)
{
	//�е��ң�
	CreateWindows(IHINSTANCE,IWidth,IHeight);
	HWND Hwnd = WindowInstance->GetHWND();
	int width = WindowInstance->GetWidth();
	int height = WindowInstance->GetHeight();
	DXInstance = new DX_App(Hwnd);
	DXInstance->Initialize(width, height);

	ID3D12GraphicsCommandList* CmdList = DXInstance->GetCommandList();
	ID3D12Device* Device = DXInstance->GetDevice();
	ID3D12CommandAllocator* CmdAllocator = DXInstance->GetCommandAllocator();

	//�����Ⱦ��Ϣ
	DXInfinstance = new DX_Information(Device, CmdList);
	DFInstance = new BDeferredRendering(Device, CmdList);

	DXInstance->ExecuteCommandList();
	DXInstance->FlushCommandQueue();

	DFInstance->Initialize(Device, CmdList);

	Timer.Reset();

}

/// <summary>
/// ��CPU����װ��GPU
/// �ٵ���Render
/// </summary>
/// <param name="IRenderscene"></param>
int BRenderer::Render(ARender_Scene* IRenderscene, bool IsDebug)
{
	DebugFlag = IsDebug;
	MSG msg = { 0 };
	Timer.Reset();
	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			Render((RRender_Scene*)IRenderscene);
		}
	}

	return (int)msg.wParam;
	//???ǿ��ת����

}

/// <summary>
/// ��Ⱦ��ͳ�����з������ƻ���
/// 3ά��Ⱦ
/// UI����
/// Ϊ������Դ�󶨷���
/// </summary>
/// <param name="IRenderscene">������Դ</param>
void BRenderer::Render(RRender_Scene* IRenderscene)
{
	Timer.Tick();

	CalculateFrameStats();

	DXInstance->UpdateFence();
	DXInstance->ResetCommandAllocator();

	//����״̬���
	switch (CheckenderingResource(IRenderscene))
	{
	case RENDER_SCENE_STATE::CHANGE_SCENE://��������
		//ж�ص�ǰ����
		UnLoadRenderingResource();
		//ע�ᵱǰ������Դ
		LoadRenderingResource(IRenderscene);
		break;
	case RENDER_SCENE_STATE::UPDATE_SCENE://ԭ�������ݱ仯
		UnLoadRenderingResource();
		break;
	case RENDER_SCENE_STATE::UNCHANGE_SCENE://�ޱ仯
		break;
	};

	BGPU_Resource_Factory GPUResourcefactory;

	//�����������ˢ��
	if (DebugFlag)
	{
		DebugKeyboardInput();
	}
	else
	{
		RCamera* Camera = (RCamera*)Rendersence->Cameragroup[0];
		Camera->UpdateViewMatrix();
	}
	//������� �ƹ��GPU����ˢ��
	GPUResourcefactory.UpdateGPUScene(IRenderscene);


	//����ˢ��
	AniamtionUpdate(IRenderscene);


	
	

	//
	//3ά��Ⱦ
	DFInstance->Render(DXInstance->GetDevice(),
		DXInstance->GetCommandList(),
		Rendersence,
		DXInstance->CurrentBackBuffer(),
		DXInstance->CurrentBackBufferView());

	DXInstance->ExecuteCommandList();
	DXInstance->SwitchBackBufferFence();

}

//����Ҫ��Ҫ��anime����
void BRenderer::AniamtionUpdate(RRender_Scene* IRenderscene)
{
	auto skeletongroup = *IRenderscene->GetSkeletonGroup();
	BAnimator_Factory Animatorfactory;
	BGPU_Resource_Factory GPUResourcefactory;

	for (int i = 0; i < skeletongroup.size(); i++)
	{
		Animatorfactory.AnimationCPUUpdate((RSkeleton_Model*)skeletongroup[i], Timer);

		GPUResourcefactory.UpdateGPUVertexCB((RSkeleton_Model*)skeletongroup[i]);
	}
}


RENDER_SCENE_STATE BRenderer::CheckenderingResource(const RRender_Scene* IRenderscene)
{
	if (Rendersence == nullptr || Rendersence->GetID() != IRenderscene->GetID())
	{
		return RENDER_SCENE_STATE::CHANGE_SCENE;
	}
	else// if ()????��̬ˢ��
	{
		return RENDER_SCENE_STATE::UNCHANGE_SCENE;
	}
}

//�ؿ���Դ��ȡ��GPU
void BRenderer::LoadRenderingResource(RRender_Scene* IRendersence)
{
	if (Rendersence)
	{
		UnLoadRenderingResource();
	}
	Rendersence = IRendersence;

	BGPU_Resource_Factory GPUResourcefactory;
	//���س�������
	GPUResourcefactory.AssignGPUScene(DXInstance->GetDevice(),IRendersence);
	//���س�������
	GPUResourcefactory.AssignGPUObjects(DXInstance->GetDevice(), DXInstance->GetCommandList(),IRendersence);
	//....

}

void BRenderer::UpdateGPUResource()
{

}

//�ͷŹؿ���Դ
void BRenderer::UnLoadRenderingResource()
{
	//...
}



////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////DebugControl////////////////////////////////////////////////////////
/////////////////////////DebugControl///////////////////////////////////////////////////////////
///////////////////////DebugControl////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
POINT mLastMousePos;
float dx = 0;
float dy = 0;
int IsMouseDown = 0;
bool IsMouseMove = false;
void DebugOnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	IsMouseDown = 1;
	
}

void DebugOnMouseUp(WPARAM btnState, int x, int y)
{
	IsMouseDown = 0;
	ReleaseCapture();
}

void DebugOnMouseMove(WPARAM btnState, int x, int y)
{
	IsMouseMove = true;
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void BRenderer::DebugKeyboardInput()
{
	const float dt = Timer.DeltaTime();

	RCamera* Camera = (RCamera*)Rendersence->Cameragroup[0];

	if (GetAsyncKeyState('W') & 0x8000)
		Camera->Walk(10.0f * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		Camera->Walk(-10.0f * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		Camera->Strafe(-10.0f * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		Camera->Strafe(10.0f * dt);

	if (IsMouseDown == 1)
	{
		SetCapture(WindowInstance->GetHWND());
		IsMouseDown++;
	}
	else if(IsMouseDown > 1 && IsMouseMove)
	{
		Camera->Pitch(dy);
		Camera->RotateY(dx);
		IsMouseMove = false;
	}
	else
	{
		dx = 0;
		dy = 0;
	}

	Camera->UpdateViewMatrix();
}

void BRenderer::CalculateFrameStats()
{

	using std::wstring;
	using std::to_wstring;

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((Timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);

		wstring windowText = mMainWndCaption +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr;

		SetWindowText(WindowInstance->GetHWND(), windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}
