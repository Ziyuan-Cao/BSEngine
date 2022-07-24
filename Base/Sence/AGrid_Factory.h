#pragma once 
#include "../ABase_Factory.h"
#include "GGrid.h"
class AGrid_Factory : public ABase_Factory<GGrid>
{
public:
	void NewGrid(GGrid * OGrid);

	void ReadGrid(GGrid* OGrid);

	void SaveGrid(GGrid* IGrid);

	void SetAttribe(GGrid* IOGrid);
};