#pragma once 
#include "BGraphics.h"

#include <DirectXCollision.h>

class  RLight : public ALight
{
public:

	struct LightNFWVPT
	{
		float mLightNearZ = 0.0f;
		float mLightFarZ = 0.0f;
		XMFLOAT3 mLightPosW;
		XMFLOAT4X4 mLightView = MathHelper::Identity4x4();
		XMFLOAT4X4 mLightProj = MathHelper::Identity4x4();
		XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();
	};

	void GetLightMatrix(DirectX::BoundingSphere const & ISphereBound, LightNFWVPT & OLightNFWVPT);
};
