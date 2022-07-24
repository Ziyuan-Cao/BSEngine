#include "LRuning_Time_Game_Logic.h"
#include "Base/Tool/TMathTool.h"

void LRuning_Time_Game_Logic::CharToMonsterDamage(GCharacter* ICharacter, GMonster* IOMonster)
{
	const GWeapon* Weapon =  ICharacter->GetEquipedWeapon();
	float Damage = Weapon->GetATK();
	if (Damage > IOMonster->GetBloodvolume())
	{
		MonsterDead(IOMonster, ICharacter);
	}
	else
	{
		IOMonster->Bloodvolume -= Damage;
	}
}

void LRuning_Time_Game_Logic::MonsterToCharDamage(GCharacter* ICharacter, GMonster* IOMonster)
{
	const GWeapon* Weapon = IOMonster->GetEquipedWeapon();
	float Damage = Weapon->GetATK();
	if (Damage > ICharacter->GetBloodvolume())
	{
		CharDead(ICharacter);
	}
	else
	{
		ICharacter->Bloodvolume -= Damage;
	}
}

//??? time!
void LRuning_Time_Game_Logic::Move(
	const GMap* IMap,
	ACreature* IOCreature, 
	const GGrid* IGrid, 
	const float ITime, 
	const Position_Vector2 IDirection)
{
	Position_Vector2 Resultposition = { 0,0 };
	Position_Vector2 Destposition = { 0,0 };
	Position_Vector2 Orgposition = { 0,0 };
	float Length = 0.0f;
	float Speed = IOCreature->Getmovespeed();
	Length = ITime * Speed;
	IOCreature->GetPosition(Orgposition);
	Destposition.X = IDirection.X * Length;
	Destposition.Y = IDirection.Y * Length;
	
	float StepY = 0;
	float StepX = 0;

	int Xa = IDirection.X > 0 ? 1 : -1; //往右走还是往左走
	int Ya = IDirection.Y > 0 ? 1 : -1; //往上走还是往下走

	Grid_Vector2 Currentgrid = GMap::GetGridPosition(Orgposition);

	//检查哪边更长，以最长为单位，逐步走
	if (IDirection.Y / IDirection.X < 1)
	{
		//X偏移格数更多

		//边线
		float LY = 0;
		//向上走
		if (IDirection.X > 0)
		{
			//上边界
			LY = GMap::GetLinePosition(
				Currentgrid, GMap::DIRECTION_TYPE::DIRECTION_TYPE_UP);
		}
		else
		{
			//下边界
			LY = GMap::GetLinePosition(
				Currentgrid, GMap::DIRECTION_TYPE::DIRECTION_TYPE_DOWN);
		}

		while (StepX < abs(Destposition.X))
		{
			//求Step后Y有没有超过格子线
			Position_Vector2 BPosition = { 0,0 };
			Position_Vector2 Xpos = { StepX + Orgposition.X,0 };
			Position_Vector2 XDir = { 0,1 };
			Intersection(Orgposition, IDirection, Xpos, XDir, BPosition);
			//偏移了一格
			if (( BPosition.Y > LY && Ya > 0) || (BPosition.Y < LY && Ya < 0))
			{
				//{n+1,m+1}
				LY += GRID_SIZE * Ya;
				Currentgrid.X += Xa;
				Currentgrid.Y += Ya;
				//...
			}
			//没偏移
			else
			{
				//{n+1,m}
				Currentgrid.X += Xa;
				//...
			}
			StepY = BPosition.Y;
			StepX += GRID_SIZE * Xa;
		}

	}
	else
	{
		//Y偏移格数更多

		//边线
		float LX = 0;
		//向右走
		if (IDirection.Y > 0)
		{
			//右边界
			LX = GMap::GetLinePosition(
				Currentgrid, GMap::DIRECTION_TYPE::DIRECTION_TYPE_RIGHT);
		}
		else
		{
			//左边界
			LX = GMap::GetLinePosition(
				Currentgrid, GMap::DIRECTION_TYPE::DIRECTION_TYPE_LEFT);
		}

		while (StepY < abs(Destposition.Y))
		{
			//求Step后Y有没有超过格子线
			Position_Vector2 BPosition = { 0,0 };
			Position_Vector2 Ypos = { StepY + Orgposition.Y,0 };
			Position_Vector2 YDir = { 0,1 };
			Intersection(Orgposition, IDirection, Ypos, YDir, BPosition);
			//偏移了一格
			if ((BPosition.X > LX && Xa > 0) || (BPosition.X < LX && Xa < 0))
			{
				//{n+1,m+1}
				LX += GRID_SIZE * Xa;
				Currentgrid.X += Xa;
				Currentgrid.Y += Ya;
				//...
			}
			//没偏移
			else
			{
				//{n,m+1}
				Currentgrid.Y += Ya;
				//...
			}
			StepX = BPosition.X;
			StepY += GRID_SIZE * Ya;
		}

	}


	/*
	Position_Vector2 Resultpostion = { 0,0 };
	Position_Vector2 Destpostion = { 0,0 };
	float Length = 0.0f;
	float Speed = IOCreature->Getmovespeed();
	Length = ITime * Speed;
	
	Destpostion.X = IDirection.X * Length;
	Destpostion.Y = IDirection.Y * Length;
	IOCreature->GetPosition(Resultpostion);
	//移动的位置
	Resultpostion += Destpostion;
	//移动的位置的格子坐标
	int DestX = Resultpostion.X;
	int DestY = Resultpostion.Y;
	bool OutOfRegion = false;

	if(DestX >= IMap->GetXSize()-1)
	{
		Resultpostion.X = (IMap->GetXSize() - 1) * GRID_SIZE + GRID_SIZE/2;
		DestX = IMap->GetXSize()-1;
		OutOfRegion = true;
	}
	else if(DestX < 0)
	{
		Resultpostion.X = -GRID_SIZE / 2;
		DestX = 0;
		OutOfRegion = true;
	}
	if (DestY >= IMap->GetYSize()-1)
	{
		Resultpostion.Y = (IMap->GetYSize() - 1) * GRID_SIZE + GRID_SIZE / 2;
		DestY = IMap->GetYSize()-1;
		OutOfRegion = true;
	}
	else if (DestY < 0)
	{
		Resultpostion.Y = -GRID_SIZE / 2;
		DestY = 0;
		OutOfRegion = true;
	}

	if (!OutOfRegion)
	{
		const GGrid* DestGrid = IMap->GetGrid(DestX, DestY);
		GRID_TYPE DestGridType = DestGrid->GetGridType();
		switch (DestGridType)
		{
		case GRID_TYPE::GRID_TYPE_BORDER:
			Position_Vector2 Gridconter = IMap->GetGridCenterPosition({ DestX,DestY });
			Position_Vector2 Charpostion = {0,0};
			IOCreature->GetPosition(Charpostion);
			Position_Vector2 Gridcontertochar = Gridconter - Charpostion;
			float dir = Gridcontertochar.Y / Gridcontertochar.X;
			if (dir >= 1 || dir <= -1)
			{
				//纵坐标压边
				if (Gridcontertochar.Y > 0)
				{
					Resultpostion.Y = Gridconter.Y + GRID_SIZE / 2;
				}
				else
				{
					Resultpostion.Y = Gridconter.Y - GRID_SIZE / 2;
				}

			}
			if(dir <= 1 && dir >= -1)
			{
				//横坐标压边
				if (Gridcontertochar.X > 0)
				{
					Resultpostion.X = Gridconter.X + GRID_SIZE / 2;
				}
				else
				{
					Resultpostion.X = Gridconter.X - GRID_SIZE / 2;
				}
			}

			IOCreature->Position = Resultpostion;

			break;
		case GRID_TYPE::GRID_TYPE_NORMAL:
			IOCreature->Position = Resultpostion;
			//...
			break;
			//...
		default: //???
			IOCreature->Position = Resultpostion;
			break;

		}
	}
	*/
}

void LRuning_Time_Game_Logic::SwitchWeapon(ACreature* IOCreature, WEAPON_INDEX_TYPE ISwitchweapontype)
{
	IOCreature->Weaponindex = ISwitchweapontype;
}

//???装填时间
void LRuning_Time_Game_Logic::ReloadWeapon(ACreature* IOCreature)
{
	IOCreature->WeaponslotsInf[(int)IOCreature->Weaponindex].Maganizenumber = IOCreature->Weaponslots[(int)IOCreature->Weaponindex]->GetBoxmagazine();
}

void LRuning_Time_Game_Logic::MonsterDead(GMonster* IOMonster, GCharacter* IOCharacter)
{
	CalExpGold(IOCharacter, IOMonster->GetExperiencevalue(), IOMonster->GetGoldvalue());
	IOCharacter->WeaponInfgroup[IOCharacter->Weaponslots[IOCharacter->Weaponindex]].Blooddamaged += IOMonster->GetBlooddamagevalue();
	//...
}

void LRuning_Time_Game_Logic::CharDead(GCharacter* IOCharacter)
{
	//...
}

void LRuning_Time_Game_Logic::CalExpGold(GCharacter* IOCharacter,float IExp, int IGold)
{
	IOCharacter->Experience += IExp;
	IOCharacter->Gold += IGold;
	//...
}