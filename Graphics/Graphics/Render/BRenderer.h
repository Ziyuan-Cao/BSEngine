#pragma once
#include "BGraphics.h"
#include "DirectX/DX_App.h"
#include "Render/BDeferredRendering.h"
#include "Render/BAnimator_Factory.h"
#include "Window/BWindow.h"
#include "Window/BProcess.h"



class BRenderer : public ARenderer
{
public:

	virtual void CreateWindows(HINSTANCE IHINSTANCE, UINT IWidth, UINT IHeight);
	//进入程序调用
	virtual void RenderInitialize() override;
	virtual void RenderInitialize(HINSTANCE IHINSTANCE, UINT IWidth, UINT IHeight) override;
	virtual void RenderInitialize(HWND IHWND, UINT IWidth, UINT IHeight) override;


	virtual int Render(ARender_Scene* IRenderscene, bool IsDebug = true) override;

	//每帧调用
	void Render(RRender_Scene * IRenderscene);
	void AniamtionUpdate(RRender_Scene* IRenderscene);

	RENDER_SCENE_STATE CheckenderingResource(const RRender_Scene* IRenderscene);

	//关卡资源读取入GPU
	void LoadRenderingResource(RRender_Scene* IRendersence);

	//释放关卡资源
	virtual void UnLoadRenderingResource() override;

	virtual void UpdateGPUResource() override;

	//Debug
	void DebugKeyboardInput();
	void CalculateFrameStats();
	//Debug


private:
	BProcess* ProcessInstance;
	BWindow* WindowInstance;

	DX_Information* DXInfinstance;

	DX_App* DXInstance;

	BDeferredRendering* DFInstance;

	//当前绑定的场景
	RRender_Scene* Rendersence;

	bool DebugFlag = false;
	TTimer Timer;
	std::wstring mMainWndCaption = L"Graphic test";
};

