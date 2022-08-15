#pragma once 
#include "BGraphics.h"
#include"Pre_Define.h"
#include "TMathTool.h"


//∑¥…‰

class RTexture : public ATexture
{

public:
	bool GPUInit = false;

	ID3D12Resource* TextureGPU = nullptr;
	ID3D12Resource* Textureuploader = nullptr;
	D3D12_SHADER_RESOURCE_VIEW_DESC TextureSRV;
public:
	void SetTextureType(ATexture::TEXTURE_TYPE ITextureType) { TextureType = ITextureType; };

	ATexture::TEXTURE_TYPE GetTextureType() { return TextureType; };

	RTexture() {};

	RTexture(ATexture::TEXTURE_TYPE ITextureType) : TextureType(ITextureType) {};

private:
	

	ATexture::TEXTURE_TYPE TextureType = ATexture::TEXTURE_TYPE::COLOR_TEXTURE;
		ID3D12Resource* Resource = nullptr;
		ID3D12Resource* UploadHeap = nullptr;
};