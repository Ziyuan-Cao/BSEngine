#define DLL_GRAPHICS_API _declspec(dllexport)

#include "BGraphics.h"
#include "BRenderer.h"


DLL_GRAPHICS_API ARenderer* ARenderer_Factory::CreateRenderer()
{
	return new BRenderer();
}