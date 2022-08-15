#pragma once 
#include "BGraphics.h"
#include "RTexture.h"
//类的GPU类型可能和结构体不同
class  RMaterial : public AMaterial
{
public:
	std::vector<RTexture*> TextureGroup;
	MaterialData GetMaterialData();
};
