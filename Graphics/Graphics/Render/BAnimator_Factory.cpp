#include "BAnimator_Factory.h"
#include "TMathTool.h"

using namespace DirectX;

void BAnimator_Factory::AnimationCPUUpdate(RSkeleton_Model* IOSkeletonObject, const TTimer& ITimer)
{
	if (IOSkeletonObject->hasAnimation)
	{
		float currenttime = ITimer.TotalTime() * 1000;

		float animationtime = IOSkeletonObject->Animationlength * 1000;

		float second = (long)currenttime % (long)animationtime;

		second /= 1000;

		float frame = second * IOSkeletonObject->Framecount;

		float frameinterpolation = frame - (int)frame;


		auto lVertexCount = IOSkeletonObject->Verticescurrent.size();

		DirectX::XMMATRIX* lClusterDeformation = new DirectX::XMMATRIX[lVertexCount];
		memset(lClusterDeformation, 0, lVertexCount * (unsigned int)sizeof(DirectX::XMMATRIX));

		double* lClusterWeight = new double[lVertexCount];
		memset(lClusterWeight, 0, lVertexCount * sizeof(double));

		//�����ؽ� 
		int jointcount = IOSkeletonObject->Jointsgroup.size();
		std::vector<bool> IsJointRefresh;
		IsJointRefresh.assign(jointcount,false);

		for (int i = 0; i < jointcount; i++)
		{
			//ComputeCurrentPosition(IOSkeletonObject,IsJointRefresh,i);

			if (IOSkeletonObject->Jointsgroup[i].Animation.size() == 0)
			{
				continue;
			}

			//����FBX�л�ȡ���� ÿ֡�ؽڶ�������λ�� �븸ĸ�������
			auto& Jointgroup = IOSkeletonObject->Jointsgroup;
			const auto& Animation = Jointgroup[i].Animation;


			//DirectX ����˷��� FBX����˷�������
			//auto lClusterRelativeCurrentPositionInverse = IOSkeletonObject->GlobalOffPosition[frame] * Animation[frame].ClusterGlobalCurrentPosition;
			//auto pVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * Jointgroup[i].ClusterRelativeInitPosition;

			auto TransformMatrixA = Animation[frame].ClusterGlobalCurrentPosition;
			auto TransformMatrixB = Animation[frame+1].ClusterGlobalCurrentPosition;
			//ʱ���ֵ
			auto pVertexTransformMatrix = MatrixInterpolation(TransformMatrixA, TransformMatrixB, frameinterpolation);

			auto& Affectpointlist = Jointgroup[i].AffectPointList;
			for (int j = 0; j < Affectpointlist.size(); j++)
			{
				int vertexindex = Affectpointlist[j].Pointindex;
				double weight = Affectpointlist[j].Weight;

				if (vertexindex >= lVertexCount)
					continue;

				if (weight == 0.0)
					continue;

				DirectX::XMMATRIX lInfluence = pVertexTransformMatrix;
				lInfluence = lInfluence * weight;
				//�ۼ�Ӱ��
				lClusterDeformation[vertexindex] += lInfluence;
				//�ۼ�Ȩ��
				lClusterWeight[vertexindex] += weight;
			}

		}

		const auto& Vertexgroup = IOSkeletonObject->CPUMeshdata.Vertices;
		auto& Vertexcurrent = IOSkeletonObject->Verticescurrent;

		for (int vertexindex = 0; vertexindex < lVertexCount; vertexindex++)
		{
			double lWeight = lClusterWeight[vertexindex];
			if (lWeight != 0.0)
			{
				DirectX::XMVECTOR vertex = DirectX::XMVectorSet(
					Vertexgroup[vertexindex].Position.x,
					Vertexgroup[vertexindex].Position.y,
					Vertexgroup[vertexindex].Position.z,
					1);

				vertex = XMVector4Transform(vertex, lClusterDeformation[vertexindex]);
				
				float x = XMVectorGetX(vertex);
				float y = XMVectorGetY(vertex);
				float z = XMVectorGetZ(vertex);

				Vertexcurrent[vertexindex].Position = XMFLOAT3(x, y, z);
			}

		}

		delete[] lClusterDeformation;
		delete[] lClusterWeight;
	}
}

void ComputeCurrentPosition(
	RSkeleton_Model* IOSkeletonObject,
	std::vector<bool>& IsJointRefresh,
	int Jointindex)
{
	if (IsJointRefresh[Jointindex])
	{
		return;
	}
	const auto& Vertexgroup = IOSkeletonObject->CPUMeshdata.Vertices;
	auto& Vertexcurrent = IOSkeletonObject->Verticescurrent;
	auto& Jointgroup = IOSkeletonObject->Jointsgroup;

	int ParentJoint = Jointgroup[Jointindex].ParentJoint;
	//���ڵ���Ѹ���
	if (IsJointRefresh[ParentJoint] || ParentJoint == -1)
	{
		//�ؽ�
		

		//����
		auto& Affectpointlist = Jointgroup[Jointindex].AffectPointList;
		for (int i = 0; i < Affectpointlist.size(); i++)
		{
			
		}


	}
	else//���㸸�ڵ�
	{
		ComputeCurrentPosition(IOSkeletonObject, IsJointRefresh, ParentJoint);
	}
}