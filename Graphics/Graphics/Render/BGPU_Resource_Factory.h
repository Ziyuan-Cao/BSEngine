#pragma once 
#include "Resource/RRender_Scene.h"
#include "Render/Auxiliary/BGPU_Upload_Resource.h"
//为资源管理分配GPU内存资源
//更新资源
//释放资源

class BGPU_Resource_Factory
{
public:
    //模型 变换矩阵？
	void AssignGPUObject(
        ID3D12Device* IDevice,
        ID3D12GraphicsCommandList* ICmdList,
        RObject_Model * IObject_Model);

    void AssignGPUObjects(
        ID3D12Device* IDevice,
        ID3D12GraphicsCommandList* ICmdList,
        RRender_Scene* IOGPUScene);


    void ReleaseGPUObject(
        ID3D12Device* IDevice,
        ID3D12GraphicsCommandList* ICmdList,
        RObject_Model* IObject_Model);
    
    //场景资源
    //模型矩阵集合
    //材质集合
    //灯光集合
    //摄像机集合
    void AssignGPUScene(
        ID3D12Device* IDevice,
        RRender_Scene * IOGPUScene
        );

    void UpdateGPUScene(RRender_Scene* IOGPUScene);
    
    void UpdateGPULightCB(RRender_Scene* IOGPUScene);
    void UpdateGPUSceneCB(RRender_Scene* IOGPUScene);
    void UpdateGPUMaterials(RObject_Model* IObject_Model);
    void UpdateGPUObjectCB(RObject_Model* IObject_Model);
    void UpdateGPUVertexCB( RSkeleton_Model* IObject_Model);

    static ID3D12Resource* CreateDefaultBuffer(
        ID3D12Device* IDevice,
        ID3D12GraphicsCommandList* ICmdList,
        const void* IInitData,
        UINT64 ByteSize,
        ID3D12Resource* OUploadBuffer);

    static ID3DBlob* CompileShader(
        const std::wstring& filename,
        const D3D_SHADER_MACRO* defines,
        const std::string& entrypoint,
        const std::string& target);



};

