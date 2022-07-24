#pragma once
#include "Rendering/BGraphics.h"
#include "Base/ABase_Define.h"

enum ATTACK_TYPE
{
	ATTACK_TYPE_SHORT_RANGE = 0,
	ATTACK_TYPE_LONG_RANGE = 1
};


class GWeapon
{
public:
	const ASkeleton_Model* GetSkeletonmodel()const {return Skeletonmodel;}
	const void GetID(char OID[ID_LENGTH]) const {memcpy(OID, ID, ID_LENGTH * sizeof(char));}
	const void GetName(std::wstring& OName) const {OName = Name;}
	const int GetHoldingposition()const {return Holdingposition;}
	const unsigned int GetWeight()const {return Weight;}
	const ATTACK_TYPE GetAttacktype()const {return Attacktype;}
	const float GetATK()const { return ATK; };
	const int GetAttackrange()const {return Attackrange;};
	const float GetMovespeed()const { return Movespeed; };
	const float GetAttackreadytime()const {return Attackreadytime;};
	const float GetAttackcooldowntime()const { return Attackcooldowntime; };
	const int GetBoxmagazine()const { return Boxmagazine; };
	const float GetReloadtime()const { return Reloadtime; };


private:
	//动态模型指针
	ASkeleton_Model* Skeletonmodel;
	//武器ID
	char ID[ID_LENGTH];
	//武器名称
	std::wstring Name;
	//所持位置
	int Holdingposition = 0;
	//重量
	unsigned int Weight;
	//攻击类型
	ATTACK_TYPE Attacktype = ATTACK_TYPE_SHORT_RANGE;
	//伤害
	float ATK = 1.0f;
	//攻击范围
	int Attackrange = 1;
	//移动速度
	float Movespeed = 1.0f;
	//攻击冷却时间
	float Attackreadytime = 1.0f;
	//攻击启动时间
	float Attackcooldowntime = 1.0f;
	//弹夹弹药量
	int Boxmagazine = 1;
	//装填时间
	float Reloadtime = 1.0f;
};