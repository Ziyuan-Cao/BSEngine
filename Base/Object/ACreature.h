#pragma once
#include "Rendering/BGraphics.h"
#include "Base/ABase_Define.h"
#include "Base/Tool/TMathTool.h"
#include "GWeapon.h"

class ACreature
{
	friend class LRuning_Time_Game_Logic;
public:
	const float GetBloodvolume() const {return Bloodvolume;};
	void GetPosition(Position_Vector2& OPosition) const {OPosition = Position;};
	void GetName(std::wstring& OName) const {OName = Name;};
	void GetID(char OID[ID_LENGTH]) const {memcpy(OID, ID, ID_LENGTH * sizeof(char));};
	const float Getmovespeed()const { return Defaultmovespeed * GetEquipedWeapon()->GetMovespeed(); };
	const ASkeleton_Model* GetSkeletonmodel() const {return Skeletonmodel;};

	const GWeapon* GetEquipedWeapon() const { return Weaponslots[Weaponindex]; };


protected:
	ACreature();

protected:
	//动态模型指针
	ASkeleton_Model* Skeletonmodel;
	//生物ID
	char ID[ID_LENGTH];
	//生物名称
	std::wstring Name;
	//血量
	float Bloodvolume = 10.0f;
	//体积
	unsigned int Volume = 1;
	//位置
	Position_Vector2 Position = {0,0};
	//移动速度
	float Defaultmovespeed = 1.0f;

	//武器栏
	GWeapon* Weaponslots[WEAPON_NUMBER];
	//弹药记录
	struct  WeaponslotInf
	{
		unsigned int Maganizenumber = 0;
	};
	WeaponslotInf WeaponslotsInf[WEAPON_NUMBER];

	//当前武器
	WEAPON_INDEX_TYPE Weaponindex = WEAPON_INDEX_TYPE_MAIN;

};