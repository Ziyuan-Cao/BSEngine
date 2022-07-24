#pragma once 
#include <string>
#include <vector>

//格子大小
#define GRID_SIZE 2.0f

//ID位数
#define ID_LENGTH 24

//武器栏长度及索引
enum WEAPON_INDEX_TYPE
{
	WEAPON_INDEX_TYPE_MAIN = 0,
	WEAPON_INDEX_TYPE_DEPUTY = 1,
	WEAPON_INDEX_TYPE_CLOSE = 2,
	WEAPON_INDEX_TYPE_NUMBER = 3
};
#define WEAPON_NUMBER WEAPON_INDEX_TYPE_NUMBER

class GTime
{
	float time = 0.0f;

};
