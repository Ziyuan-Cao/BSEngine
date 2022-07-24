#pragma once
#include "ACreature.h"
#include <map>


class GCharacter : public ACreature
{
	friend class ACharacter_Factory;
	friend class LRuning_Time_Game_Logic;

private:
	//当前级数经验值
	float Experience = 1.0f; 
	//等级
	int Level;
	//所持金钱
	int Gold;
	//负重量
	unsigned int Weight;

	//武器信息结构
	struct WeaponInformation
	{
		bool Unlock = false;
		int Blooddamaged = 1;
	};
	//武器信息表
	std::map<GWeapon*,WeaponInformation> WeaponInfgroup;


};