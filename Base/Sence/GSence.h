#pragma once 
#include "GGrid.h"
#include "Base/Object/GMonster.h"

#include <vector>
#include <list>

// 中心点为{2n,2m}
// 上线为{x,2m+1}
// 下线为{x,2m-1}
// 左线为{2n-1,y}
// 右线为{2n+1,y}
//地图左下角中心点为(0,0),最边角为(-1,-1)
//右上角中心点为(2n,2m),
//最边角为(2n+1,2m+1)

class GMap 
{
public:
	GMap(int IXSize = 10,int IYSize = 10);

	bool SetGrids(const Grid_Vector2 IStartposition,
		const Grid_Vector2 IEndPosition,
		const GGrid* IGrid);

	const GGrid* GetGrid(const int IX, const int IY) const { return Gridgroup[IX][IY]; };

	const int GetXSize()const { return XSize; };
	const int GetYSize()const { return YSize; };

	const static Position_Vector2 GetGridCenterPosition(const Grid_Vector2& IGridposition)
	{
		return { IGridposition.X * GRID_SIZE, IGridposition.Y * GRID_SIZE};
	};

	const static Grid_Vector2 GetGridPosition(const Position_Vector2 IPosition)
	{
		//区分正负
		int symX = IPosition.X / IPosition.X;
		int symY = IPosition.Y / IPosition.Y;
		int X = (IPosition.X + symX) / GRID_SIZE;
		int Y = (IPosition.Y + symY) / GRID_SIZE;
		return { X,Y };
	}

	enum DIRECTION_TYPE
	{
		DIRECTION_TYPE_UP,
		DIRECTION_TYPE_DOWN,
		DIRECTION_TYPE_LEFT,
		DIRECTION_TYPE_RIGHT
	};

	const static float GetLinePosition(const Grid_Vector2& IGridposition, const DIRECTION_TYPE IDirection)
	{
		switch(IDirection)
		{
		case DIRECTION_TYPE::DIRECTION_TYPE_UP:
			return IGridposition.Y * GRID_SIZE + 1;
			break;
		case DIRECTION_TYPE::DIRECTION_TYPE_DOWN:
			return IGridposition.Y * GRID_SIZE - 1;
			break;
		case DIRECTION_TYPE::DIRECTION_TYPE_LEFT:
			return IGridposition.X * GRID_SIZE + 1;
			break;
		case DIRECTION_TYPE::DIRECTION_TYPE_RIGHT:
			return IGridposition.X * GRID_SIZE - 1;
			break;
		}
		return 0;
	}




private:
	std::vector<std::vector<const GGrid*>> Gridgroup;
	//n
	int XSize = 10;
	//m
	int YSize = 10;

	Grid_Vector2 Entryposition = {0,0};

	Grid_Vector2 Exitposition = {9,9};

	struct Monster_Gird
	{
		Grid_Vector2 position = {0,0};
		GMonster* GMonster;
	};

	std::vector<Monster_Gird*> Monstergridgroup;

};



class GSence
{
friend class ASence_Factory;

private:
	void InsertMap(GMap *IMap);
	
	void DeleteMap(GMap* IMap);
	
	bool GetMap(GMap* IMap,int index);

	bool GetMapList(std::list<GMap*>* OMapgroup);

	void ChangeMapIndex(GMap* IMap);


	std::list<GMap*> Mapgroup;


	
};

