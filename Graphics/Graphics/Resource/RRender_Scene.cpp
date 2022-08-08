#include "RRender_Scene.h"

RRender_Scene::RRender_Scene()
{
	SceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	SceneBounds.Radius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f);
}