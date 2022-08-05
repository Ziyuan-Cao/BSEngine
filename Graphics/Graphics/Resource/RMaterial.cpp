#ifdef  DLL_GRAPHICS_API
#else
#define DLL_GRAPHICS_API _declspec(dllexport)
#endif
#include "RMaterial.h"

//DLL_GRAPHICS_API void AMaterial::SetLightColor(DirectX::XMFLOAT3 ILightcolor)
//{
//	Materialdata.Lightcolor = ILightcolor;
//}

AMaterial::MaterialData RMaterial::GetMaterialData()
{
	return Materialdata;
}