#pragma once 
#include "RObject_Model.h"

//
class RSkeleton_Model :  public RObject_Model//, public ASkeleton_Model
{
public:
	struct Keyframe;
	//状态机

	//影响点
	struct AffectPoint
	{
		int Pointindex = 0;
		double Weight = 0;
	};

	//关节数据joint
	struct Joint
	{
		//关节最初始位置（皮肤与关节相对位置的矩阵）
		//DirectX::XMMATRIX ClusterRelativeInitPosition;
		
		//动画帧（多动画需要map存储）
		std::vector <Keyframe> Animation;

		std::vector<AffectPoint> AffectPointList;

		int ParentJoint = -1;

		~Joint()
		{
			AffectPointList.clear();
			Animation.clear();
		}

	};
	//动作

	//アニメーション
	//每一帧关节的位置
	struct Keyframe 
	{
		
		DirectX::XMMATRIX  ClusterGlobalCurrentPosition;

		Keyframe() 
		{}
	};

	//FBX中主体位置的偏移
	//std::vector<DirectX::XMMATRIX> GlobalOffPosition;

	//动画时间长度（多动画需要map存储）
	float Animationlength;
	int Framecount = 30;

public:
	RSkeleton_Model();

public:

	BGPU_Upload_Resource<GVertex>* CurrentVertexbufferGPU = nullptr;

	std::vector<Joint> Jointsgroup;
	std::vector<GVertex> Verticescurrent;
};