#pragma once 
#include "Base/Sence/GSence.h"
#include "Base/Sence/GGrid.h"
#include "Base/Object/GWeapon.h"
#include "Base/Object/ACreature.h"
#include "Base/Object/GCharacter.h"
#include "Base/Object/GMonster.h"

//简单的，对基类数据直接操作
//1.伤害判定
//2.移动判定
//3.切换武器
//4.武器装填
//5.死亡
//6.经验金币计算


class LRuning_Time_Game_Logic
{
	void CharToMonsterDamage(GCharacter * ICharacter,GMonster* IOMonster);

	void MonsterToCharDamage(GCharacter* ICharacter, GMonster* IOMonster);

	void Move(const GMap* IMap,ACreature * IOCreature,const GGrid* IGrid, const float ITime, const Position_Vector2 IDirection);

	void SwitchWeapon(ACreature* IOCreature, WEAPON_INDEX_TYPE ISwitchweapontype);

	void ReloadWeapon(ACreature* IOCreature);

	void MonsterDead(GMonster* IOMonster, GCharacter* IOCharacter);

	void CharDead(GCharacter* IOCharacter);

	void CalExpGold(GCharacter* IOCharacter, float IExp, int IGold);
};