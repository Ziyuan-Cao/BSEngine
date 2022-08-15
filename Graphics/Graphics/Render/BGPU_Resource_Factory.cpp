#include "Render/BGPU_Resource_Factory.h"
#include "Auxiliary/DDSTextureLoader.h"
#include "DirectX/DX_Information.h"

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
    IObject_Model->MaterialconstantsGPU = new BGPU_Upload_Resource<AMaterial::MaterialData>(IDevice, Matnumber, false);
    UpdateGPUMaterials(IObject_Model);

    AssignTextures(IDevice, ICmdList, IObject_Model);

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

/// <summary>
/// 为灯光和场景常量申请GPU资源
/// </summary>
/// <param name="IDevice"></param>
/// <param name="IOGPUScene"></param>
void BGPU_Resource_Factory::AssignGPUScene(
    ID3D12Device* IDevice,
    RRender_Scene* IOGPUScene
)
{
    int lightsnum = IOGPUScene->Lightgroup.size();

    //帧资源多个对应多线程？？？2
    IOGPUScene->SceneconstantsGPU = new BGPU_Upload_Resource<RRender_Scene::SceneConstants>(IDevice,1,true);

    IOGPUScene->LightGPU = new BGPU_Upload_Resource<ALight::LightData>(IDevice, lightsnum, false);

}

void BGPU_Resource_Factory::AssignTextures(
    ID3D12Device* IDevice,
    ID3D12GraphicsCommandList* ICmdList,
    RObject_Model* IObject_Model)
{

    std::vector<RTexture*> texturebuffer;

    int Matnumber = IObject_Model->CPUMeshdata.Materialgroup.size();
    for (int i = 0; i < Matnumber; i++)
    {
        auto & textures = IObject_Model->CPUMeshdata.Materialgroup[i]->TextureGroup;

        for (int j = 0; j < textures.size(); j++)
        {
            if (textures[j]->GPUInit)
            {
                continue;
            }
            textures[j]->GPUInit = true;
            ThrowIfFailed(
                DirectX::CreateDDSTextureFromFile12(
                    IDevice, 
                    ICmdList,
                    textures[j]->Filepath.c_str(),
                    &textures[j]->TextureGPU,
                    &textures[j]->Textureuploader));

            ZeroMemory(&textures[j]->TextureSRV, sizeof(textures[j]->TextureSRV));
            textures[j]->TextureSRV.Texture2D.MipLevels = 1;
            textures[j]->TextureSRV.Texture2D.MostDetailedMip = 0;
            textures[j]->TextureSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // srv FORMAT
            textures[j]->TextureSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            textures[j]->TextureSRV.Format = textures[j]->TextureGPU->GetDesc().Format; // 精度有关

            texturebuffer.push_back(textures[j]);

        }
    }

    if (!texturebuffer.empty())
    {
        IObject_Model->hasTexture = true;

        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = texturebuffer.size();
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(IDevice->CreateDescriptorHeap(
            &srvHeapDesc, IID_PPV_ARGS(&IObject_Model->TextureSRVHeap)));

        DX_Information* DXInf = DX_Information::GetInstance();
        
                //SRV 
                CD3DX12_CPU_DESCRIPTOR_HANDLE srvHeapHandle(IObject_Model->TextureSRVHeap->GetCPUDescriptorHandleForHeapStart());
        
                for (UINT i = 0; i < texturebuffer.size(); i++)
                {
                    IDevice->CreateShaderResourceView(texturebuffer[i]->TextureGPU,
                        &texturebuffer[i]->TextureSRV,
                        srvHeapHandle);
                    srvHeapHandle.Offset(1, DXInf->CbvSrvUavDescriptorsize);
                }
    }


}


void BGPU_Resource_Factory::AssignTexture(ID3D12Device* IDevice,
    ID3D12GraphicsCommandList* ICmdList,
    RTexture* IOTexture,
    ID3D12DescriptorHeap* IOsrvHeapHandle,
    int IIndex)
{

    ThrowIfFailed(
        DirectX::CreateDDSTextureFromFile12(
            IDevice,
            ICmdList,
            IOTexture->Filepath.c_str(),
            &IOTexture->TextureGPU,
            &IOTexture->Textureuploader));

    ZeroMemory(&IOTexture->TextureSRV, sizeof(IOTexture->TextureSRV));
    IOTexture->TextureSRV.Texture2D.MipLevels = 1;
    IOTexture->TextureSRV.Texture2D.MostDetailedMip = 0;
    IOTexture->TextureSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // srv FORMAT
    IOTexture->TextureSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    IOTexture->TextureSRV.Format = IOTexture->TextureGPU->GetDesc().Format; // 精度有关

    DX_Information* DXInf = DX_Information::GetInstance();

    //SRV 
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHeapHandle(IOsrvHeapHandle->GetCPUDescriptorHandleForHeapStart());
    srvHeapHandle.Offset(IIndex, DXInf->CbvSrvUavDescriptorsize);

    IDevice->CreateShaderResourceView(IOTexture->TextureGPU,
        &IOTexture->TextureSRV,
        srvHeapHandle);

}


void BGPU_Resource_Factory::UpdateGPUScene(RRender_Scene* IOGPUScene)
{
    UpdateGPUSceneCB(IOGPUScene);
    UpdateGPULightCB(IOGPUScene);

    for (int i = 0 ; i < IOGPUScene->Staticgroup.size();i++)
    {
        auto object = (RObject_Model*)IOGPUScene->Staticgroup[i];
        UpdateGPUMaterials(object);
        UpdateGPUObjectCB(object);
    }

    for (int i = 0; i < IOGPUScene->Skeletongroup.size(); i++)
    {
        auto object = (RObject_Model*)IOGPUScene->Skeletongroup[i];
        UpdateGPUMaterials(object);
        UpdateGPUObjectCB(object);
    }

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

void BGPU_Resource_Factory::UpdateGPULightCB(RRender_Scene* IOGPUScene)
{
    int lightsnum = IOGPUScene->Lightgroup.size();

    for (int i = 0; i < lightsnum; i++)
    {
        IOGPUScene->LightGPU->CopyData(i, IOGPUScene->Lightgroup[i]->Lightdata);
    }
}

void BGPU_Resource_Factory::UpdateGPUSceneCB(RRender_Scene* IOGPUScene)
{
    RRender_Scene::SceneConstants Scenecontants;

    RCamera* Camera = (RCamera*)IOGPUScene->Cameragroup[0];

    //???强制转换？ 
    //摄像机局部坐标系 
    XMMATRIX view = Camera->GetView();
    //投影变换
    XMMATRIX proj = Camera->GetProj();

    //投影变换矩阵 世界到屏幕空间
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);

    //
    XMVECTOR matrixview = XMMatrixDeterminant(view);
    XMVECTOR matrixproj = XMMatrixDeterminant(proj);
    XMVECTOR matrixviewproj = XMMatrixDeterminant(viewProj);
    //逆矩阵
    XMMATRIX invView = XMMatrixInverse(&matrixview, view);
    XMMATRIX invProj = XMMatrixInverse(&matrixproj, proj);
    XMMATRIX invViewProj = XMMatrixInverse(&matrixviewproj, viewProj);

    //Shadow
    RLight::LightNFWVPT lightNFWVPT;
    ((RLight*)IOGPUScene->Lightgroup[0])->GetLightMatrix(IOGPUScene->GetSceneBounds(), lightNFWVPT);
    XMMATRIX shadowTransform = XMLoadFloat4x4(&lightNFWVPT.mShadowTransform);

    //
    XMStoreFloat4x4(&Scenecontants.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&Scenecontants.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&Scenecontants.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&Scenecontants.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&Scenecontants.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&Scenecontants.InvViewProj, XMMatrixTranspose(invViewProj));
    XMStoreFloat4x4(&Scenecontants.ShadowTransform, XMMatrixTranspose(shadowTransform));


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

    IOGPUScene->SceneconstantsGPU->CopyData(0, Scenecontants);

}



void BGPU_Resource_Factory::UpdateGPUMaterials(RObject_Model* IObject_Model)
{
    auto currMaterialbuffer = IObject_Model->GetMaterialConstantsGPU();

    int Matnumber = IObject_Model->CPUMeshdata.Materialgroup.size();
    for (int i = 0; i < Matnumber;i++)//each materials
    {
        auto mat = IObject_Model->CPUMeshdata.Materialgroup[i]->GetMaterialData();

        currMaterialbuffer->CopyData(i, mat);
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