#ifdef  DLL_GRAPHICS_API
#else
#define DLL_GRAPHICS_API _declspec(dllexport)
#endif

#include "BRenderer.h"

ARenderer* ARenderer_Factory::CreateRenderer()
{
	return new BRenderer();
}