#pragma once 
#include"Pre_Define.h"
#include "TMathTool.h"

class ENGINEDLL_API AResource
{
public:
	//��������
	virtual bool ReadData() = 0;
private:
	void* Data=nullptr;
	std::wstring Filepath();
};