#pragma once 
#include "BGraphics.h"
#include"Pre_Define.h"
#include "TMathTool.h"

class RTexture : public ATexture
{

private:
		ID3D12Resource* Resource = nullptr;
		ID3D12Resource* UploadHeap = nullptr;
};