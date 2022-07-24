#pragma once 

#include <DirectXMath.h>

#include"Rendering/BGraphics.h"

#define GRID_SIZE 1.0f

enum GRID_TYPE
{
	GRID_TYPE_ENTRY,
	GRID_TYPE_EXIT,
	GRID_TYPE_BORDER,
	GRID_TYPE_NORMAL,
	GRID_TYPE_EMPTY,
	GRID_TYPE_TRAP
};

class GGrid
{
public:
	friend class AGrid_Factory;

	void GetStaticmodel(AStatic_Model* OStaticmodel) const { OStaticmodel = Staticmodel; };
	float GetVelocitybonus() const { return Velocitybonus; };
	GRID_TYPE GetGridType() const { return Type; };
private:
	AStatic_Model* Staticmodel;

	float Velocitybonus = 1.0f;

	GRID_TYPE Type = GRID_TYPE_NORMAL;
	

};