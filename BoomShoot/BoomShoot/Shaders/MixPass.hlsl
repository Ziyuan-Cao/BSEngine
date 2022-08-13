#include "Common.hlsl"

Texture2D LightMap : register(t0);
Texture2D SceneDepthMap: register(t1);
Texture2D SSAOMap: register(t2);
Texture2D GBuffer[5] : register(t3);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

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
    float4 Lightcolor = LightMap.Sample(gsamPointClamp, pIn.texcoord);

    uint width, height, numMips;
    SSAOMap.GetDimensions(0, width, height, numMips);


    float percentLit = 0.0f;
    float dx = 1.0f / (float)width;
    float dy = 1.0f / (float)height;
    const float2 offsets[9] =
    {
        float2(-dx,  -dy), float2(0.0f,  -dy), float2(dx,  -dy),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dy), float2(0.0f,  +dy), float2(dx,  +dy)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
	    //变黑0.005使得顶点不于阴影图重叠，导致的交错
	    percentLit += SSAOMap.Sample(gsamPointClamp,
            pIn.texcoord + offsets[i]).x;
    }

    percentLit /= 9.0;
    //Debug
    float4 rescolor = Lightcolor * percentLit;


    return rescolor;
}