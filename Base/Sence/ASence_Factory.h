#pragma once 
#include "../ABase_Factory.h"
#include "Rendering/BGraphics.h"
#include "GSence.h"


class ASence_Factory : public ABase_Factory<GSence>
{
public:
	void ReadSence(GSence* OSence);

	void NewSence(GSence* OSence);

	void SaveSence(GSence* ISence);

//CreateSence

	void CreateMap(GSence* IOSence);

	void SetGrids(GMap* IOMap,
		const Grid_Vector2 IStartposition,
		const Grid_Vector2 IEndPosition,
		const GGrid* IGrid);

	void CheckMap(GSence* IOSence);
	
//CreateSence

};