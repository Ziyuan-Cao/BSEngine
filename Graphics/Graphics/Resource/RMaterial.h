#pragma once 
#include "BGraphics.h"
#include "RTexture.h"
//���GPU���Ϳ��ܺͽṹ�岻ͬ
class  RMaterial : public AMaterial
{
public:
	std::vector<RTexture*> TextureGroup;
	MaterialData GetMaterialData();
};
