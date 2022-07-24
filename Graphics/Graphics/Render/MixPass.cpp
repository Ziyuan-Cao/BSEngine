#include "MixPass.h"

void MixPass::Draw(ID3D12Device* IDevice,
    ID3D12GraphicsCommandList* ICmdList,
    RRender_Scene* IRenderscene,
    ID3D12Resource* ORendertarget,
    D3D12_CPU_DESCRIPTOR_HANDLE ORendertargetView)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    CD3DX12_RESOURCE_BARRIER Currentbackbarrier = CD3DX12_RESOURCE_BARRIER::Transition(ORendertarget,
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    ICmdList->ResourceBarrier(1, &Currentbackbarrier);

    //???
    ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap };
    ICmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    ICmdList->SetGraphicsRootSignature(RootSignature);

    ICmdList->SetPipelineState(PSOs["Mix"]);

    ICmdList->SetGraphicsRootConstantBufferView(0, DXInf->Resourceheap->GetResource("mViewCB")->GetGPUVirtualAddress());

    auto passCB = IRenderscene->GetSceneConstantsGPU();
    ICmdList->SetGraphicsRootConstantBufferView(1, passCB->Resource()->GetGPUVirtualAddress());

    //???
    //auto matBuffer = d3dApp->mCurrFrameResource->MaterialBuffer->Resource();
    //ICmdList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());
    
    CD3DX12_GPU_DESCRIPTOR_HANDLE depthDescriptor(SRVHeap->GetGPUDescriptorHandleForHeapStart());
    depthDescriptor.Offset(DepthIndex, DXInf->CbvSrvUavDescriptorsize);
    ICmdList->SetGraphicsRootDescriptorTable(3, depthDescriptor);

    //CD3DX12_GPU_DESCRIPTOR_HANDLE profileDescriptor(mSRVHeap->GetGPUDescriptorHandleForHeapStart());
    //profileDescriptor.Offset(mProfileIndex, DXInf->CbvSrvUavDescriptorsize);
    //ICmdList->SetGraphicsRootDescriptorTable(4, profileDescriptor);

    CD3DX12_GPU_DESCRIPTOR_HANDLE LightDescriptor(SRVHeap->GetGPUDescriptorHandleForHeapStart());
    LightDescriptor.Offset(LightIndex, DXInf->CbvSrvUavDescriptorsize);
    ICmdList->SetGraphicsRootDescriptorTable(5, LightDescriptor);

    ICmdList->SetGraphicsRootDescriptorTable(6, SRVHeap->GetGPUDescriptorHandleForHeapStart());

    ICmdList->ClearRenderTargetView(ORendertargetView, Colors::LightSteelBlue, 0, nullptr);
    D3D12_CPU_DESCRIPTOR_HANDLE Currentbackbufferview = ORendertargetView;
    ICmdList->OMSetRenderTargets(1, &Currentbackbufferview, true, nullptr);

    ICmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    ICmdList->IASetVertexBuffers(0, 1, &VbView);
    ICmdList->DrawInstanced(4, 1, 0, 0);

    Currentbackbarrier = CD3DX12_RESOURCE_BARRIER::Transition(ORendertarget,
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    ICmdList->ResourceBarrier(1, &Currentbackbarrier);

}


void MixPass::VertexsAndIndexesInput()
{
    DX_Information* DXInf = DX_Information::GetInstance();

    struct ScreenQuadVertex
    {
        DirectX::XMFLOAT4 position;
        DirectX::XMFLOAT2 texcoord;
    };

    ScreenQuadVertex QuadVerts[] =
    {
        { { -1.0f,1.0f, 0.0f,1.0f },{ 0.0f,0.0f } },
        { { 1.0f, 1.0f, 0.0f,1.0f }, {1.0f,0.0f } },
        { { -1.0f, -1.0f, 0.0f,1.0f },{ 0.0f,1.0f } },
    { { 1.0f, -1.0f, 0.0f,1.0f },{ 1.0f,1.0f } }
    };

    //mVB was created in lightPass
    VbView.BufferLocation = DXInf->Resourceheap->GetResource("mVB")->GetGPUVirtualAddress();
    VbView.StrideInBytes = sizeof(ScreenQuadVertex);
    VbView.SizeInBytes = sizeof(QuadVerts);
}

void MixPass::BuildDescriptorHeaps(ID3D12Device* IDevice)
{
    //create SRV DescriptorHeap
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {}; //Gbuffer + DepthSentil + profile + light
    srvHeapDesc.NumDescriptors = GBufferRTCount + 1 + 1 + 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(IDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&SRVHeap)));
}

void MixPass::CreateDescriptors(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();
    //fill out the heap with descriptors
    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(SRVHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < GBufferRTCount; i++) {
        IDevice->CreateShaderResourceView(
            DXInf->Resourceheap->GetResource("GBuffer" + std::to_string(i)),
            DXInf->Resourceheap->GetSRVDesc("GBuffer" + std::to_string(i)),
            hDescriptor); 
        hDescriptor.Offset(1, DXInf->CbvSrvUavDescriptorsize);
    }

    //??
    //DepthIndex = GBufferRTCount;
    //ProfileIndex = DepthIndex + 1;
    //LightIndex = ProfileIndex + 1;

    //IDevice->CreateShaderResourceView(
    //    DXInf->Resourceheap->GetResource("DepthStencilBuffer"),
    //    DXInf->Resourceheap->GetSRVDesc("DepthStencilBuffer"),
    //    hDescriptor);
    //hDescriptor.Offset(1, DXInf->CbvSrvUavDescriptorsize);

    //IDevice->CreateUnorderedAccessView(
    //    DXInf->Resourceheap->GetResource("PostProfileMap"),
    //    nullptr,
    //    DXInf->Resourceheap->GetUAVDesc("PostProfileMap"),
    //    hDescriptor);
    //hDescriptor.Offset(1, DXInf->CbvSrvUavDescriptorsize);

    //IDevice->CreateShaderResourceView(
    //    DXInf->Resourceheap->GetResource("LightMap"),
    //    DXInf->Resourceheap->GetSRVDesc("LightMap"),
    //    hDescriptor);

    DepthIndex = GBufferRTCount;
    LightIndex = DepthIndex + 1;

    IDevice->CreateShaderResourceView(
        DXInf->Resourceheap->GetResource("DepthStencilBuffer"),
        DXInf->Resourceheap->GetSRVDesc("DepthStencilBuffer"),
        hDescriptor);
    hDescriptor.Offset(1, DXInf->CbvSrvUavDescriptorsize);

    IDevice->CreateShaderResourceView(
        DXInf->Resourceheap->GetResource("LightMap"),
        DXInf->Resourceheap->GetSRVDesc("LightMap"),
        hDescriptor);
}

void MixPass::BuildHeaps(ID3D12GraphicsCommandList* ICmdList)
{

}

void MixPass::BuildRootSignature(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    CD3DX12_DESCRIPTOR_RANGE range[4];
    range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
    range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    range[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, GBufferRTCount, 2);
    CD3DX12_ROOT_PARAMETER rootParameters[7];
    rootParameters[0].InitAsConstantBufferView(0);
    rootParameters[1].InitAsConstantBufferView(1);
    rootParameters[2].InitAsShaderResourceView(0, 1);//gMaterialData : register(t0, space1);
    rootParameters[3].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[4].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[5].InitAsDescriptorTable(1, &range[2], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[6].InitAsDescriptorTable(1, &range[3], D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = DXInf->GetStaticSamplers();

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
    descRootSignature.Init(7,
        rootParameters,
        (UINT)staticSamplers.size(),
        staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ID3DBlob* rootSigBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    ThrowIfFailed(D3D12SerializeRootSignature(
        &descRootSignature, 
        D3D_ROOT_SIGNATURE_VERSION_1, 
        &rootSigBlob, 
        &errorBlob));

    ThrowIfFailed(IDevice->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));

}

void MixPass::BuildShadersAndInputLayout()
{
    Shaders["MixVS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\MixPass.hlsl", nullptr, "VS", "vs_5_1");
    Shaders["MixPS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\MixPass.hlsl", nullptr, "PS", "ps_5_1");

    InputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void MixPass::BuildPSO(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
    ZeroMemory(&descPipelineState, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    descPipelineState.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
    descPipelineState.pRootSignature = RootSignature;
    descPipelineState.VS =
    {
        reinterpret_cast<BYTE*>(Shaders["MixVS"]->GetBufferPointer()),
        Shaders["MixVS"]->GetBufferSize()
    };
    descPipelineState.PS =
    {
        reinterpret_cast<BYTE*>(Shaders["MixPS"]->GetBufferPointer()),
        Shaders["MixPS"]->GetBufferSize()
    };
    descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    descPipelineState.DepthStencilState.DepthEnable = false;
    descPipelineState.SampleMask = UINT_MAX;
    descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    descPipelineState.NumRenderTargets = 1;
    //descPipelineState.RTVFormats[0] = mRtvFormat[0];
    descPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    descPipelineState.SampleDesc.Count = 1;
    DXInf->SetPSO(descPipelineState);
    ThrowIfFailed(IDevice->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&PSOs["Mix"])));

}



//void MixPass::Update(const GameTimer& gt)
//{
//
//}