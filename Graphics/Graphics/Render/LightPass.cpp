#include "LightPass.h"

void LightPass::VertexsAndIndexesInput()
{
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

    DX_Information* DXInf = DX_Information::GetInstance();

    D3D12_RESOURCE_DESC resourceDesc;
    ZeroMemory(&resourceDesc, sizeof(resourceDesc));
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Width = sizeof(QuadVerts);
    resourceDesc.Height = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    DXInf->Resourceheap->AddResource("mVB",
        resourceDesc, 
        QuadVerts,
        sizeof(QuadVerts),
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_RESOURCE_STATE_GENERIC_READ);

    VbView.BufferLocation = DXInf->Resourceheap->GetResource("mVB")->GetGPUVirtualAddress();
    VbView.StrideInBytes = sizeof(ScreenQuadVertex);
    VbView.SizeInBytes = sizeof(QuadVerts);
}

void LightPass::BuildDescriptorHeaps(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    //create SRV DescriptorHeap
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {}; //Gbuffer + DepthSentil
    srvHeapDesc.NumDescriptors = GBufferRTCount + 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(IDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&SRVHeap)));

    //RTV
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(IDevice->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(&RTVHeap)));

}

void LightPass::CreateDescriptors(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    //fill out the heap with descriptors
    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(SRVHeap->GetCPUDescriptorHandleForHeapStart());

    D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;
    ZeroMemory(&descSRV, sizeof(descSRV));
    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    for (int i = 0; i < GBufferRTCount; i++) {
        descSRV.Format = DXInf->Resourceheap->GetFormat("GBuffer" + std::to_string(i));
        IDevice->CreateShaderResourceView(
            DXInf->Resourceheap->GetResource("GBuffer" + std::to_string(i)),
            DXInf->Resourceheap->GetSRVDesc("GBuffer" + std::to_string(i)),
            hDescriptor);
        hDescriptor.Offset(1, DXInf->CbvSrvUavDescriptorsize);
    }

    mDepthIndex = GBufferRTCount;

    IDevice->CreateShaderResourceView(
        DXInf->Resourceheap->GetResource("DepthStencilBuffer"),
        DXInf->Resourceheap->GetSRVDesc("DepthStencilBuffer"),
        hDescriptor);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart());

    IDevice->CreateRenderTargetView(
        DXInf->Resourceheap->GetResource("LightMap"),
        DXInf->Resourceheap->GetRTVDesc("LightMap"),
        rtvHeapHandle);
}

void LightPass::BuildHeaps(ID3D12GraphicsCommandList* ICmdList)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    CreateCB();
    
    D3D12_RESOURCE_DESC resourceDesc;
    ZeroMemory(&resourceDesc, sizeof(resourceDesc));
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.MipLevels = 1;

    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Width = DXInf->GetWClientWidth();
    resourceDesc.Height = DXInf->GetWClientHeight();
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_CLEAR_VALUE clearVal;
    clearVal.Color[0] = DXInf->Clearcolor[0];
    clearVal.Color[1] = DXInf->Clearcolor[1];
    clearVal.Color[2] = DXInf->Clearcolor[2];
    clearVal.Color[3] = DXInf->Clearcolor[3];

    resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    clearVal.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    DXInf->Resourceheap->AddResource("LightMap",
        resourceDesc,
        nullptr,
        0,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_PRESENT,
        &clearVal);

    D3D12_RENDER_TARGET_VIEW_DESC descRTV;
    ZeroMemory(&descRTV, sizeof(descRTV));
    descRTV.Texture2D.MipSlice = 0;
    descRTV.Texture2D.PlaneSlice = 0;
    descRTV.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    descRTV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXInf->Resourceheap->CreateRTVDesc("LightMap", descRTV);

    D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;
    ZeroMemory(&descSRV, sizeof(descSRV));
    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descSRV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXInf->Resourceheap->CreateSRVDesc("LightMap", descSRV);


}

void LightPass::BuildRootSignature(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    CD3DX12_DESCRIPTOR_RANGE descriptorRange[2];
    descriptorRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    descriptorRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, GBufferRTCount, 1,1);

    CD3DX12_ROOT_PARAMETER rootParameters[4];
    //rootParameters[0].InitAsConstantBufferView(0); //MVB
    rootParameters[0].InitAsConstantBufferView(0); //Senceconstant
    rootParameters[1].InitAsShaderResourceView(0,1); //Light
    //rootParameters[2].InitAsShaderResourceView(0, 1);//gMaterialData : register(t0, space1);
    rootParameters[2].InitAsDescriptorTable(1, &descriptorRange[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[3].InitAsDescriptorTable(1, &descriptorRange[1], D3D12_SHADER_VISIBILITY_PIXEL);
    auto staticSamplers = DXInf->GetStaticSamplers();

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
    descRootSignature.Init(4,
        rootParameters,
        (UINT)staticSamplers.size(),
        staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ID3DBlob* rootSigBlob;
    ID3DBlob* errorBlob;

    ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob));

    ThrowIfFailed(IDevice->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));

}

void LightPass::BuildShadersAndInputLayout()
{
    Shaders["LightVS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\LightPass.hlsl", nullptr, "VS", "vs_5_1");
    Shaders["LightPS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\LightPass.hlsl", nullptr, "PS", "ps_5_1");
       
    InputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void LightPass::BuildPSO(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
    ZeroMemory(&descPipelineState, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    descPipelineState.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
    descPipelineState.pRootSignature = RootSignature;
    descPipelineState.VS =
    {
        reinterpret_cast<BYTE*>(Shaders["LightVS"]->GetBufferPointer()),
        Shaders["LightVS"]->GetBufferSize()
    };
    descPipelineState.PS =
    {
        reinterpret_cast<BYTE*>(Shaders["LightPS"]->GetBufferPointer()),
        Shaders["LightPS"]->GetBufferSize()
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
    //descPipelineState.SampleDesc.Count = 1;
    DXInf->SetPSO(descPipelineState);
    ThrowIfFailed(IDevice->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&PSOs["light"])));

}

void LightPass::CreateCB()
{
    //DX_Information* DXInf = DX_Information::GetInstance();

    ////CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_UPLOAD);
    //D3D12_RESOURCE_DESC resourceDesc;
    //ZeroMemory(&resourceDesc, sizeof(resourceDesc));
    //resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    //resourceDesc.Alignment = 0;
    //resourceDesc.SampleDesc.Count = 1;
    //resourceDesc.SampleDesc.Quality = 0;
    //resourceDesc.MipLevels = 1;
    //resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    //resourceDesc.DepthOrArraySize = 1;
    //resourceDesc.Width = sizeof(RCamera::CameraData);
    //resourceDesc.Height = 1;
    //resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    //
    //DXInf->Resourceheap->AddResource("mViewCB",
    //    resourceDesc,
    //    &Cameradata,
    //    sizeof(RCamera::CameraData),
    //    D3D12_HEAP_TYPE_UPLOAD,
    //    D3D12_RESOURCE_STATE_GENERIC_READ);

}

void LightPass::UpdateConstantBuffer()
{
    //DX_Information* DXInf = DX_Information::GetInstance();

    //void* mapped = nullptr;
    //DXInf->Resourceheap->GetResource("mViewCB")->Map(0, nullptr, &mapped);
    //memcpy(mapped, &Cameradata, sizeof(RCamera::CameraData));
    //DXInf->Resourceheap->GetResource("mViewCB")->Unmap(0, nullptr);
}

void LightPass::Update()
{
    UpdateConstantBuffer();
}


void LightPass::Draw(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList, RRender_Scene* IRenderscene)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    CD3DX12_RESOURCE_BARRIER Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(DXInf->Resourceheap->GetResource("LightMap"),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    ICmdList->ResourceBarrier(1, &Resourcebarrier);

    ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap };

    ICmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    ICmdList->SetGraphicsRootSignature(RootSignature);

    ICmdList->SetPipelineState(PSOs["light"]);

    //ICmdList->SetGraphicsRootConstantBufferView(0, DXInf->Resourceheap->GetResource("mViewCB")->GetGPUVirtualAddress());

    //SenceCB
    auto passCB = IRenderscene->GetSceneConstantsGPU();
    ICmdList->SetGraphicsRootConstantBufferView(0, passCB->Resource()->GetGPUVirtualAddress());

    //Light
    auto LightCB = IRenderscene->GetLightsGPU();
    ICmdList->SetGraphicsRootShaderResourceView(1, LightCB->Resource()->GetGPUVirtualAddress());

    //auto matBuffer = DXInf->mCurrFrameResource->MaterialBuffer->Resource();
    //ICmdList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());

    CD3DX12_GPU_DESCRIPTOR_HANDLE depthDescriptor(SRVHeap->GetGPUDescriptorHandleForHeapStart());
    depthDescriptor.Offset(mDepthIndex, DXInf->CbvSrvUavDescriptorsize);
    ICmdList->SetGraphicsRootDescriptorTable(2, depthDescriptor);

    ICmdList->SetGraphicsRootDescriptorTable(3, SRVHeap->GetGPUDescriptorHandleForHeapStart());


    ICmdList->ClearRenderTargetView(RTVHeap->GetCPUDescriptorHandleForHeapStart(), DXInf->Clearcolor, 0, nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE CPUDeschandle = RTVHeap->GetCPUDescriptorHandleForHeapStart();

    ICmdList->OMSetRenderTargets(1, &CPUDeschandle, true, nullptr);

    ICmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    ICmdList->IASetVertexBuffers(0, 1, &VbView);
    ICmdList->DrawInstanced(4, 1, 0, 0);

    Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(DXInf->Resourceheap->GetResource("LightMap"),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    ICmdList->ResourceBarrier(1, &Resourcebarrier);

}

