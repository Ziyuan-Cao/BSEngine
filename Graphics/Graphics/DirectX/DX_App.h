#pragma once


#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include <comdef.h>

#include "Pre_Define.h"

//1.搭建驱动交互接口（只暴露必要接口）
//2.交换链
//3.提供命令列表，分配器
//4.每帧填充CurrentBackBuffer()
class DX_App
{
protected:
	
	DX_App(const DX_App& rhs) = delete;
	DX_App& operator=(const DX_App& rhs) = delete;
	virtual ~DX_App();

public:
	ENGINEDLL_API DX_App(HWND hwnd);
	static ENGINEDLL_API DX_App* GetApp();

	HWND      MainWnd()const;
	float     AspectRatio()const;

	ID3D12Device* GetDevice() { return md3dDevice; };
	ID3D12GraphicsCommandList* GetCommandList() { return mCommandList; };
	ID3D12CommandAllocator* GetCommandAllocator() { return mDirectCmdListAlloc; };
	bool Get4xMsaaState()const;
	void Set4xMsaaState(bool value);

	int Run();

	virtual ENGINEDLL_API bool Initialize(int IWidth, int IHeight);

	ID3D12Resource* CurrentBackBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;

	void ExecuteCommandList();
	void FlushCommandQueue();
	void ResetCommandAllocator();
	void SwitchBackBufferFence();
	void UpdateFence();

protected:
	virtual void CreateSwapRtvDescriptorHeaps();
	virtual void OnResize(int IWidth, int IHeight);
	virtual void SwapResize(int IWidth, int IHeight);
	virtual void ScreenViewportResize(int IWidth, int IHeight);


	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

	bool InitDirect3D();
	void CreateCommandObjects();
	void CreateSwapChain();

	//void CalculateFrameStats();

	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

protected:

	static DX_App* mApp;

	HWND      mhMainWnd = nullptr; // main window handle


	// Set true to use 4X MSAA (?.1.8).  The default is false.
	bool      m4xMsaaState = false;    // 4X MSAA enabled
	UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

	// Used to keep track of the 揹elta-time?and game time (?.4).

	IDXGIFactory4* mdxgiFactory;
	IDXGISwapChain* mSwapChain;
	ID3D12Device* md3dDevice;

	ID3D12Fence* mFence;
	UINT64 mCurrentFence = 0;

	ID3D12CommandQueue* mCommandQueue;
	ID3D12CommandAllocator* mDirectCmdListAlloc;
	ID3D12GraphicsCommandList* mCommandList;

	static const int SwapChainBufferCount = 2;
	int mCurrBackBuffer = 0;
	ID3D12Resource* mSwapChainBuffer[SwapChainBufferCount];


	ID3D12DescriptorHeap* mRtvHeap;


	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	UINT mRtvDescriptorSize = 0;

	UINT mCbvSrvUavDescriptorSize = 0;

	// Derived class should set these in derived constructor to customize starting values.
	
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	int mClientWidth = 800;
	int mClientHeight = 600;

	//DEBUG
	UINT64 CFence = 0;
};

