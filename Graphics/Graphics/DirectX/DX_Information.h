#pragma once
#include "Pre_Define.h"
#include "DirectX/BResource_Heap.h"

//#include "Render/Auxiliary/FrameResource.h"
#include "TMathTool.h"

#include <array>

//读者写者
// 1.DX设备信息 CommandList
// 2.窗口信息 Width Height
// 3.GPU缓存资源 
// 4.最终输出画面 mSwapChainBuffer

class DX_Information
{
public:
    DX_Information() = delete;
    DX_Information(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICommandList);

    static DX_Information* DXInfinstance;

    //need check
    static DX_Information* GetInstance() 
    {
        return DXInfinstance;
    }


    BResource_Heap* Resourceheap = nullptr;
    //FrameResource* mCurrFrameResource = nullptr;

    UINT RtvDescriptorsize = 0;
    UINT DsvDescriptorsize = 0;
    UINT CbvSrvUavDescriptorsize = 0;

    int Clientwidth = 800;
    int Clientheight = 600;



    D3D12_VIEWPORT Screenviewport;//need init
    D3D12_RECT Scissorrect;

    //std::vector<std::unique_ptr<Texture>> mTextures;
    //std::vector< std::unique_ptr<Texture>> mCubeTextures;
    //std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
    //std::unordered_map<std::string, std::vector<std::unique_ptr<Material>>> mMaterials;
    
    UINT material_size = 0;
    UINT render_nums = 0;

    // Set true to use 4X MSAA (?.1.8).  The default is false.
    bool      Msaa4xstate = false;    // 4X MSAA enabled
    UINT      Msaa4xquality = 0;      // quality level of 4X MSAA

    DXGI_FORMAT Rtvformat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT Backbufferformat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT Depthstencilformat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    float Clearcolor[4] = { 0.0,0.0f,0.0f,0.0f };

    XMFLOAT4X4 Shadowtransform = MathHelper::Identity4x4();//should be fixed in better way to transform

    //Camera mCamera;

    //std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

public:
    void OnResize();

    //void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

    const D3D12_VIEWPORT* const  GetScreenViewport()const;
    const D3D12_RECT* const  GetScissorRect()const;

    void SetPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7Ui64> GetStaticSamplers();

};