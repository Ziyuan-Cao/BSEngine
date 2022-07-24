#pragma once
#include <fbxsdk.h>
#include <cstdint>
#include <DirectXMath.h>
#include <vector>
#include <map>
class Fbx
{
private:
	FbxManager* lSdkManager;
	FbxScene* lScene;
	void PrintNode(FbxNode* pNode);
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
	void PrintAttribute(FbxNodeAttribute* pAttribute);
	void ProcessNode(FbxNode* pNode);
	bool ProcessMesh(FbxNode* pNode);
	bool ProcessCamera();
	bool ProcessLight(FbxNode* pNode);

	void ProcessSkeletonHeirarchy(FbxNode* rootnode);
	void ProcessSkeletonHeirarchyre(FbxNode* node, int depth, int index, int parentindex);


public:

	FbxMesh* pMesh;
	FbxNode* pMeshNode;
	FbxPose* pPose;
	FbxAnimStack* PAnimStack;
	FbxTakeInfo* PAnimInfo;
	std::vector<int> SkeletonParentmap;
	std::vector<FbxSkeleton*> pSkeletongroup;
	std::map<unsigned long,int> pSkeletongroupIndexmap;
	

	bool ReadFbx(const char* lFilename);
	void ReadVertex(int ctrlPointIndex, DirectX::XMFLOAT3* pVertex);
	void ReadColor(int ctrlPointIndex, int vertexCounter, DirectX::XMFLOAT4* pColor);
	bool ReadUV(int ctrlPointIndex, int textureUVIndex, int uvLayer, DirectX::XMFLOAT2* pUV);
	void ReadNormal(int ctrlPointIndex, int vertexCounter, DirectX::XMFLOAT3* pNormal);
	void ReadTangent(int ctrlPointIndex, int vertecCounter, DirectX::XMFLOAT3* pTangent);

	void GetAnimationStack();

	void PrintFbx();
	
};