#include "SSAOPass.h"
#include <ctime>

void SSAOPass::OnResize(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList)
{
    if (!Init)
    {
        BuildNoiseMap(IDevice,ICmdList);
    }
    VertexsAndIndexesInput();
    BuildHeaps(ICmdList);
    BuildDescriptorHeaps(IDevice);
    CreateDescriptors(IDevice);

    Init = true;
}


void SSAOPass::VertexsAndIndexesInput()
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


void SSAOPass::BuildDescriptorHeaps(ID3D12Device* IDevice)
{
    //create SRV DescriptorHeap
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {}; //Gbuffer + DepthSentil + NoiseMap
    srvHeapDesc.NumDescriptors = GBufferRTCount + 1  + 1 ;
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

void SSAOPass::CreateDescriptors(ID3D12Device* IDevice)
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

    DepthIndex = GBufferRTCount;
    NoiseIndex = DepthIndex + 1;

    IDevice->CreateShaderResourceView(
        DXInf->Resourceheap->GetResource("DepthStencilBuffer"),
        DXInf->Resourceheap->GetSRVDesc("DepthStencilBuffer"),
        hDescriptor);
    hDescriptor.Offset(1, DXInf->CbvSrvUavDescriptorsize);

    IDevice->CreateShaderResourceView(
        NoiseMapGPU,
        &NoiseMapdescSRV,
        hDescriptor);
    hDescriptor.Offset(1, DXInf->CbvSrvUavDescriptorsize);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart());

    IDevice->CreateRenderTargetView(
        DXInf->Resourceheap->GetResource("SSAOMap"),
        DXInf->Resourceheap->GetRTVDesc("SSAOMap"),
        rtvHeapHandle);

}

void SSAOPass::BuildNoiseMap(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList)
{
    float noiseTextureFloats[192];

    srand(time(0));

    for (int i = 0; i < 64; i++)
    {
        int Rnd1 = rand();
        int Rnd2 = rand();
        int Rnd3 = rand();
        float R0 = float(Rnd1 % (2)) - 1;
        float R1 = float(Rnd1 % (2)) - 1;

        int index = i * 3;
        noiseTextureFloats[index] = R0;
        noiseTextureFloats[index + 1] = R1;
        noiseTextureFloats[index + 2] = 0.0f;
    }

    NoiseMapGPUSUBRESOURCE.pData = noiseTextureFloats;
    NoiseMapGPUSUBRESOURCE.RowPitch = 8 * 12;
    NoiseMapGPUSUBRESOURCE.SlicePitch = 0;

    D3D12_RESOURCE_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 1D  2D 3D
    texDesc.Alignment = 0;
    texDesc.Width = 8;
    texDesc.Height = 8;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT; // FORMAT
    texDesc.DepthOrArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES Heapproperties(D3D12_HEAP_TYPE_DEFAULT);
    IDevice->CreateCommittedResource(
        &Heapproperties,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&NoiseMapGPU)
    );

    

    const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(NoiseMapGPU, 0, num2DSubresources);

    CD3DX12_HEAP_PROPERTIES Heapproperties_upload(D3D12_HEAP_TYPE_UPLOAD);

    CD3DX12_RESOURCE_DESC Resourcedescbuffer = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

    IDevice->CreateCommittedResource(
        &Heapproperties_upload,
        D3D12_HEAP_FLAG_NONE,
        &Resourcedescbuffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadHeap));

    CD3DX12_RESOURCE_BARRIER Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(NoiseMapGPU,
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

    ICmdList->ResourceBarrier(1, &Resourcebarrier);

    // Use Heap-allocating UpdateSubresources implementation for variable number of subresources (which is the case for textures).
    UpdateSubresources(ICmdList, NoiseMapGPU, textureUploadHeap, 0, 0, num2DSubresources, &NoiseMapGPUSUBRESOURCE);

    Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(NoiseMapGPU,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    ICmdList->ResourceBarrier(1, &Resourcebarrier);

    ZeroMemory(&NoiseMapdescSRV, sizeof(NoiseMapdescSRV));
    NoiseMapdescSRV.Texture2D.MipLevels = 1;
    NoiseMapdescSRV.Texture2D.MostDetailedMip = 0;
    NoiseMapdescSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // srv FORMAT
    NoiseMapdescSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    NoiseMapdescSRV.Format = texDesc.Format;

}

void SSAOPass::BuildHeaps(ID3D12GraphicsCommandList* ICmdList)
{

    DX_Information* DXInf = DX_Information::GetInstance();

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

    DXInf->Resourceheap->AddResource("SSAOMap",
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
    DXInf->Resourceheap->CreateRTVDesc("SSAOMap", descRTV);

    D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;
    ZeroMemory(&descSRV, sizeof(descSRV));
    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descSRV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXInf->Resourceheap->CreateSRVDesc("SSAOMap", descSRV);
}

void SSAOPass::BuildRootSignature(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();
    
    CD3DX12_DESCRIPTOR_RANGE range[3];
    range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    
    range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, GBufferRTCount, 2);

    CD3DX12_ROOT_PARAMETER rootParameters[4];
    rootParameters[0].InitAsConstantBufferView(0);
    rootParameters[1].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[2].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[3].InitAsDescriptorTable(1, &range[2], D3D12_SHADER_VISIBILITY_PIXEL);

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

void SSAOPass::BuildShadersAndInputLayout()
{
    Shaders["SSAOVS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\SSAOPass.hlsl", nullptr, "VS", "vs_5_1");
    Shaders["SSAOPS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\SSAOPass.hlsl", nullptr, "PS", "ps_5_1");

    InputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void SSAOPass::BuildPSO(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
    ZeroMemory(&descPipelineState, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    descPipelineState.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
    descPipelineState.pRootSignature = RootSignature;
    descPipelineState.VS =
    {
        reinterpret_cast<BYTE*>(Shaders["SSAOVS"]->GetBufferPointer()),
        Shaders["SSAOVS"]->GetBufferSize()
    };
    descPipelineState.PS =
    {
        reinterpret_cast<BYTE*>(Shaders["SSAOPS"]->GetBufferPointer()),
        Shaders["SSAOPS"]->GetBufferSize()
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
    ThrowIfFailed(IDevice->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&PSOs["SSAO"])));

}


void SSAOPass::Draw(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList, RRender_Scene* IRenderscene)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    CD3DX12_RESOURCE_BARRIER Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(DXInf->Resourceheap->GetResource("SSAOMap"),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    ICmdList->ResourceBarrier(1, &Resourcebarrier);

    ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap };

    ICmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    ICmdList->SetGraphicsRootSignature(RootSignature);

    ICmdList->SetPipelineState(PSOs["SSAO"]);

    auto passCB = IRenderscene->GetSceneConstantsGPU();
    ICmdList->SetGraphicsRootConstantBufferView(0, passCB->Resource()->GetGPUVirtualAddress());

    //GBUFFER
    ICmdList->SetGraphicsRootDescriptorTable(3, SRVHeap->GetGPUDescriptorHandleForHeapStart());

    //Depth
    CD3DX12_GPU_DESCRIPTOR_HANDLE depthDescriptor(SRVHeap->GetGPUDescriptorHandleForHeapStart());
    depthDescriptor.Offset(DepthIndex, DXInf->CbvSrvUavDescriptorsize);
    ICmdList->SetGraphicsRootDescriptorTable(1, depthDescriptor);

    //Depth
    CD3DX12_GPU_DESCRIPTOR_HANDLE noiseDescriptor(SRVHeap->GetGPUDescriptorHandleForHeapStart());
    noiseDescriptor.Offset(NoiseIndex, DXInf->CbvSrvUavDescriptorsize);
    ICmdList->SetGraphicsRootDescriptorTable(2, depthDescriptor);

    ICmdList->ClearRenderTargetView(RTVHeap->GetCPUDescriptorHandleForHeapStart(), DXInf->Clearcolor, 0, nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE CPUDeschandle = RTVHeap->GetCPUDescriptorHandleForHeapStart();

    ICmdList->OMSetRenderTargets(1, &CPUDeschandle, true, nullptr);

    ICmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    ICmdList->IASetVertexBuffers(0, 1, &VbView);
    ICmdList->DrawInstanced(4, 1, 0, 0);

    Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(DXInf->Resourceheap->GetResource("SSAOMap"),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    ICmdList->ResourceBarrier(1, &Resourcebarrier);

}






