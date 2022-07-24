#include "LInput_Process.h"
#include<Windows.h>

LInput_Process::LInput_Process()
{
	//左键
	Key_MLAttack = { VK_LBUTTON ,0 };
	//右键
	Key_MR = { VK_RBUTTON ,0 };
	//上移
	Key_MoveUp = { 'W' ,0};
	//下移
	Key_MoveDown = { 'S' ,0 };
	//左移
	Key_MoveLeft = { 'A' ,0 };
	//右移
	Key_MoveRight = { 'D' ,0 };
	//回退
	Key_Esc = { VK_ESCAPE , 0 };
	//回避
	Key_Space = { VK_SPACE , 0 };
	//装填
	Key_Reload = {'R', 0};
}

void LInput_Process::FrameRefresh()
{
	Key_MLAttack.num = 0;
	Key_MR.num = 0;
	Key_MoveUp.num = 0;
	Key_MoveDown.num = 0;
	Key_MoveLeft.num = 0;
	Key_MoveRight.num = 0;
	Key_Esc.num = 0;
	Key_Space.num = 0;
	Key_Reload.num = 0;
	Movedirection = 0;

	if (GetAsyncKeyState(Key_MLAttack.VKvalue) & 0x8000)
	{
		Key_MLAttack.num++;
	}
	if (GetAsyncKeyState(Key_MR.VKvalue) & 0x8000)
	{
		Key_MR.num++;
	}

	if (GetAsyncKeyState(Key_MoveUp.VKvalue) & 0x8000)
	{
		Key_MoveUp.num++;
		Movedirection |= MOVEUPDIR;
	}
	if (GetAsyncKeyState(Key_MoveDown.VKvalue) & 0x8000)
	{
		Key_MoveDown.num++;
		Movedirection |= MOVEDOWNDIR;
	}
	if (GetAsyncKeyState(Key_MoveLeft.VKvalue) & 0x8000)
	{
		Key_MoveLeft.num++;
		Movedirection |= MOVELEFTDIR;
	}
	if (GetAsyncKeyState(Key_MoveRight.VKvalue) & 0x8000)
	{
		Key_MoveRight.num++;
		Movedirection |= MOVERIGHTDIR;
	}

	if (GetAsyncKeyState(Key_Esc.VKvalue) & 0x8000)
	{
		Key_Esc.num++;
	}
	if (GetAsyncKeyState(Key_Space.VKvalue) & 0x8000)
	{
		Key_Space.num++;
	}
	if (GetAsyncKeyState(Key_Reload.VKvalue) & 0x8000)
	{
		Key_Reload.num++;
	}
}


bool LInput_Process::MoveDirectionInput(Position_Vector2& ODirection)
{
	if (Movedirection == MOVEUPDIR & MOVELEFTDIR)
	{
		ODirection = { 1,1 };
		return true;
	}
	if (Movedirection == MOVEUPDIR & MOVERIGHTDIR)
	{
		ODirection = { -1,1 };
		return true;
	}
	if(Movedirection == MOVEUPDIR)
	{
		ODirection = {0,1};
		return true;
	}
	if (Movedirection == MOVEDOWNDIR & MOVELEFTDIR)
	{
		ODirection = { 1,-1 };
		return true;
	}
	if (Movedirection == MOVEDOWNDIR & MOVERIGHTDIR)
	{
		ODirection = { -1,-1 };
		return true;
	}
	if (Movedirection == MOVEDOWNDIR)
	{
		ODirection = { 0,-1 };
		return true;
	}
	if (Movedirection == MOVERIGHTDIR)
	{
		ODirection = { 1,0 };
		return true;
	}
	if (Movedirection == MOVELEFTDIR)
	{
		ODirection = { -1,0};
		return true;
	}

	return false;
}

bool LInput_Process::ClickInput(Position_Vector2& ODirection)
{
	//获得鼠标位置
	if (Key_MoveLeft.num > 0)
	{
		//...
		return true;
	}
}

bool LInput_Process::AvoidInput()
{
	if (Key_Space.num > 0)
	{
		return true;
	}
}

bool LInput_Process::ReloadInput()
{
	if (Key_Reload.num > 0)
	{
		return true;
	}
}

bool LInput_Process::SwitchInput(WEAPON_INDEX_TYPE& Weaponindex)
{
	//...
	return true;
}

bool LInput_Process::StopInput()
{
	if (Key_Esc.num > 0)
	{
		return true;
	}
	return false;
}