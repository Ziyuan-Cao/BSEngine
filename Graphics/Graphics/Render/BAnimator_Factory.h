#pragma once
#include "Resource/Model/RSkeleton_Model.h"


class BAnimator_Factory
{
public:
	void AnimationCPUUpdate(RSkeleton_Model* IOSkeletonObject, const TTimer& ITimer);
};