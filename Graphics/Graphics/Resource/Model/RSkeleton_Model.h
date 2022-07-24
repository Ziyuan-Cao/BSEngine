#pragma once 
#include "RObject_Model.h"

//
class RSkeleton_Model :  public RObject_Model//, public ASkeleton_Model
{
public:
	struct Keyframe;
	//״̬��

	//Ӱ���
	struct AffectPoint
	{
		int Pointindex = 0;
		double Weight = 0;
	};

	//�ؽ�����joint
	struct Joint
	{
		//�ؽ����ʼλ�ã�Ƥ����ؽ����λ�õľ���
		//DirectX::XMMATRIX ClusterRelativeInitPosition;
		
		//����֡���ද����Ҫmap�洢��
		std::vector <Keyframe> Animation;

		std::vector<AffectPoint> AffectPointList;

		int ParentJoint = -1;

		~Joint()
		{
			AffectPointList.clear();
			Animation.clear();
		}

	};
	//����

	//���˥�`�����
	//ÿһ֡�ؽڵ�λ��
	struct Keyframe 
	{
		
		DirectX::XMMATRIX  ClusterGlobalCurrentPosition;

		Keyframe() 
		{}
	};

	//FBX������λ�õ�ƫ��
	//std::vector<DirectX::XMMATRIX> GlobalOffPosition;

	//����ʱ�䳤�ȣ��ද����Ҫmap�洢��
	float Animationlength;
	int Framecount = 30;

public:
	RSkeleton_Model();

public:

	BGPU_Upload_Resource<GVertex>* CurrentVertexbufferGPU = nullptr;

	std::vector<Joint> Jointsgroup;
	std::vector<GVertex> Verticescurrent;
};