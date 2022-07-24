#pragma once 
#include"BGraphics.h"

//提供本地加载资源方法

class RResrouce_Factory : public AResrouce_Factory
{
	virtual bool LoadData() override;
};