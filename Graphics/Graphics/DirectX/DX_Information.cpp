#include "DirectX/DX_Information.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

DX_Information* DX_Information::DXInfinstance = nullptr;

DX_Information::DX_Information(ID3D12Device* IDevice, ID3D12GraphicsCommandList* ICommandList)
{

    //mCamera.SetLens(0.25f * MathHelper::Pi, 16.0f / 9.0f, 0.5f, 1000.0f);

    Resourceheap = new BResource_Heap(IDevice);

    RtvDescriptorsize = IDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    DsvDescriptorsize = IDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    CbvSrvUavDescriptorsize = IDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    OnResize();

    DXInfinstance = this;
}

void DX_Information::OnResize()
{

    Resourceheap->Resize();

    // Update the viewport transform to cover the client area.
    Screenviewport.TopLeftX = 0;
    Screenviewport.TopLeftY = 0;
    Screenviewport.Width = static_cast<float>(Clientwidth);
    Screenviewport.Height = static_cast<float>(Clientheight);
    Screenviewport.MinDepth = 0.0f;
    Screenviewport.MaxDepth = 1.0f;

    Scissorrect = { 0, 0, Clientwidth, Clientheight };


}


const D3D12_VIEWPORT* const DX_Information::GetScreenViewport()const
{
    return &Screenviewport;
}

const D3D12_RECT* const DX_Information::GetScissorRect()const
{
    return &Scissorrect;
}

void DX_Information::SetPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc)
{
    psoDesc.RTVFormats[0] = Backbufferformat;
    psoDesc.SampleDesc.Count = Msaa4xstate ? 4 : 1;
    psoDesc.SampleDesc.Quality = Msaa4xstate ? (Msaa4xquality - 1) : 0;
    psoDesc.DSVFormat = Depthstencilformat;
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7Ui64> DX_Information::GetStaticSamplers()
{
    // Applications usually only need a handful of samplers.  So just define them all up front
    // and keep them available as part of the root signature.  

    const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
        0, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW*/

    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
        1, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
        2, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
        3, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
        4, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
        0.0f,                             // mipLODBias
        8);                               // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
        5, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
        0.0f,                              // mipLODBias
        8);                                // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC shadow(
        6, // shaderRegister
        D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
        0.0f,                               // mipLODBias
        16,                                 // maxAnisotropy
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

    return {
        pointWrap, pointClamp,
        linearWrap, linearClamp,
        anisotropicWrap, anisotropicClamp,
        shadow
    };
}
