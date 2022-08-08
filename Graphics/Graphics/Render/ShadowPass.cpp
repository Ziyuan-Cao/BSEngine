#include "ShadowPass.h"

// 输入灯光
// 输入物体
// 输出一张阴影图

void ShadowPass::OnResize(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList)
{

    VertexsAndIndexesInput();
    BuildHeaps(ICmdList);
    BuildDescriptorHeaps(IDevice);
    CreateDescriptors(IDevice);
    InitShadowConstant(IDevice);
    if (!Init)
    {
        ReloadDescriptors(IDevice);
    }
    Init = false;
}


void ShadowPass::VertexsAndIndexesInput()
{

}


void ShadowPass::BuildHeaps(ID3D12GraphicsCommandList* ICmdList)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = ShadowBufferWidth;
    depthStencilDesc.Height = ShadowBufferHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

    depthStencilDesc.SampleDesc.Count = DXInf->Msaa4xstate ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = DXInf->Msaa4xstate ? (DXInf->Msaa4xquality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = DXInf->Depthstencilformat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    DXInf->Resourceheap->AddResource("ShadowBuffer",
        depthStencilDesc,
        nullptr,
        0,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear);

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = DXInf->Depthstencilformat;
    dsvDesc.Texture2D.MipSlice = 0;
    DXInf->Resourceheap->CreateDSVDesc("ShadowBuffer", dsvDesc);

    D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;
    ZeroMemory(&descSRV, sizeof(descSRV));
    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    DXInf->Resourceheap->CreateSRVDesc("ShadowBuffer", descSRV);
}


void ShadowPass::BuildDescriptorHeaps(ID3D12Device* IDevice)
{
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(IDevice->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(&DSVHeap)));

}

void ShadowPass::CreateDescriptors(ID3D12Device* IDevice)
{

    ReloadDescriptors(IDevice);
}

void ShadowPass::ReloadDescriptors(ID3D12Device* IDevice)
{

    DX_Information* DXInf = DX_Information::GetInstance();

    //DSV
    IDevice->CreateDepthStencilView(
        DXInf->Resourceheap->GetResource("ShadowBuffer"),
        DXInf->Resourceheap->GetDSVDesc("ShadowBuffer"),
        DSVHeap->GetCPUDescriptorHandleForHeapStart());
}


void ShadowPass::BuildRootSignature(ID3D12Device* IDevice)
{

    DX_Information* DXInf = DX_Information::GetInstance();

    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[3];

    //ObjCB
    slotRootParameter[0].InitAsConstantBufferView(0);//cbPerObject : register(b0) 
    //SenceCB
    slotRootParameter[1].InitAsConstantBufferView(1);//cbPass : register(b1)

    auto staticSamplers = DXInf->GetStaticSamplers();

    // A root signature is an array of root parameters.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    ID3DBlob* serializedRootSig = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        &serializedRootSig, &errorBlob);

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(IDevice->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&RootSignature)));
}

void ShadowPass::BuildShadersAndInputLayout()
{
    Shaders["ShadowVS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\ShadowPass.hlsl", nullptr, "VS", "vs_5_1");
    Shaders["ShadowPS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\ShadowPass.hlsl", nullptr, "PS", "ps_5_1");

    InputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "MATERIALID",0,DXGI_FORMAT_R32G32_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
}

void ShadowPass::BuildPSO(ID3D12Device* IDevice)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    CD3DX12_RASTERIZER_DESC opaqueRastDesc(D3D12_DEFAULT);
    opaqueRastDesc.CullMode = D3D12_CULL_MODE_NONE;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
    //
    // PSO for opaque objects.
    //
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
    opaquePsoDesc.pRootSignature = RootSignature;
    opaquePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(Shaders["ShadowVS"]->GetBufferPointer()),
        Shaders["ShadowVS"]->GetBufferSize()
    };
    opaquePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(Shaders["ShadowPS"]->GetBufferPointer()),
        Shaders["ShadowPS"]->GetBufferSize()
    };
    opaquePsoDesc.RasterizerState = opaqueRastDesc;
    opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaquePsoDesc.SampleMask = UINT_MAX;
    opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    DXInf->SetPSO(opaquePsoDesc);
    opaquePsoDesc.NumRenderTargets = 1;

    ThrowIfFailed(IDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&PSOs["Shadow"])));

}

void ShadowPass::InitShadowConstant(ID3D12Device* IDevice)
{
    ShadowconstantsGPU = new BGPU_Upload_Resource<RRender_Scene::SceneConstants>(IDevice, 1, true);
}

void ShadowPass::UpdateShadowConstant(RRender_Scene* IRenderscene,RLight* ILight)
{
    RRender_Scene::SceneConstants Shadowcontants;

    RLight::LightNFWVPT lightNFWVPT;
    ILight->GetLightMatrix(IRenderscene->GetSceneBounds(), lightNFWVPT);

    XMMATRIX view = XMLoadFloat4x4(&lightNFWVPT.mLightView);
    XMMATRIX proj = XMLoadFloat4x4(&lightNFWVPT.mLightProj);
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);

    XMVECTOR matrixview = XMMatrixDeterminant(view);
    XMVECTOR matrixproj = XMMatrixDeterminant(proj);
    XMVECTOR matrixviewproj = XMMatrixDeterminant(viewProj);

    XMMATRIX invView = XMMatrixInverse(&matrixview, view);
    XMMATRIX invProj = XMMatrixInverse(&matrixproj, proj);
    XMMATRIX invViewProj = XMMatrixInverse(&matrixviewproj, viewProj);

    UINT w = ShadowBufferWidth;
    UINT h = ShadowBufferHeight;

    XMStoreFloat4x4(&Shadowcontants.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&Shadowcontants.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&Shadowcontants.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&Shadowcontants.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&Shadowcontants.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&Shadowcontants.InvViewProj, XMMatrixTranspose(invViewProj));
    Shadowcontants.EyePosW = lightNFWVPT.mLightPosW;
    Shadowcontants.RenderTargetSize = XMFLOAT2((float)w, (float)h);
    Shadowcontants.InvRenderTargetSize = XMFLOAT2(1.0f / w, 1.0f / h);
    Shadowcontants.NearZ = lightNFWVPT.mLightNearZ;
    Shadowcontants.FarZ = lightNFWVPT.mLightFarZ;

    auto currPassCB = ShadowconstantsGPU;
    currPassCB->CopyData(0, Shadowcontants);
}


void ShadowPass::Draw(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList, RRender_Scene* IRenderscene)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    ICmdList->SetGraphicsRootSignature(RootSignature);

    ICmdList->SetPipelineState(PSOs["Shadow"]);

    //SenceCB
    auto passCB = ShadowconstantsGPU;
    ICmdList->SetGraphicsRootConstantBufferView(1, passCB->Resource()->GetGPUVirtualAddress());

    ICmdList->RSSetViewports(1, DXInf->GetScreenViewport());
    ICmdList->RSSetScissorRects(1, DXInf->GetScissorRect());


    CD3DX12_RESOURCE_BARRIER Recourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        DXInf->Resourceheap->GetResource("ShadowBuffer"),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_DEPTH_WRITE);

    ICmdList->ResourceBarrier(1,
        &Recourcebarrier);

    ICmdList->ClearDepthStencilView(DSVHeap->GetCPUDescriptorHandleForHeapStart(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f,
        0,
        0,
        nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE DSVDeschandle = DSVHeap->GetCPUDescriptorHandleForHeapStart();

    ICmdList->OMSetRenderTargets(0,
        nullptr,
        true,
        &DSVDeschandle);

    //绘制场景物体，需要区分各类物体
    const std::vector<RRender_Scene::RenderItem>& Renderitems = IRenderscene->GetRenderItems();
    for (int i = 0; i < Renderitems.size(); i++)
    {
        DrawRenderItem(ICmdList, Renderitems[i]);
    }

    Recourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        DXInf->Resourceheap->GetResource("ShadowBuffer"),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        D3D12_RESOURCE_STATE_PRESENT);

    ICmdList->ResourceBarrier(1,
        &Recourcebarrier);

}

void ShadowPass::DrawRenderItem(ID3D12GraphicsCommandList* ICmdList, const RRender_Scene::RenderItem& IRenderitem)
{
    UINT objCBByteSize = IRenderitem.Objectmodel->GetObjectConstantsGPU()->GetElementbytesize();
    D3D12_GPU_VIRTUAL_ADDRESS ObjCBaddress = IRenderitem.Objectmodel->GetObjectConstantsGPU()->Resource()->GetGPUVirtualAddress();

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

        //当统一顶点和索引时可用这个
        //ICmdList->DrawIndexedInstanced(&(*Renderitem)[i]->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
        ICmdList->DrawIndexedInstanced(Renderitem[i].Indexcount, 1, 0, 0, 0);

    }
}
