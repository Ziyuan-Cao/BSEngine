#ifdef  DLL_GRAPHICS_API
#else
#define DLL_GRAPHICS_API _declspec(dllexport)
#endif
#include "BGraphics.h"
#include "RRender_Scene.h"

ARender_Scene* ARender_Scene_Factory::CreatRenderScene()
{
	return new RRender_Scene();
}




