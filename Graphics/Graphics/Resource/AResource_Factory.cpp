#include "BGraphics.h"
#include "RTexture.h"
#include "Model/RStatic_Model.h"
#include "Model/RSkeleton_Model.h"
#include "RCamera.h"
#include "RMaterial.h"
#include "RLight.h"

#include "Auxiliary/Fbx.h"
#include <string>

bool GetAnimationData(RSkeleton_Model* IOSkeletonmodel, Fbx* IFbx);

bool AResource_Factory::LoadData()
{
	return true;
}                                                                                                                                                                                                                                                                                                                                                                             

AObject_Model* AResource_Factory::CreateSkeletonModel()
{
	return new RSkeleton_Model();
}

AObject_Model* AResource_Factory::CreateStaticModel()
{
	return new RStatic_Model();
}

ATexture* AResource_Factory::CreateTexture()
{
	return new RTexture();
}

ACamera* AResource_Factory::CreateCamera()
{
	return new RCamera();
}

ALight* AResource_Factory::CreateLight()
{
	return new RLight();
}

AMaterial* AResource_Factory::CreateMaterial()
{
	return new RMaterial();
}

std::wstring StringToWString(const std::string& str) {
    int num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wide = new wchar_t[num];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, num);
    std::wstring w_str(wide);
    delete[] wide;
    return w_str;
}

std::string WstringToString(std::wstring wstr)
{
    std::string result;
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    char* buffer = new char[len + 1];
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
    buffer[len] = '\0';
    result.append(buffer);
    delete[] buffer;
    return result;
}

std::string GetFbxFile(std::wstring DirPath)
{
    HANDLE hFind;
    WIN32_FIND_DATA data;
    std::string res;
    std::wstring SearchName = DirPath + L"*.fbx";
    int i = 0;
    hFind = FindFirstFile(SearchName.c_str(), &data);
    if (hFind != INVALID_HANDLE_VALUE) {

        res = WstringToString(DirPath + data.cFileName);
        FindClose(hFind);
    }
    return res;
}

bool AResource_Factory::LoadFbx(AObject_Model* IOObjectmodel, std::wstring IFilename)
{

    Fbx myFbx;
    std::string Fbxpath = GetFbxFile(IFilename);
    if (Fbxpath.empty()) return false;
    myFbx.ReadFbx(Fbxpath.c_str());

    RObject_Model* Objectmodel = (RObject_Model*)IOObjectmodel;

    auto& vertices = Objectmodel->CPUMeshdata.Vertices;
    auto& indices = Objectmodel->CPUMeshdata.Indices;
    XMVECTOR& vMin = Objectmodel->CPUMeshdata.VMin;
    XMVECTOR& vMax = Objectmodel->CPUMeshdata.VMax;
    //std::vector<int>& verteices_offset = Objectmodel->CPUMeshdata.Verteicesoffset;
    std::vector<int>& indeices_offset = Objectmodel->CPUMeshdata.Indeicesoffset;
    int& materialnums = Objectmodel->CPUMeshdata.Materialnums;
    BoundingBox& bounds = Objectmodel->CPUMeshdata.Bounds;

    using GeoElement = FbxGeometryElement;
    using LayerElement = FbxLayerElement;
    const int polygonCount = myFbx.pMesh->GetPolygonCount();
    auto controlPoints = myFbx.pMesh->GetControlPoints();
    const int controlPointCount = myFbx.pMesh->GetControlPointsCount();
    fbxsdk::FbxLayerElementArrayTemplate<int>* pMaterialIndices = &myFbx.pMesh->GetElementMaterial()->GetIndexArray();

    int etc = 0;

    bool HasNormal = myFbx.pMesh->GetElementNormalCount() > 0;
    bool HasUV = myFbx.pMesh->GetElementUVCount() > 0;
    bool NormalAllByControlPoint = true;
    bool UVAllByControlPoint = true;
    const FbxGeometryElementNormal* lNormalElement = NULL;
    const FbxGeometryElementUV* lUVElement = NULL;
    FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
    FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;

    FbxVector4 lCurrentNormal;
    FbxVector2 lCurrentUV;
    //需要区分 controlpoint 和 vertice
    if (HasNormal)
    {
        lNormalMappingMode = myFbx.pMesh->GetElementNormal(0)->GetMappingMode();
        lNormalElement = myFbx.pMesh->GetElementNormal(0);
        HasNormal = lNormalMappingMode != FbxGeometryElement::eNone;
        NormalAllByControlPoint = HasNormal && (lNormalMappingMode == FbxGeometryElement::eByControlPoint);
    }
    if (HasUV)
    {
        lUVMappingMode = myFbx.pMesh->GetElementUV(0)->GetMappingMode();
        lUVElement = myFbx.pMesh->GetElementUV(0);
        HasUV = lUVMappingMode != FbxGeometryElement::eNone;
        UVAllByControlPoint = HasUV && (lUVMappingMode == FbxGeometryElement::eByControlPoint);
    }

    vertices.assign(controlPointCount, GVertex());
    for(int i = 0; i < controlPointCount; i++)
    {
        vertices[i].Position = XMFLOAT3(
            controlPoints[i].mData[0],
            controlPoints[i].mData[1],
            controlPoints[i].mData[2]
        );

        // Save the normal.
        if (NormalAllByControlPoint)
        {
            int lNormalIndex = i;
            if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
            {
                lNormalIndex = lNormalElement->GetIndexArray().GetAt(i);
            }
            lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
            vertices[i].Normal = XMFLOAT3(
                static_cast<float>(lCurrentNormal.mData[0]),
                static_cast<float>(lCurrentNormal.mData[1]),
                static_cast<float>(lCurrentNormal.mData[2])
            );
        }

        // Save the UV.
        if (UVAllByControlPoint)
        {
            int lUVIndex = i;
            if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
            {
                lUVIndex = lUVElement->GetIndexArray().GetAt(i);
            }
            lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
            vertices[i].TexC = XMFLOAT2(
                static_cast<float>(lCurrentUV.mData[0]),
                static_cast<float>(lCurrentUV.mData[1])
            );
        }
    }

    int PolygonType = myFbx.pMesh->GetPolygonSize(0);
    indices.assign((int)polygonCount * PolygonType, std::int32_t());
    //Objectmodel->SetPrimitiveType(PolygonType);
    int IndeicesCount = 0;
    for (int polygon = 0; polygon < polygonCount; polygon++) //each polygon
    {
        const int polyVertCount = myFbx.pMesh->GetPolygonSize(polygon);
        for (int polyVert = 0; polyVert < polyVertCount; polyVert++) //each Index
        {
            const int cpIndex = myFbx.pMesh->GetPolygonVertex(polygon, polyVert);
            
            //indices.push_back(cpIndex);
            indices[IndeicesCount] = cpIndex;

            if (!UVAllByControlPoint)
            {
                const int uvElementsCount = myFbx.pMesh->GetElementUVCount();
                for (int uvElement = 0; uvElement < uvElementsCount; uvElement++)
                {
                    const FbxGeometryElementUV* geometryElementUV = myFbx.pMesh->GetElementUV(uvElement);
                    const auto mapMode = geometryElementUV->GetMappingMode();
                    const auto refMode = geometryElementUV->GetReferenceMode();
                    int directIndex = -1;
                    if (GeoElement::eByControlPoint == mapMode)
                    {
                        if (GeoElement::eDirect == refMode) directIndex = cpIndex;
                        else if (GeoElement::eIndexToDirect == refMode)
                            directIndex = geometryElementUV->GetIndexArray().GetAt(cpIndex);
                    }
                    else if (
                        GeoElement::eByPolygonVertex == mapMode
                        && (GeoElement::eDirect == refMode || FbxGeometryElement::eIndexToDirect == refMode)
                        )
                    {
                        directIndex = myFbx.pMesh->GetTextureUVIndex(polygon, polyVert);
                    }
                    if (directIndex == -1) continue;
                    FbxVector2 uv = geometryElementUV->GetDirectArray().GetAt(directIndex);
                    //UV
                    vertices[cpIndex].TexC = XMFLOAT2(
                        static_cast<float>(uv.mData[0]),
                        static_cast<float>(uv.mData[1])
                    );
                }
            }
            if (!NormalAllByControlPoint)
            {
                const int normalElementCount = myFbx.pMesh->GetElementNormalCount();
                for (int normalElement = 0; normalElement < normalElementCount; normalElement++)
                {
                    const FbxGeometryElementNormal* geometryElementNormal = myFbx.pMesh->GetElementNormal(normalElement);
                    const LayerElement::EMappingMode mapMode = geometryElementNormal->GetMappingMode();
                    const LayerElement::EReferenceMode refMode = geometryElementNormal->GetReferenceMode();
                    int directIndex = -1;
                    if (GeoElement::eByPolygonVertex == mapMode || GeoElement::eByControlPoint == mapMode)
                    {
                        if (GeoElement::eDirect == refMode) directIndex = IndeicesCount;
                        else if (GeoElement::eIndexToDirect == refMode)
                            directIndex = geometryElementNormal->GetIndexArray().GetAt(IndeicesCount);
                    }
                    if (directIndex == -1) continue;
                    FbxVector4 norm = geometryElementNormal->GetDirectArray().GetAt(directIndex);
                    //NORMAL
                    vertices[cpIndex].Normal = XMFLOAT3(
                        static_cast<float>(norm.mData[0]),
                        static_cast<float>(norm.mData[1]),
                        static_cast<float>(norm.mData[2])
                    );
                }
            }

            XMVECTOR P = XMLoadFloat3(&vertices[cpIndex].Position);

            XMVECTOR N = XMLoadFloat3(&vertices[cpIndex].Normal);

            XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            if (fabsf(XMVectorGetX(XMVector3Dot(N, up))) < 1.0f - 0.001f)
            {
                XMVECTOR T = XMVector3Normalize(XMVector3Cross(up, N));
                XMStoreFloat3(&vertices[cpIndex].TangentU, T);
            }
            else
            {
                up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
                XMVECTOR T = XMVector3Normalize(XMVector3Cross(N, up));
                XMStoreFloat3(&vertices[cpIndex].TangentU, T);
            }

            vMin = XMVectorMin(vMin, P);
            vMax = XMVectorMax(vMax, P);

            vertices[cpIndex].MaterialId.x = pMaterialIndices->GetAt(polygon);
            if (vertices[cpIndex].MaterialId.x != etc)
            {
                etc = vertices[cpIndex].MaterialId.x;

                indeices_offset.push_back(IndeicesCount);
                materialnums++;
            }
            IndeicesCount++;
        }
    }

    materialnums++;
    indeices_offset.push_back(IndeicesCount);

    XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
    XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));

    IOObjectmodel->Materialnums = materialnums;


    {
        ////将所有顶点按索引顺序遍历了一遍（导致顶点按照索引重复了）
        //int PolygonType = myFbx.pMesh->GetPolygonSize(0);
        //int VerticesCount = (int)polygonCount * PolygonType;
        //Objectmodel->SetPrimitiveType(PolygonType);

        //vertices.assign(VerticesCount, GVertex());
        //indices.assign((int)polygonCount * PolygonType, std::int32_t());
        ////TriangleMtlIndex.assign(polygonCount, 0);
        //int vertexID = 0;
        //for (int polygon = 0; polygon < polygonCount; polygon++) //each polygon
        //{
        //    const int polyVertCount = myFbx.pMesh->GetPolygonSize(polygon);

        //    for (int polyVert = 0; polyVert < polyVertCount; polyVert++) //each Vertex
        //    {
        //        const int cpIndex = myFbx.pMesh->GetPolygonVertex(polygon, polyVert);
        //        //vertex
        //        vertices[vertexID].Position = XMFLOAT3(
        //            controlPoints[cpIndex].mData[0],
        //            controlPoints[cpIndex].mData[1],
        //            controlPoints[cpIndex].mData[2]
        //        );
        //        //index
        //        indices[vertexID] = vertexID;


        //        const int uvElementsCount = myFbx.pMesh->GetElementUVCount();
        //        for (int uvElement = 0; uvElement < uvElementsCount; uvElement++)
        //        {
        //            const FbxGeometryElementUV* geometryElementUV = myFbx.pMesh->GetElementUV(uvElement);
        //            const auto mapMode = geometryElementUV->GetMappingMode();
        //            const auto refMode = geometryElementUV->GetReferenceMode();
        //            int directIndex = -1;
        //            if (GeoElement::eByControlPoint == mapMode)
        //            {
        //                if (GeoElement::eDirect == refMode) directIndex = cpIndex;
        //                else if (GeoElement::eIndexToDirect == refMode)
        //                    directIndex = geometryElementUV->GetIndexArray().GetAt(cpIndex);
        //            }
        //            else if (
        //                GeoElement::eByPolygonVertex == mapMode
        //                && (GeoElement::eDirect == refMode || FbxGeometryElement::eIndexToDirect == refMode)
        //                )
        //            {
        //                directIndex = myFbx.pMesh->GetTextureUVIndex(polygon, polyVert);
        //            }
        //            if (directIndex == -1) continue;
        //            FbxVector2 uv = geometryElementUV->GetDirectArray().GetAt(directIndex);
        //            //UV
        //            vertices[vertexID].TexC = XMFLOAT2(
        //                static_cast<float>(uv.mData[0]),
        //                static_cast<float>(uv.mData[1])
        //            );
        //        }

        //        const int normalElementCount = myFbx.pMesh->GetElementNormalCount();
        //        for (int normalElement = 0; normalElement < normalElementCount; normalElement++)
        //        {
        //            const FbxGeometryElementNormal* geometryElementNormal = myFbx.pMesh->GetElementNormal(normalElement);
        //            const LayerElement::EMappingMode mapMode = geometryElementNormal->GetMappingMode();
        //            const LayerElement::EReferenceMode refMode = geometryElementNormal->GetReferenceMode();
        //            int directIndex = -1;
        //            if (GeoElement::eByPolygonVertex == mapMode)
        //            {
        //                if (GeoElement::eDirect == refMode) directIndex = vertexID;
        //                else if (GeoElement::eIndexToDirect == refMode)
        //                    directIndex = geometryElementNormal->GetIndexArray().GetAt(vertexID);
        //            }
        //            if (directIndex == -1) continue;
        //            FbxVector4 norm = geometryElementNormal->GetDirectArray().GetAt(directIndex);
        //            //NORMAL
        //            vertices[vertexID].Normal = XMFLOAT3(
        //                static_cast<float>(norm.mData[0]),
        //                static_cast<float>(norm.mData[1]),
        //                static_cast<float>(norm.mData[2])
        //            );
        //        }

        //        XMVECTOR P = XMLoadFloat3(&vertices[vertexID].Position);

        //        XMVECTOR N = XMLoadFloat3(&vertices[vertexID].Normal);

        //        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        //        if (fabsf(XMVectorGetX(XMVector3Dot(N, up))) < 1.0f - 0.001f)
        //        {
        //            XMVECTOR T = XMVector3Normalize(XMVector3Cross(up, N));
        //            XMStoreFloat3(&vertices[vertexID].TangentU, T);
        //        }
        //        else
        //        {
        //            up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        //            XMVECTOR T = XMVector3Normalize(XMVector3Cross(N, up));
        //            XMStoreFloat3(&vertices[vertexID].TangentU, T);
        //        }

        //        vMin = XMVectorMin(vMin, P);
        //        vMax = XMVectorMax(vMax, P);

        //        //TriangleMtlIndex[triangleIndex] = materialIndex;
        //        vertices[vertexID].MaterialId.x = pMaterialIndices->GetAt(polygon);
        //        if (vertices[vertexID].MaterialId.x != etc)
        //        {
        //            etc = vertices[vertexID].MaterialId.x;
        //            verteices_offset.push_back(vertexID);
        //            indeices_offset.push_back(vertexID);
        //            materialnums++;
        //        }
        //        vertexID++;
        //    }
        //    //number of material types 

        //}

        //materialnums++;
        //verteices_offset.push_back(vertexID);
        //indeices_offset.push_back(vertexID);

        //XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
        //XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));

        //IOObjectmodel->Materialnums = materialnums;
    }
    if (IOObjectmodel->hasAnimation)
    {
        auto* Skeletonmodel = (RSkeleton_Model*)IOObjectmodel;
        GetAnimationData(Skeletonmodel, &myFbx);

        //多做一个动画顶点缓存
        Skeletonmodel->Verticescurrent.assign(controlPointCount,GVertex());
        
        const UINT vbByteSize = (UINT)Objectmodel->CPUMeshdata.Vertices.size() * sizeof(GVertex);
        void* StaVectorptr = Objectmodel->CPUMeshdata.Vertices.data();
        void* SkeVectorptr = Skeletonmodel->Verticescurrent.data();
        CopyMemory(SkeVectorptr, StaVectorptr, vbByteSize);

    }

    return true;
}

void ConvertFBXMatrix(FbxAMatrix& IFBXMatrix, DirectX::XMMATRIX& IODIRMatrix)
{
    IODIRMatrix = DirectX::XMMATRIX(
        IFBXMatrix.Get(0, 0), IFBXMatrix.Get(0, 1), IFBXMatrix.Get(0, 2),IFBXMatrix.Get(0, 3),
        IFBXMatrix.Get(1, 0), IFBXMatrix.Get(1, 1), IFBXMatrix.Get(1, 2), IFBXMatrix.Get(1, 3), 
        IFBXMatrix.Get(2, 0), IFBXMatrix.Get(2, 1), IFBXMatrix.Get(2, 2), IFBXMatrix.Get(2, 3), 
        IFBXMatrix.Get(3, 0), IFBXMatrix.Get(3, 1), IFBXMatrix.Get(3, 2), IFBXMatrix.Get(3, 3));

}


// Get the geometry offset to a node. It is never inherited by the children.
FbxAMatrix GetGeometry(FbxNode* pNode)
{
    const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(lT, lR, lS);
}

void ReadNode(RSkeleton_Model* IOSkeletonmodel,
    Fbx* IFbx, 
    FbxNode* pNode)
{
    FbxMesh* IFbxMesh = pNode->GetMesh();
    auto& IOSkeletongroupIndexmap = IFbx->pSkeletongroupIndexmap;
    auto& IParentIndexmap = IFbx->SkeletonParentmap;
    FbxAnimStack* IAnimStack = IFbx->PAnimStack;
    FbxTakeInfo* IAnimInfo = IFbx->PAnimInfo;

    const bool lHasShape = IFbxMesh->GetShapeCount() > 0;

    if (lHasShape)
    {//形状变化

    }
    //皮肤变化
    //FbxAMatrix geometryTransform = GetGeometry(IFbxMesh->GetNode());

    const int lSkinCount = IFbxMesh->GetDeformerCount(FbxDeformer::eSkin);

    //遍历皮肤计算位置
    for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
    {
        FbxSkin* lSkin = (FbxSkin*)(IFbxMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin));
        //遍历影响该皮肤的组
        int lClusterCount = lSkin->GetClusterCount();
        for (int lclusterIndex = 0; lclusterIndex < lClusterCount; lclusterIndex++)
        {
            FbxCluster* lCluster = lSkin->GetCluster(lclusterIndex);
            if (!lCluster)
            {
                continue;
            }
            //FBX的skeleton组
            FbxNode* lLinkNode = lCluster->GetLink();
            if (!lLinkNode)
            {
                continue;
            }

            unsigned long test = lLinkNode->GetUniqueID();
            int joinIndex = IOSkeletongroupIndexmap[lLinkNode->GetUniqueID()]-1;
            int associatedCtrlPointCount = lCluster->GetControlPointIndicesCount();
            int* lCtrlPointIndices = lCluster->GetControlPointIndices();
            double* lCtrlPointWeights = lCluster->GetControlPointWeights();
            int ctrlPointIndex;

            //构建自己的Joints，添加影响点和父母索引
            IOSkeletonmodel->Jointsgroup[joinIndex].AffectPointList.assign(associatedCtrlPointCount, RSkeleton_Model::AffectPoint());
            IOSkeletonmodel->Jointsgroup[joinIndex].ParentJoint = IParentIndexmap[joinIndex];

            //影响点Associate Vertices
            for (int j = 0; j < associatedCtrlPointCount; j++)
            {
                IOSkeletonmodel->Jointsgroup[joinIndex].AffectPointList[j].Pointindex = lCtrlPointIndices[j];
                IOSkeletonmodel->Jointsgroup[joinIndex].AffectPointList[j].Weight = lCtrlPointWeights[j];
            }

            //皮肤矩阵和关节矩阵Matrix

            FbxAMatrix lReferenceGlobalInitPosition;
            FbxAMatrix lClusterGlobalInitPosition;

            FbxAMatrix lReferenceGeometry;
            FbxAMatrix lClusterRelativeInitPosition;

            //骨骼初始世界坐标
            lCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
            //物体初始世界坐标
            lReferenceGeometry = GetGeometry(IFbxMesh->GetNode()); //Geometric Transformation
            //骨骼真实初始世界坐标
            lReferenceGlobalInitPosition *= lReferenceGeometry;
            //皮肤世界初始绑定变换矩阵
            lCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
            //皮肤相对位置矩阵 = 皮肤世界初始绑定变换矩阵-1 * 骨骼世界变换矩阵
            lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

            //每帧关节的位置
            FbxTime start = IAnimInfo->mLocalTimeSpan.GetStart();
            FbxTime end = IAnimInfo->mLocalTimeSpan.GetStop();
            FbxLongLong animationlength = end.GetFrameCount(FbxTime::eFrames30) - start.GetFrameCount(FbxTime::eFrames30) + 1;
            auto& anim = IOSkeletonmodel->Jointsgroup[joinIndex].Animation;
            anim.assign(animationlength, RSkeleton_Model::Keyframe());
            for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames30); i <= end.GetFrameCount(FbxTime::eFrames30); ++i)
            {
                //each second's tranformMatrix of Joint
                FbxTime time;
                time.SetFrame(i, FbxTime::eFrames30);
                //关节世界坐标
                FbxAMatrix lReferenceGlobalCurrentPosition = lLinkNode->EvaluateGlobalTransform(time);
                //主体偏移世界坐标
                FbxAMatrix lGlobalCurrentPosition = pNode->EvaluateGlobalTransform(time);
                lGlobalCurrentPosition = lGlobalCurrentPosition * lReferenceGeometry;
                //真正的关节世界坐标
                lReferenceGlobalCurrentPosition = lGlobalCurrentPosition * lReferenceGlobalCurrentPosition;
                //皮肤世界矩阵 = 真正的关节世界坐标 * 皮肤相对位置矩阵
                FbxAMatrix lVertexTransformMatrix = lReferenceGlobalCurrentPosition * lClusterRelativeInitPosition;
                //保存皮肤世界矩阵
                ConvertFBXMatrix(lVertexTransformMatrix, anim[i].ClusterGlobalCurrentPosition);
            }

            //保存皮肤相对位置矩阵
            //ConvertFBXMatrix(lClusterRelativeInitPosition, IOSkeletonmodel->Jointsgroup[joinIndex].ClusterRelativeInitPosition);
        }
    }
}

void ReadNodeRecursive(
    RSkeleton_Model* IOSkeletonmodel,
    Fbx* IFbx,
    FbxNode* pNode/*,FbxAMatrix& pParentGlobalPosition*/)
{
    FbxTakeInfo* IAnimInfo = IFbx->PAnimInfo;

    //记录每帧主体位置偏移
    FbxAMatrix lGeometryOffset = GetGeometry(pNode);

    FbxTime start = IAnimInfo->mLocalTimeSpan.GetStart();
    FbxTime end = IAnimInfo->mLocalTimeSpan.GetStop();
    FbxLongLong animationlength = end.GetFrameCount(FbxTime::eFrames30) - start.GetFrameCount(FbxTime::eFrames30) + 1;
    //IOSkeletonmodel->GlobalOffPosition.assign(animationlength, DirectX::XMMATRIX());
    
    int hour;
    int minute;
    int second;
    int frame;
    int field;
    int residual;

    end.GetTime(hour, minute, second, frame, field, residual);

    IOSkeletonmodel->Animationlength = hour * 3600 + minute * 60 + second + (float)frame / 60.0;

    IOSkeletonmodel->Framecount = 30;

    //for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames30); i <= end.GetFrameCount(FbxTime::eFrames30); ++i)
    //{
    //    //each second's tranformMatrix of Joint
    //    FbxTime time;
    //    time.SetFrame(i, FbxTime::eFrames30);
    //    FbxAMatrix lGlobalPosition = pNode->EvaluateGlobalTransform(time);
    //    FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;
    //    lGlobalOffPosition = lGlobalOffPosition.Inverse();
    //    //保存物体变换矩阵
    //    ConvertFBXMatrix(lGlobalOffPosition, IOSkeletonmodel->GlobalOffPosition[i]);
    //}

    if (pNode->GetNodeAttribute())
    {
        ReadNode(IOSkeletonmodel, IFbx, pNode);
    }

    //暂时没必要遍历scene所有的node
    //const int lChildCount = pNode->GetChildCount();
    //for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
    //{
    //    ReadNodeRecursive(IOSkeletonmodel, IFbx, pNode->GetChild(lChildIndex) /*lGlobalPosition*/);
    //}
}


//1.需要关节位置 Skeleton
//2.需要皮肤对关节的矩阵位置 Cluster
//3.需要关节变化数据  Animation Stack
bool GetAnimationData(RSkeleton_Model* IOSkeletonmodel,Fbx* IFbx)
{

    FbxMesh* IFbxMesh = IFbx->pMesh;
    FbxAnimStack* IAnimStack = IFbx->PAnimStack;
    FbxTakeInfo* IAnimInfo = IFbx->PAnimInfo;
    auto& IOSkeletongroupIndexmap = IFbx->pSkeletongroupIndexmap;
    auto& IParentIndexmap = IFbx->SkeletonParentmap;

    const int lVertexCount = IFbxMesh->GetControlPointsCount();

    // No vertex to draw.
    if (lVertexCount == 0)
    {
        return false;
    }

    //检查有无动画
    const bool lHasVertexCache = IFbxMesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
        (static_cast<FbxVertexCacheDeformer*>(IFbxMesh->GetDeformer(0, FbxDeformer::eVertexCache)))->Active.Get();
    const bool lHasShape = IFbxMesh->GetShapeCount() > 0;
    const bool lHasSkin = IFbxMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
    const bool lHasDeformation = lHasVertexCache || lHasShape || lHasSkin;

    //有动画
    if (lHasDeformation)
    {
        if (lHasVertexCache)
        {//顶点缓存（帧动画）//？？？没返回结果
            //ReadVertexCacheData(IFbxMesh);
        }
        else
        {//计算顶点位置

            IOSkeletonmodel->Jointsgroup.assign(IFbx->pSkeletongroup.size(), RSkeleton_Model::Joint());

            ReadNodeRecursive(IOSkeletonmodel, IFbx, IFbx->pMeshNode);
        }

        return true;
    }

    return false;

}

void ReadVertexCacheData(FbxMesh* pMesh)
{
    FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(pMesh->GetDeformer(0, FbxDeformer::eVertexCache));
    FbxCache* lCache = lDeformer->GetCache();
    int                     lChannelIndex = lCache->GetChannelIndex(lDeformer->Channel.Get());
    unsigned int            lVertexCount = (unsigned int)pMesh->GetControlPointsCount();
    bool                    lReadSucceed = false;
    float* lReadBuf = NULL;
    unsigned int			BufferSize = 0;

    if (lDeformer->Type.Get() != FbxVertexCacheDeformer::ePositions)
        // only process positions
        return;

    unsigned int Length = 0;
    lCache->Read(NULL, Length, FBXSDK_TIME_ZERO, lChannelIndex);
    if (Length != lVertexCount * 3)
        // the content of the cache is by vertex not by control points (we don't support it here)
        return;

    {//??? 按照PointCount读取所有帧数据
        unsigned int lPointCount = lCache->GetPointCount();
        std::vector<FbxVector4*> OVertexArray;
        OVertexArray.assign(lPointCount, nullptr);

        for (int i = 0; i < lPointCount; i++)
        {
            unsigned int lReadBufIndex = 0;
            OVertexArray[i] = new FbxVector4[lVertexCount];
            while (lReadBufIndex < 3 * lVertexCount)
            {
                OVertexArray[i][lReadBufIndex / 3].mData[0] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
                OVertexArray[i][lReadBufIndex / 3].mData[1] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
                OVertexArray[i][lReadBufIndex / 3].mData[2] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
            }
        }
    }
}

//？？？
void ReadShapeDeformation(FbxMesh* pMesh)
{
    //int lVertexCount = pMesh->GetControlPointsCount();

    //int lBlendShapeDeformerCount = pMesh->GetDeformerCount(FbxDeformer::eBlendShape);
    //for (int lBlendShapeIndex = 0; lBlendShapeIndex < lBlendShapeDeformerCount; ++lBlendShapeIndex)
    //{
    //    FbxBlendShape* lBlendShape = (FbxBlendShape*)pMesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);
    //    int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
    //    for (int lChannelIndex = 0; lChannelIndex < lBlendShapeChannelCount; ++lChannelIndex)
    //    {
    //        FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
    //        if (lChannel)
    //        {
    //            // Get the percentage of influence on this channel.
    //            FbxAnimCurve* lFCurve = pMesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, pAnimLayer);
    //            if (!lFCurve) continue;

    //            int lShapeCount = lChannel->GetTargetShapeCount();
    //            double* lFullWeights = lChannel->GetTargetShapeFullWeights();

    //            int lStartIndex = -1;
    //            int lEndIndex = -1;
    //            for (int lShapeIndex = 0; lShapeIndex < lShapeCount; ++lShapeIndex)
    //            {

    //            }

    //        }
    //    }
    //}
}

FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
    FbxAMatrix lPoseMatrix;
    FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

    memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

    return lPoseMatrix;
}

FbxAMatrix GetGlobalPosition(
    FbxNode* pNode, 
    const FbxTime& pTime, 
    FbxPose* pPose = NULL,
    FbxAMatrix* pParentGlobalPosition = NULL)
{
    FbxAMatrix lGlobalPosition;
    bool        lPositionFound = false;

    if (pPose)
    {
        int lNodeIndex = pPose->Find(pNode);

        if (lNodeIndex > -1)
        {
            // The bind pose is always a global matrix.
            // If we have a rest pose, we need to check if it is
            // stored in global or local space.
            if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
            {
                lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
            }
            else
            {
                // We have a local matrix, we need to convert it to
                // a global space matrix.
                FbxAMatrix lParentGlobalPosition;

                if (pParentGlobalPosition)
                {
                    lParentGlobalPosition = *pParentGlobalPosition;
                }
                else
                {
                    if (pNode->GetParent())
                    {
                        lParentGlobalPosition = GetGlobalPosition(pNode->GetParent(), pTime, pPose);
                    }
                }

                FbxAMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
                lGlobalPosition = lParentGlobalPosition * lLocalPosition;
            }

            lPositionFound = true;
        }
    }

    if (!lPositionFound)
    {
        // There is no pose entry for that node, get the current global position instead.

        // Ideally this would use parent global position and local position to compute the global position.
        // Unfortunately the equation 
        //    lGlobalPosition = pParentGlobalPosition * lLocalPosition
        // does not hold when inheritance type is other than "Parent" (RSrs).
        // To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
        lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
    }

    return lGlobalPosition;
}



bool AResource_Factory::AddMaterial(AObject_Model* IOObjectmodel, std::vector<AMaterial*>& IMaterial)
{
    RObject_Model* Objectmodel = (RObject_Model*)IOObjectmodel;
    int Matnums = Objectmodel->CPUMeshdata.Materialnums;

    if (IMaterial.size() != Matnums)
    {
        return false;
    }

    auto& Matgroup = Objectmodel->CPUMeshdata.Materialgroup;
    Matgroup.assign(Matnums,nullptr);
    for (int i = 0; i < Matnums; i++)
    {
        Matgroup[i] = (RMaterial*)IMaterial[i];
    }
    return true;
}