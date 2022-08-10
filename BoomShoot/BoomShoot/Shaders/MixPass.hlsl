#include "Common.hlsl"

Texture2D LightMap : register(t0);
Texture2D SceneDepthMap: register(t1);
Texture2D SSAOMap: register(t2);
Texture2D GBuffer[5] : register(t3);


cbuffer cbPass : register(b0)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float4x4 gShadowTransform;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;

    Light gLights[MaxLights];
};

struct vs_in {
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct vs_out {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

vs_out  VS(vs_in vIn)
{
    vs_out vOut;
    vOut.position = vIn.position;
    vOut.texcoord = vIn.texcoord;
    return vIn;
}


float4 PS(vs_out pIn) : SV_TARGET
{
    //Debug
    float4 rescolor = LightMap[pIn.position.xy] * SSAOMap[pIn.position.xy];


    return rescolor;
}