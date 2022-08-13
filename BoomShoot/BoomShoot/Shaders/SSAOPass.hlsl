#include "Common.hlsl"

Texture2D SceneDepthMap: register(t0);
Texture2D NoiseMap: register(t1);
Texture2D GBuffer[5] : register(t2);

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
};

cbuffer SSAOBuffer : register(b1)
{
    float2 screenSize;
    float2 noiseScale; // tiling
    float radius;
    float power;
    float kernelSize;
    float3 kernelOffsets[16];
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
    float2 screenPixelOffset = float2(2.0f, -2.0f) / float2(screenSize.x, screenSize.y);
    float2 positionScreen = (float2(coords.xy) + 0.5f) * screenPixelOffset.xy + float2(-1.0f, 1.0f);
    float viewSpaceZ = gProj._43 / (zbuffer - gProj._33);


    float2 screenSpaceRay = float2(positionScreen.x / gProj._11, positionScreen.y / gProj._22);
    float3 positionView;
    positionView.z = viewSpaceZ;
    positionView.xy = screenSpaceRay.xy * positionView.z;

    return positionView;
}

vs_out  VS(vs_in vIn)
{
    vs_out vOut;
    vOut.texcoord = vIn.texcoord;
    vOut.position = float4(vOut.texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);


    return vIn;
}


float4 PS(vs_out pIn) : SV_TARGET
{ 

    float4 gbuffera = GBuffer[0].Sample(gsamPointClamp, pIn.texcoord);
    float4 gbufferb = GBuffer[1].Sample(gsamPointClamp, pIn.texcoord);
    float4 gbufferd = GBuffer[3].Sample(gsamPointClamp, pIn.texcoord);

    //摄像机局部坐标系法线
    float3 normal = normalize(mul(gbufferb.xyz, gView));

    //摄像机局部坐标系的位置和法线
    float4 viewpostion = mul(float4(gbufferd.xyz, 1.0), gView);
    viewpostion.xyz /= viewpostion.w;
    float3 centerDepthPos = viewpostion.xyz;
    //float centerZBuffer = SceneDepthMap.Sample(gsamPointClamp, pIn.texcoord).r;
    //float3 centerDepthPos = ComputePositionViewFromZ(uint2(pIn.position.xy), centerZBuffer);

    //随机点
    float3 randomVector = NoiseMap.Sample(gsamLinearWrap, pIn.texcoord * noiseScale).xyz;
    //随机切线
    float3 tangent = normalize(randomVector - normal * dot(randomVector, normal));
    float3 bitangent = cross(normal, tangent);
    //切线空间
    float3x3 transformMat = float3x3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < (int)kernelSize; ++i)
    {
        //在半球内找随机点
        float3 samplePos = mul(kernelOffsets[i], transformMat);
        samplePos = samplePos * radius + centerDepthPos;

        //方向
        float3 sampleDir = normalize(samplePos - centerDepthPos);
        //方向
        float nDotS = max(dot(normal, sampleDir), 0);

        //转换到屏幕空间 找对应深度
        float4 offset = mul(float4(samplePos, 1.0), gProj);
        //缩放归一
        offset.xy /= offset.w;
        float4 offsetposition = GBuffer[3].Sample(gsamPointClamp, float2(offset.x * 0.5 + 0.5, -offset.y * 0.5 + 0.5));
        offsetposition = mul(float4(offsetposition.xyz, 1.0), gView);
        offsetposition.xyz /= offsetposition.w;
        float sampleDepth = offsetposition.z;
        //使用深度重新计算摄像机局部坐标系内的为三维坐标
        //float sampleDepth = SceneDepthMap.Sample(gsamPointClamp, float2(offset.x * 0.5 + 0.5, -offset.y * 0.5 + 0.5)).r;
        //sampleDepth = ComputePositionViewFromZ(offset.xy, sampleDepth).z;

        //距离 大于 半径 则趋近于0 表示未受遮挡
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(centerDepthPos.z - sampleDepth));
        //如果采样点在自己前面则不加，否则按照角度叠加遮挡
        occlusion += rangeCheck * step(sampleDepth, samplePos.z) * nDotS;
    }
    //反向遮挡值
    occlusion = 1.0 - (occlusion / kernelSize);
    float finalOcclusion = pow(occlusion, power);

    return float4(finalOcclusion, finalOcclusion, finalOcclusion,1.0);
}
