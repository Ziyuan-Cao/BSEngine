#include "Render/BasePass.h"
#include <string>

void BasePass::OnResize(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList)
{

    VertexsAndIndexesInput();
    BuildHeaps(ICmdList);
    BuildDescriptorHeaps(IDevice);
    CreateDescriptors(IDevice);
    if (!Init)
    {
        ReloadDescriptors(IDevice);
    }
    Init = false;
}

//void BasePass::LoadTexture()
//{
//    BuildDescriptorHeaps();
//    CreateDescriptors();
//}

void BasePass::VertexsAndIndexesInput()
{
    
}


void BasePass::BuildHeaps(ID3D12GraphicsCommandList* ICmdList)
{
    D3D12_RESOURCE_DESC resourceDesc;
    ZeroMemory(&resourceDesc, sizeof(resourceDesc));
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.MipLevels = 1;

    DX_Information* DXInf = DX_Information::GetInstance();


    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Width = DXInf->GetWClientWidth();
    resourceDesc.Height = DXInf->GetWClientHeight();
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    resourceDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

    D3D12_CLEAR_VALUE clearVal;
    clearVal.Color[0] = DXInf->Clearcolor[0];
    clearVal.Color[1] = DXInf->Clearcolor[1];
    clearVal.Color[2] = DXInf->Clearcolor[2];
    clearVal.Color[3] = DXInf->Clearcolor[3];

    D3D12_RENDER_TARGET_VIEW_DESC descRTV;
    ZeroMemory(&descRTV, sizeof(descRTV));
    descRTV.Texture2D.MipSlice = 0;
    descRTV.Texture2D.PlaneSlice = 0;
    descRTV.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    descRTV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

    D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;
    ZeroMemory(&descSRV, sizeof(descSRV));
    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descSRV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

    for (int i = 0; i < GBufferRTCount; i++) {
        clearVal.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        DXInf->Resourceheap->AddResource("GBuffer" + std::to_string(i),
            resourceDesc,
            nullptr,
            0,
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearVal);

        DXInf->Resourceheap->CreateRTVDesc("GBuffer" + std::to_string(i), descRTV);

        DXInf->Resourceheap->CreateSRVDesc("GBuffer" + std::to_string(i), descSRV);

    }

    // Create the depth/stencil buffer and view.
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = DXInf->GetWClientWidth();
    depthStencilDesc.Height = DXInf->GetWClientHeight();
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
    DXInf->Resourceheap->AddResource("DepthStencilBuffer",
        depthStencilDesc,
        nullptr,
        0,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear);


    // Create descriptor to mip level 0 of entire resource using the format of the resource.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = DXInf->Depthstencilformat;
    dsvDesc.Texture2D.MipSlice = 0;
    DXInf->Resourceheap->CreateDSVDesc("DepthStencilBuffer", dsvDesc);

    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    DXInf->Resourceheap->CreateSRVDesc("DepthStencilBuffer", descSRV);

    // Transition the resource from its initial state to be used as a depth buffer.
    CD3DX12_RESOURCE_BARRIER Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        DXInf->Resourceheap->GetResource("DepthStencilBuffer"),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_GENERIC_READ);
    
    ICmdList->ResourceBarrier(1,&Resourcebarrier);
}


//after texture 
void BasePass::BuildDescriptorHeaps(ID3D12Device* IDevice)
{
    //create SRV DescriptorHeap
    //每画一个物体要用对应的贴图堆描述符
    // 使用贴图堆紧凑内存，优化速度
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(IDevice->CreateDescriptorHeap(
        &srvHeapDesc, IID_PPV_ARGS(&SRVHeap)));
    
    //create RTV DescriptorHeap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = GBufferRTCount;
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

//缺阴影？
void BasePass::CreateDescriptors(ID3D12Device* IDevice)
{

    ReloadDescriptors(IDevice);
}

void BasePass::ReloadDescriptors(ID3D12Device* IDevice)
{

    DX_Information* DXInf = DX_Information::GetInstance();

    //阴影？？
    IDevice->CreateShaderResourceView(DXInf->Resourceheap->GetResource("ShadowBuffer"),
        DXInf->Resourceheap->GetSRVDesc("ShadowBuffer"),
        SRVHeap->GetCPUDescriptorHandleForHeapStart());

    //RTV
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart());


    for (UINT i = 0; i < GBufferRTCount; i++)
    {
        IDevice->CreateRenderTargetView(
            DXInf->Resourceheap->GetResource("GBuffer" + std::to_string(i)),
            DXInf->Resourceheap->GetRTVDesc("GBuffer" + std::to_string(i)),
            rtvHeapHandle);
        rtvHeapHandle.Offset(1, DXInf->RtvDescriptorsize);
    }


    //DSV
    IDevice->CreateDepthStencilView(
        DXInf->Resourceheap->GetResource("DepthStencilBuffer"),
        DXInf->Resourceheap->GetDSVDesc("DepthStencilBuffer"),
        DSVHeap->GetCPUDescriptorHandleForHeapStart());
}

void BasePass::BuildRootSignature(ID3D12Device* IDevice)
{

    DX_Information* DXInf = DX_Information::GetInstance();

    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 1);

    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[4];

    // Perfomance TIP: Order from most frequent to least frequent.
    //ObjCB
    slotRootParameter[0].InitAsConstantBufferView(0);//cbPerObject : register(b0) 
    //SenceCB
    slotRootParameter[1].InitAsConstantBufferView(1);//cbPass : register(b1)
    //MatCB
    slotRootParameter[2].InitAsShaderResourceView(0, 1);//gMaterialData : register(t0, space1);
    //ShadowMap
    slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);//gShadowMap : register(t0,space2);

    //Texture...                                                                    
    //slotRootParameter[4].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);// gCubeMap : register(t0);
    //slotRootParameter[5].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);// gTextureMaps[10] : register(t2); 

    auto staticSamplers = DXInf->GetStaticSamplers();

    // A root signature is an array of root parameters.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
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

void BasePass::BuildShadersAndInputLayout()
{
    Shaders["standardVS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\BasePass.hlsl", nullptr, "VS", "vs_5_1");
    Shaders["opaquePS"] = BGPU_Resource_Factory::CompileShader(L"Shaders\\BasePass.hlsl", nullptr, "PS", "ps_5_1");

    InputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "MATERIALID",0,DXGI_FORMAT_R32G32_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
}

void BasePass::BuildPSO(ID3D12Device* IDevice)
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
        reinterpret_cast<BYTE*>(Shaders["standardVS"]->GetBufferPointer()),
        Shaders["standardVS"]->GetBufferSize()
    };
    opaquePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(Shaders["opaquePS"]->GetBufferPointer()),
        Shaders["opaquePS"]->GetBufferSize()
    };
    opaquePsoDesc.RasterizerState = opaqueRastDesc;
    opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaquePsoDesc.SampleMask = UINT_MAX;
    opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    DXInf->SetPSO(opaquePsoDesc);
    opaquePsoDesc.NumRenderTargets = GBufferRTCount;
    for(int i = 0; i < GBufferRTCount;i++)
    opaquePsoDesc.RTVFormats[i] = DXGI_FORMAT_R32G32B32A32_FLOAT;

    
    ThrowIfFailed(IDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&PSOs["opaque"])));

}

void BasePass::Draw(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICmdList, RRender_Scene* IRenderscene)
{
    DX_Information* DXInf = DX_Information::GetInstance();

    ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap };
    ICmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    ICmdList->SetGraphicsRootSignature(RootSignature);

    ICmdList->SetPipelineState(PSOs["opaque"]);

    //SenceCB
    auto passCB = IRenderscene->GetSceneConstantsGPU();
    ICmdList->SetGraphicsRootConstantBufferView(1, passCB->Resource()->GetGPUVirtualAddress());

    //Shadow
    //auto shadowBuffer = DXInf->Resourceheap->GetResource("ShadowBuffer")->GetGPUVirtualAddress();
    //ICmdList->SetGraphicsRootShaderResourceView(3, shadowBuffer);
    ICmdList->SetGraphicsRootDescriptorTable(3, SRVHeap->GetGPUDescriptorHandleForHeapStart());

    ICmdList->RSSetViewports(1, DXInf->GetScreenViewport());
    ICmdList->RSSetScissorRects(1, DXInf->GetScissorRect());

    // Clear the back buffer and depth buffer.
    auto Gbuffer_Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        RTVHeap->GetCPUDescriptorHandleForHeapStart(),
        0,
        DXInf->RtvDescriptorsize);
    auto Gbuffer_Handle_c = Gbuffer_Handle;
    for (int i = 0; i < GBufferRTCount; i++)
    {
        ICmdList->ClearRenderTargetView(Gbuffer_Handle_c, DXInf->Clearcolor, 0, nullptr);
        Gbuffer_Handle_c.Offset(1, DXInf->RtvDescriptorsize);
    }

    CD3DX12_RESOURCE_BARRIER Recourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        DXInf->Resourceheap->GetResource("DepthStencilBuffer"),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_RESOURCE_STATE_DEPTH_WRITE);

    ICmdList->ResourceBarrier(1,
        &Recourcebarrier);

    ICmdList->ClearDepthStencilView(DSVHeap->GetCPUDescriptorHandleForHeapStart(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f,
        0,
        0,
        nullptr);

    // Specify the buffers we are going to render to.
    D3D12_CPU_DESCRIPTOR_HANDLE CPUDeschandle = DSVHeap->GetCPUDescriptorHandleForHeapStart();

    ICmdList->OMSetRenderTargets(GBufferRTCount, 
        &Gbuffer_Handle, 
        true, 
        &CPUDeschandle);


    //绘制场景物体，需要区分各类物体
    const std::vector<RRender_Scene::RenderItem>& Renderitems = IRenderscene->GetRenderItems();
    for (int i = 0; i < Renderitems.size(); i++)
    {
        DrawRenderItem(ICmdList,Renderitems[i]);
    }

    Recourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        DXInf->Resourceheap->GetResource("DepthStencilBuffer"),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        D3D12_RESOURCE_STATE_GENERIC_READ);

    ICmdList->ResourceBarrier(1,
        &Recourcebarrier);

}
   
void BasePass::DrawRenderItem(ID3D12GraphicsCommandList* ICmdList, const RRender_Scene::RenderItem& IRenderitem)
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

