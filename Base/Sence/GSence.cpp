#include "GSence.h"
#include "AGrid_Factory.h"
#include <utility>

GMap::GMap(int IXSize,int IYSize)
{
	XSize = IXSize;
	YSize = IYSize;

	GGrid* Gridinit = nullptr;

	AGrid_Factory Gridfactory;
	Gridfactory.NewGrid(Gridinit);

	Gridgroup.assign(IYSize, std::vector<const GGrid* >(IXSize, Gridinit));

	Entryposition[0] = 0;
	Entryposition[1] = 0;

	Exitposition[0] = IXSize - 1;
	Exitposition[1] = IYSize - 1;

}

bool GMap::SetGrids(const Grid_Vector2 IStartposition,
	const Grid_Vector2 IEndPosition,
	const GGrid* IGrid)
{
	if( XSize < max(IStartposition.X, IEndPosition.X)
		|| YSize < max(IStartposition.Y, IEndPosition.Y)
		|| 0 > min(IStartposition.X, IEndPosition.X)
		|| 0 > min(IStartposition.Y, IEndPosition.Y))
	{
		return false;
	}

	for(int i = IStartposition.X; i < IEndPosition.X; i++)
	{
		for(int j = IStartposition.Y; j < IEndPosition.Y; j++)
		{
			Gridgroup[i][j] = IGrid;
		}
	}
	return true;
}


void GSence::InsertMap(GMap* IMap)
{
	Mapgroup.push_back(std::move(IMap));
	IMap = nullptr;
}

bool GSence::GetMapList(std::list<GMap*>* OMapgroup)
{
	OMapgroup = &Mapgroup;
	return true;
}

bool GSence::GetMap(GMap* IMap, int index)
{
	if (index > Mapgroup.size()-1)
	{
		IMap = nullptr;
		return false;
	}

	std::list<GMap*>::iterator map_it = Mapgroup.begin();

	while (index-- > 0)
	{
		map_it++;
	}
	IMap = *map_it;

	return true;
}
