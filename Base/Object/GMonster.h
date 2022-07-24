#pragma once
#include "ACreature.h"

class GMonster : public ACreature
{
public:
	friend class AMonster_Factory;
	friend class LRuning_Time_Game_Logic;

	const float GetExperiencevalue() const { return Experiencevalue;};
	const int GetGoldvalue() const { return Goldvalue; };
	const int GetBlooddamagevalue() const { return Blooddamagevalue; };
private:
	//掉落经验值
	float Experiencevalue = 1.0f;
	//掉落金币数
	int Goldvalue = 1.0f;
	int Blooddamagevalue = 1;
};