#include"Render/BGPU_Resource_Factory.h"

void BGPU_Resource_Factory::AssignGPUObject(
    ID3D12Device* IDevice,
    ID3D12GraphicsCommandList* ICmdList,
    RObject_Model* IObject_Model)
{
    if (IObject_Model->isGPUInit)
    {
        return;
    }
    IObject_Model->isGPUInit = true;

    //MatCB
    UINT Matnumber = IObject_Model->CPUMeshdata.Materialgroup.size();
    IObject_Model->MaterialconstantsGPU = new BGPU_Upload_Resource<RMaterial>(IDevice, Matnumber, true);
    UpdateGPUMaterials(IObject_Model);
    //ObjCB
    IObject_Model->ObjectconstantsGPU = new BGPU_Upload_Resource<RObject_Model::ObjectConstant>(IDevice, Matnumber, true);
    UpdateGPUObjectCB(IObject_Model);
    //???
    IObject_Model->Geometriesnumber = Matnumber;

    //Vertex 
    if (IObject_Model->hasAnimation)
    {
        auto* Skeletonmodel = (RSkeleton_Model*)IObject_Model;
        UINT Vertexnumber = Skeletonmodel->Verticescurrent.size();
        Skeletonmodel->CurrentVertexbufferGPU = new BGPU_Upload_Resource <GVertex>(IDevice, Vertexnumber,true);
        UpdateGPUVertexCB(Skeletonmodel);

        Skeletonmodel->VertexbufferGPU = Skeletonmodel->CurrentVertexbufferGPU->Resource();
        IObject_Model->Vertexbytestride = sizeof(GVertex);
        IObject_Model->Vertexbufferbytesize = Vertexnumber * sizeof(GVertex);
    }
    else
    {
        const UINT vbByteSize = (UINT)IObject_Model->CPUMeshdata.Vertices.size() * sizeof(GVertex);
        void* Vertexbuffer = malloc(vbByteSize);
        void* CPUVectorptr = IObject_Model->CPUMeshdata.Vertices.data();
        memmove(Vertexbuffer, CPUVectorptr, vbByteSize);

        ThrowIfFailed(D3DCreateBlob(vbByteSize, &IObject_Model->VertexbufferCPU));
        CopyMemory(IObject_Model->VertexbufferCPU->GetBufferPointer(), Vertexbuffer, vbByteSize);

        IObject_Model->VertexbufferGPU = CreateDefaultBuffer(IDevice,
            ICmdList, Vertexbuffer, vbByteSize, IObject_Model->Vertexbufferuploader);

        IObject_Model->Vertexbytestride = sizeof(GVertex);
        IObject_Model->Vertexbufferbytesize = vbByteSize;
    }

    //子物体Vertex index
    for (int i = 0; i < IObject_Model->Geometriesnumber; i++)
    {
        //Mesh
        auto geo = RObject_Model::GPUMeshData();

        geo.VertexbufferCPU = IObject_Model->VertexbufferCPU;
        geo.VertexbufferGPU = IObject_Model->VertexbufferGPU;
        geo.Vertexbufferuploader = IObject_Model->Vertexbufferuploader;
        geo.Vertexbytestride = IObject_Model->Vertexbytestride;
        geo.Vertexbufferbytesize = IObject_Model->Vertexbufferbytesize;

        const UINT ibByteSize = (UINT)IObject_Model->CPUMeshdata.Indeicesoffset[i] * sizeof(std::int32_t);
        void* Indexbuffer = malloc(ibByteSize);
        void* CPUIndexptr = IObject_Model->CPUMeshdata.Indices.data();

        memmove(Indexbuffer, CPUIndexptr, ibByteSize);

        ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo.IndexbufferCPU));
        CopyMemory(geo.IndexbufferCPU->GetBufferPointer(), Indexbuffer, ibByteSize);


        geo.IndexbufferGPU = CreateDefaultBuffer(IDevice,
            ICmdList, Indexbuffer, ibByteSize, geo.Indexbufferuploader);

        geo.Indexformat = DXGI_FORMAT_R32_UINT;
        geo.Indexbufferbytesize = ibByteSize;
        geo.Indexcount = (UINT)IObject_Model->CPUMeshdata.Indeicesoffset[i];

        IObject_Model->GPUGeometries.push_back(std::move(geo));
    }

}

//全场景物体加载，需要内存优化
void BGPU_Resource_Factory::AssignGPUObjects(
    ID3D12Device* IDevice,
    ID3D12GraphicsCommandList* ICmdList,
    RRender_Scene* IOGPUScene)
{
    ////???强制转换？
    for (int i = 0; i < IOGPUScene->Staticgroup.size(); i++)
    {
        if(IOGPUScene->Staticgroup[i]->Visible == true)
        {
            AssignGPUObject(IDevice, ICmdList, (RStatic_Model*)IOGPUScene->Staticgroup[i]);
            
            RRender_Scene::RenderItem Renderitem;
            //???
            //Renderitem.TexTransform = ;?
            Renderitem.Objectmodel = (RStatic_Model*)IOGPUScene->Skeletongroup[i];
            IOGPUScene->Renderitems.push_back(Renderitem);
        }
    }
    for (int i = 0; i < IOGPUScene->Skeletongroup.size(); i++)
    {
        if (IOGPUScene->Skeletongroup[i]->Visible == true)
        {
            AssignGPUObject(IDevice, ICmdList, (RSkeleton_Model*)IOGPUScene->Skeletongroup[i]);
            RRender_Scene::RenderItem Renderitem;
            //???
            //Renderitem.TexTransform = ;?
            Renderitem.Objectmodel = (RSkeleton_Model*)IOGPUScene->Skeletongroup[i];
            IOGPUScene->Renderitems.push_back(Renderitem);
        }
    }
}


void BGPU_Resource_Factory::ReleaseGPUObject(
    ID3D12Device* IDevice,
    ID3D12GraphicsCommandList* ICmdList,
    RObject_Model* IObject_Model)
{
    //...
}

void BGPU_Resource_Factory::AssignGPUScene(
    ID3D12Device* IDevice,
    RRender_Scene* IOGPUScene
)
{
    //帧资源多个对应多线程？？？2
    IOGPUScene->SceneconstantsGPU = new BGPU_Upload_Resource<RRender_Scene::SceneConstants>(IDevice,2,true);
}

void BGPU_Resource_Factory::UpdateGPUScene(RRender_Scene* IOGPUScene)
{
    RRender_Scene::SceneConstants Scenecontants;
    
    RCamera* Camera = (RCamera*)IOGPUScene->Cameragroup[0];

    //???强制转换？
    XMMATRIX view = Camera->GetView();
    XMMATRIX proj = Camera->GetProj();

    XMMATRIX viewProj = XMMatrixMultiply(view, proj);

    XMVECTOR matrixview = XMMatrixDeterminant(view);
    XMVECTOR matrixproj = XMMatrixDeterminant(proj);
    XMVECTOR matrixviewproj = XMMatrixDeterminant(viewProj);

    XMMATRIX invView = XMMatrixInverse(&matrixview, view);
    XMMATRIX invProj = XMMatrixInverse(&matrixproj, proj);
    XMMATRIX invViewProj = XMMatrixInverse(&matrixviewproj, viewProj);
    //???
    //XMMATRIX shadowTransform = XMLoadFloat4x4(0);

    XMStoreFloat4x4(&Scenecontants.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&Scenecontants.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&Scenecontants.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&Scenecontants.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&Scenecontants.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&Scenecontants.InvViewProj, XMMatrixTranspose(invViewProj));
    //XMStoreFloat4x4(&Scenecontants.ShadowTransform, XMMatrixTranspose(shadowTransform));


    //???强制转换？
    Scenecontants.EyePosW = ((RCamera*)IOGPUScene->Cameragroup[0])->GetPosition3f();
    //Scenecontants.RenderTargetSize = XMFLOAT2((float)d3dApp->mClientWidth, (float)d3dApp->mClientHeight);
    //Scenecontants.InvRenderTargetSize = XMFLOAT2(1.0f / d3dApp->mClientWidth, 1.0f / d3dApp->mClientHeight);
    Scenecontants.NearZ = 1.0f;
    Scenecontants.FarZ = 1000.0f;
    //Scenecontants.TotalTime = gt.TotalTime();
    //Scenecontants.DeltaTime = gt.DeltaTime();
    //should be fixed in lightPass
    Scenecontants.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
    Scenecontants.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
    Scenecontants.Lights[0].Strength = { 0.9f, 0.8f, 0.7f };
    Scenecontants.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
    Scenecontants.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
    Scenecontants.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
    Scenecontants.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

    IOGPUScene->SceneconstantsGPU->CopyData(0, Scenecontants);
}

ID3D12Resource* BGPU_Resource_Factory::CreateDefaultBuffer(
    ID3D12Device* IDevice,
    ID3D12GraphicsCommandList* ICmdList,
    const void* IInitData,
    UINT64 IByteSize,
    ID3D12Resource* OUploadBuffer)
{
    ID3D12Resource* Defaultbuffer;

    // Create the actual default buffer resource.
    CD3DX12_HEAP_PROPERTIES Heapproperties(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC Resourcedesc = CD3DX12_RESOURCE_DESC::Buffer(IByteSize);

    ThrowIfFailed(IDevice->CreateCommittedResource(
        &Heapproperties,
        D3D12_HEAP_FLAG_NONE,
        &Resourcedesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&Defaultbuffer)));

    // In order to copy CPU memory data into our default buffer, we need to create
    // an intermediate upload heap. 
    CD3DX12_HEAP_PROPERTIES Heappropertiesup(D3D12_HEAP_TYPE_UPLOAD);
    Resourcedesc = CD3DX12_RESOURCE_DESC::Buffer(IByteSize);

    ThrowIfFailed(IDevice->CreateCommittedResource(
        &Heappropertiesup,
        D3D12_HEAP_FLAG_NONE,
        &Resourcedesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&OUploadBuffer)));


    // Describe the data we want to copy into the default buffer.
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = IInitData;
    subResourceData.RowPitch = IByteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
    // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
    // the intermediate upload heap data will be copied to mBuffer.

    CD3DX12_RESOURCE_BARRIER Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(Defaultbuffer,
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    ICmdList->ResourceBarrier(1, &Resourcebarrier);
    UpdateSubresources<1>(ICmdList, Defaultbuffer, OUploadBuffer, 0, 0, 1, &subResourceData);

    Resourcebarrier = CD3DX12_RESOURCE_BARRIER::Transition(Defaultbuffer,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    ICmdList->ResourceBarrier(1, &Resourcebarrier);

    // Note: uploadBuffer has to be kept alive after the above function calls because
    // the command list has not been executed yet that performs the actual copy.
    // The caller can Release the uploadBuffer after it knows the copy has been executed.


    return Defaultbuffer;
}

void BGPU_Resource_Factory::UpdateGPUMaterials(RObject_Model* IObject_Model)
{
    auto currMaterialbuffer = IObject_Model->GetMaterialConstantsGPU();

    int Matnumber = IObject_Model->CPUMeshdata.Materialgroup.size();
    for (int i = 0; i < Matnumber;i++)//each materials
    {
        RMaterial* mat = IObject_Model->CPUMeshdata.Materialgroup[i];

        currMaterialbuffer->CopyData(i, *mat);

    }
}
void BGPU_Resource_Factory::UpdateGPUObjectCB(RObject_Model* IObject_Model)
{
    auto currObjectCB = IObject_Model->GetObjectConstantsGPU();

    //int Objnumber = IObject_Model->CPUMeshdata.Objectconstants.size();
    
    XMMATRIX world = XMMatrixTranslation(IObject_Model->Transform[0],
        IObject_Model->Transform[1],
        IObject_Model->Transform[2])
        * XMMatrixRotationY(IObject_Model->Rotation[0])
        * XMMatrixRotationZ(IObject_Model->Rotation[1])
        * XMMatrixRotationX(IObject_Model->Rotation[2])
        * XMMatrixScaling(IObject_Model->Scale[0],
            IObject_Model->Scale[1],
            IObject_Model->Scale[2]);

    //???
    //XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

    RObject_Model::ObjectConstant objConstants;
    XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
    //XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
    

    for (int i = 0; i < IObject_Model->Materialnums; i++)//each materials
    {
        objConstants.MaterialIndex = i;
        currObjectCB->CopyData(i, objConstants);
    }
}

void BGPU_Resource_Factory::UpdateGPUVertexCB(
    RSkeleton_Model* IObject_Model)
{
    if (IObject_Model->hasAnimation)
    {
        void* CPUVectorptr = IObject_Model->Verticescurrent.data();
        IObject_Model->CurrentVertexbufferGPU->Copy(CPUVectorptr);
    }
}


ID3DBlob* BGPU_Resource_Factory::CompileShader(
    const std::wstring& filename,
    const D3D_SHADER_MACRO* defines,
    const std::string& entrypoint,
    const std::string& target)
{
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = S_OK;

    ID3DBlob* byteCode = nullptr;
    ID3DBlob* errors;
    hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

    if (errors != nullptr)
        OutputDebugStringA((char*)errors->GetBufferPointer());

    ThrowIfFailed(hr);

    return byteCode;
}