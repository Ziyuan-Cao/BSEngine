#include "Base/ABase_Define.h"
#include "Base/Tool/TMathTool.h"

//输入处理
// 1.方向
// 2.点击
// 3.躲避
// 4.装填
// 5.切换武器
// 6.暂停
// 

//
class LInput_Process
{
public:
	LInput_Process();

	void FrameRefresh();

	bool MoveDirectionInput(Position_Vector2 & ODirection);

	bool ClickInput(Position_Vector2& ODirection);

	bool AvoidInput();

	bool ReloadInput();

	bool SwitchInput(WEAPON_INDEX_TYPE & Weaponindex);

	bool StopInput();

	void SetDebug(bool IsDebug = true) { Debugviewcontrol = IsDebug; };
	bool GetDebugFlag() { return Debugviewcontrol; };

private:
	struct GameKey
	{
		char VKvalue;
		unsigned int num = 0;
	};

	GameKey Key_MoveUp;
	GameKey Key_MoveDown;
	GameKey Key_MoveLeft;
	GameKey Key_MoveRight;
	GameKey Key_MLAttack;
	GameKey Key_MR;
	GameKey Key_Esc;
	GameKey Key_Space;
	GameKey Key_Reload;

	const char MOVEUPDIR = 0X01;
	const char MOVEDOWNDIR = 0X02;
	const char MOVELEFTDIR = 0X04;
	const char MOVERIGHTDIR = 0X08;

	char Movedirection = 0;
	
	bool Debugviewcontrol = false;
};