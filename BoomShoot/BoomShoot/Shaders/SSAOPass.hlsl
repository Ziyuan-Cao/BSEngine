#include "Common.hlsl"

Texture2D SceneDepthMap: register(t0);
Texture2D NoiseMap: register(t1);
Texture2D GBuffer[5] : register(t2);


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
};


struct vs_in {
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct vs_out {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float3 ComputePositionViewFromZ(uint2 coords, float zbuffer)
{
    float2 screenPixelOffset = float2(2.0f, -2.0f) / float2(800, 600);
    float2 positionScreen = (float2(coords.xy) + 0.5f) * screenPixelOffset.xy + float2(-1.0f, 1.0f);
    float viewSpaceZ = gViewProj._43 / (zbuffer - gViewProj._33);


    float2 screenSpaceRay = float2(positionScreen.x / gViewProj._11, positionScreen.y / gViewProj._22);
    float3 positionView;
    positionView.z = viewSpaceZ;
    positionView.xy = screenSpaceRay.xy * positionView.z;

    return positionView;
}


vs_out  VS(vs_in vIn)
{
    vs_out vOut;
    vOut.position = vIn.position;
    vOut.texcoord = vIn.texcoord;

    return vIn;
}

float4 PS(vs_out pIn) : SV_TARGET
{ 
    float4 gbuffera = GBuffer[0][pIn.position.xy];
    float4 gbufferb = GBuffer[1][pIn.position.xy];
    float4 gbufferd = GBuffer[3][pIn.position.xy];

    float centerZBuffer = SceneDepthMap[pIn.texcoord].r;
    float3 centerDepthPos = ComputePositionViewFromZ(uint2(pIn.position.xy), centerZBuffer);
    float3 normal = gbufferb.rgb;

    float3 randomVector = NoiseMap[pIn.texcoord].xyz;
    float3 tangent = normalize(randomVector - normal * dot(randomVector, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 transformMat = float3x3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < (int)16; ++i)
    {
        float3 samplePos = mul(float3(-0.5,0,0.5), transformMat);
        samplePos = samplePos * 3.5 + centerDepthPos;

        float3 sampleDir = normalize(samplePos - centerDepthPos);

        float nDotS = max(dot(normal, sampleDir), 0);

        float4 offset = mul(float4(samplePos, 1.0), gViewProj);
        offset.xy /= offset.w;
        float sampleDepth = SceneDepthMap[float2(offset.x * 0.5 + 0.5, -offset.y * 0.5 + 0.5)].r;

        sampleDepth = ComputePositionViewFromZ(offset.xy, sampleDepth).z;

        float rangeCheck = smoothstep(0.0, 1.0, 3.5 / abs(centerDepthPos.z - sampleDepth));
        occlusion += rangeCheck * step(sampleDepth, samplePos.z) * nDotS;
    }

    occlusion = 1.0 - (occlusion / 16);
    float finalOcclusion = pow(occlusion, 2.0);

    return float4(finalOcclusion, finalOcclusion, finalOcclusion,finalOcclusion);
}