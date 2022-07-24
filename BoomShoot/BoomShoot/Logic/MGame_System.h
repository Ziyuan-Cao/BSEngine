#pragma once



//游戏逻辑主类
//复杂的，循环的处理游戏进行
//结合输入输出，处理游戏逻辑
// 1.游戏流程
// 2.入口流程
// 3.武器商店流程
// 4.界面流程

class MGame_System
{
public:
	//Enterance process
	void EnteranceSenceProcess();

	//Gun store process
	void StoreProcess();

	void BuyGun();

	//Sence process
	void GameSenceProcess();

	void Damagecalculation();
	void CreatureMovement();

	//Exit process
	void ExitGame();

	//UI process ???
	void LoadMenu();
	void LoadSetting();
	void LoadOverMenu();
	void LoadStartTitle();

};