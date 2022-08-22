#include "TransparentPass.h"
#include "DirectX/DX_Information.h"

//MeshPass
// input : objcb matcb lightmap gbuffer

void TransparentPass::VertexsAndIndexesInput()
{

}

void TransparentPass::BuildDescriptorHeaps(ID3D12Device* IDevice)
{
    //create SRV DescriptorHeap
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {}; //Gbuffer + LightMap
    srvHeapDesc.NumDescriptors = GBufferRTCount +1;
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

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(IDevice->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(&DSVHeap)));
}

void TransparentPass::CreateDescriptors(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(SRVHeap->GetCPUDescriptorHandleForHeapStart());

    LightIndex = 0;
    GBufferIndex = LightIndex+1;

    IDevice->CreateShaderResourceView(
        DXInf->Resourceheap->GetResource("LightMap"),
        DXInf->Resourceheap->GetSRVDesc("LightMap"),
        hDescriptor);
    hDescriptor.Offset(1, DXInf->CbvSrvUavDescriptorsize);


    for (int i = 0; i < GBufferRTCount; i++) {
        IDevice->CreateShaderResourceView(
            DXInf->Resourceheap->GetResource("GBuffer" + std::to_string(i)),
            DXInf->Resourceheap->GetSRVDesc("GBuffer" + std::to_string(i)),
            hDescriptor);
        hDescriptor.Offset(1, DXInf->CbvSrvUavDescriptorsize);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart());

    IDevice->CreateRenderTargetView(
        DXInf->Resourceheap->GetResource("TransparentMap"),
        DXInf->Resourceheap->GetRTVDesc("TransparentMap"),
        rtvHeapHandle);

    //DSV
    IDevice->CreateDepthStencilView(
        DXInf->Resourceheap->GetResource("DepthStencilBuffer"),
        DXInf->Resourceheap->GetDSVDesc("DepthStencilBuffer"),
        DSVHeap->GetCPUDescriptorHandleForHeapStart());
}


void TransparentPass::BuildHeaps(ID3D12GraphicsCommandList* ICmdList)
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

    DXInf->Resourceheap->AddResource("TransparentMap",
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
    DXInf->Resourceheap->CreateRTVDesc("TransparentMap", descRTV);

    D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;
    ZeroMemory(&descSRV, sizeof(descSRV));
    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descSRV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXInf->Resourceheap->CreateSRVDesc("TransparentMap", descSRV);
}


void TransparentPass::BuildRootSignature(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    CD3DX12_DESCRIPTOR_RANGE range[4];
    range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); //LightMap
    range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);//watermap1
    range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);//watermap2

    range[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, GBufferRTCount, 4);//GBuffer
   

    CD3DX12_ROOT_PARAMETER rootParameters[7];
    rootParameters[0].InitAsConstantBufferView(0);//cbPerObject : register(b0) 
    rootParameters[1].InitAsConstantBufferView(1);//cbPass : register(b1) 
    rootParameters[2].InitAsShaderResourceView(0, 1);//gMaterialData : register(t0, space1);
    rootParameters[3].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[4].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[5].InitAsDescriptorTable(1, &range[2], D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[6].InitAsDescriptorTable(1, &range[3], D3D12_SHADER_VISIBILITY_PIXEL);


    auto staticSamplers = DXInf->GetStaticSamplers();

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
    descRootSignature.Init(7,
        rootParameters,
        (UINT)staticSamplers.size(),
        staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ID3DBlob* rootSigBlob;
    ID3DBlob* errorBlob;

    ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob));

    ThrowIfFailed(IDevice->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));

}


void TransparentPass::BuildShadersAndInputLayout()
{
    Shaders["TransparentVS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\TransparentPass.hlsl", nullptr, "VS", "vs_5_1");
    //Shaders["TransparentHS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\TransparentPass.hlsl", nullptr, "HS", "hs_5_1");
    //Shaders["TransparentDS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\TransparentPass.hlsl", nullptr, "DS", "ds_5_1");
    Shaders["TransparentPS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\TransparentPass.hlsl", nullptr, "PS", "ps_5_1");

    InputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "MATERIALID",0,DXGI_FORMAT_R32G32_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
}

void TransparentPass::BuildPSO(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
    ZeroMemory(&descPipelineState, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    descPipelineState.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
    descPipelineState.pRootSignature = RootSignature;
    descPipelineState.VS =
    {
        reinterpret_cast<BYTE*>(Shaders["TransparentVS"]->GetBufferPointer()),
        Shaders["TransparentVS"]->GetBufferSize()
    };
    //descPipelineState.HS =
    //{
    //    reinterpret_cast<BYTE*>(Shaders["TransparentHS"]->GetBufferPointer()),
    //    Shaders["TransparentHS"]->GetBufferSize()
    //};
    //descPipelineState.DS =
    //{
    //    reinterpret_cast<BYTE*>(Shaders["TransparentDS"]->GetBufferPointer()),
    //    Shaders["TransparentDS"]->GetBufferSize()
    //};
    descPipelineState.PS =
    {
        reinterpret_cast<BYTE*>(Shaders["TransparentPS"]->GetBufferPointer()),
        Shaders["TransparentPS"]->GetBufferSize()
    };
    descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    descPipelineState.DepthStencilState.DepthEnable = true;
    descPipelineState.SampleMask = UINT_MAX;
    descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    descPipelineState.NumRenderTargets = 1;
    //descPipelineState.RTVFormats[0] = mRtvFormat[0];
    descPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    descPipelineState.SampleDesc.Count = 1;
    DXInf->SetPSO(descPipelineState);
    ThrowIfFailed(IDevice->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&PSOs["Transparent"])));


}

void TransparentPass::Draw(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList, RRender_Scene* IRenderscene)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    CD3DX12_RESOURCE_BARRIER Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(DXInf->Resourceheap->GetResource("TransparentMap"),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    ICmdList->ResourceBarrier(1, &Resourcebarrier);

    Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        DXInf->Resourceheap->GetResource("DepthStencilBuffer"),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_RESOURCE_STATE_DEPTH_WRITE);

    ICmdList->ResourceBarrier(1,
        &Resourcebarrier);

    auto Gbuffer_Handle_c = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        RTVHeap->GetCPUDescriptorHandleForHeapStart(),
        0,
        DXInf->RtvDescriptorsize);

    ICmdList->ClearRenderTargetView(Gbuffer_Handle_c, DXInf->Clearcolor, 0, nullptr);
    Gbuffer_Handle_c.Offset(1, DXInf->RtvDescriptorsize);

    ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap };
    ICmdList->SetGraphicsRootSignature(RootSignature);

    ICmdList->SetPipelineState(PSOs["Transparent"]);

    ICmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);


    //PASSCB
    auto passCB = IRenderscene->GetSceneConstantsGPU();
    ICmdList->SetGraphicsRootConstantBufferView(1, passCB->Resource()->GetGPUVirtualAddress());

    //LightMap
    CD3DX12_GPU_DESCRIPTOR_HANDLE LightDescriptor(SRVHeap->GetGPUDescriptorHandleForHeapStart());
    LightDescriptor.Offset(LightIndex, DXInf->CbvSrvUavDescriptorsize);
    ICmdList->SetGraphicsRootDescriptorTable(3, LightDescriptor);

    //GBUFFER
    CD3DX12_GPU_DESCRIPTOR_HANDLE GBufferDescriptor(SRVHeap->GetGPUDescriptorHandleForHeapStart());
    GBufferDescriptor.Offset(GBufferIndex, DXInf->CbvSrvUavDescriptorsize);
    ICmdList->SetGraphicsRootDescriptorTable(6, GBufferDescriptor);

    ICmdList->RSSetViewports(1, DXInf->GetScreenViewport());
    ICmdList->RSSetScissorRects(1, DXInf->GetScissorRect());

    D3D12_CPU_DESCRIPTOR_HANDLE DSVDeschandle = DSVHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE RTVDeschandle = RTVHeap->GetCPUDescriptorHandleForHeapStart();


    ICmdList->OMSetRenderTargets(1,
        &RTVDeschandle,
        true,
        &DSVDeschandle);

    const std::vector<RRender_Scene::RenderItem>& Renderitems = IRenderscene->GetRenderItems();
    for (int i = 0; i < Renderitems.size(); i++)
    {
        if (Renderitems[i].Objectmodel->isWater && Renderitems[i].Objectmodel->hasTexture)
        {
            auto TexSRVHeap = Renderitems[i].Objectmodel->GetTextureDescHeap();

            ID3D12DescriptorHeap* descriptorHeaps[] = { TexSRVHeap };
            ICmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            CD3DX12_GPU_DESCRIPTOR_HANDLE W1Descriptor(TexSRVHeap->GetGPUDescriptorHandleForHeapStart());

            ICmdList->SetGraphicsRootDescriptorTable(4, W1Descriptor);
            W1Descriptor.Offset(1, DXInf->CbvSrvUavDescriptorsize);

            ICmdList->SetGraphicsRootDescriptorTable(5, W1Descriptor);


            DrawRenderItem(ICmdList, Renderitems[i]);
        }
        if (Renderitems[i].Objectmodel->isTransparent)
        {

        }
    }


    Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        DXInf->Resourceheap->GetResource("DepthStencilBuffer"),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        D3D12_RESOURCE_STATE_GENERIC_READ);

    ICmdList->ResourceBarrier(1, &Resourcebarrier);


    Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(DXInf->Resourceheap->GetResource("TransparentMap"),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    ICmdList->ResourceBarrier(1, &Resourcebarrier);

}


void TransparentPass::DrawRenderItem(ID3D12GraphicsCommandList* ICmdList, const RRender_Scene::RenderItem& IRenderitem)
{
    UINT objCBByteSize = IRenderitem.Objectmodel->GetObjectConstantsGPU()->GetElementbytesize();
    D3D12_GPU_VIRTUAL_ADDRESS ObjCBaddress = IRenderitem.Objectmodel->GetObjectConstantsGPU()->Resource()->GetGPUVirtualAddress();

    UINT matCBByteSize = IRenderitem.Objectmodel->GetMaterialConstantsGPU()->GetElementbytesize();
    D3D12_GPU_VIRTUAL_ADDRESS MatCBaddress = IRenderitem.Objectmodel->GetMaterialConstantsGPU()->Resource()->GetGPUVirtualAddress();

    const std::vector<RObject_Model::GPUMeshData>& Renderitem = IRenderitem.Objectmodel->GetGPUGeometries();
    auto Geonumber = IRenderitem.Objectmodel->GetGeometriesnumber();

    //遍历子物体
    for (int i = 0; i < Geonumber; i++)
    {
        D3D12_VERTEX_BUFFER_VIEW Vertexbufferview = Renderitem[i].GetGPUVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW Indexbufferview = Renderitem[i].GetGPUIndexBufferView();

        ICmdList->IASetVertexBuffers(0, 1, &Vertexbufferview);
        ICmdList->IASetIndexBuffer(&Indexbufferview);
        ICmdList->IASetPrimitiveTopology(IRenderitem.Objectmodel->GetPrimitiveType());

        //子物体objCB偏移
        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = ObjCBaddress + i * objCBByteSize;
        ICmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);

        //MatCB 
        auto matBuffer = MatCBaddress + i * matCBByteSize;
        ICmdList->SetGraphicsRootShaderResourceView(2, MatCBaddress);

        //当统一顶点和索引时可用这个
        //ICmdList->DrawIndexedInstanced(&(*Renderitem)[i]->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
        ICmdList->DrawIndexedInstanced(Renderitem[i].Indexcount, 1, 0, 0, 0);

    }

}