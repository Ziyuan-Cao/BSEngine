#include "Common.hlsl"
 
StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

Texture2D gShadowMap: register(t1, space1);

Texture2D gTextureMaps[20] : register(t2);


SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gTexTransform;
	uint gMaterialIndex;
	uint gObjPad0;
	uint gObjPad1;
	uint gObjPad2;
};

cbuffer cbPass : register(b1)
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

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentU : TANGENT;
	uint2 MaterialId : MATERIALID;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float4 ShadowPosH : POSITION0;
	float3 PosW    : POSITION1;
	float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexC    : TEXCOORD;
	uint2 MaterialId : MATERIALID;
};

struct ps_output
{
	float4 GBufferA : SV_TARGET0;
	float4 GBufferB : SV_TARGET1;
	float4 GBufferC : SV_TARGET2;
	float4 GBufferD : SV_TARGET3;
	float4 GBufferE : SV_TARGET4;
};

float CalcShadowFactor(float4 shadowPosH)
{
	// Complete projection by doing division by w.
	shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC space.
	//点在阴影图中的深度 比图黑则照得到 ，比图白则被遮住
	float depth = shadowPosH.z;

	uint width, height, numMips;
	gShadowMap.GetDimensions(0, width, height, numMips);

	// Texel size.
	float dx = 1.0f / (float)width;
	float dy = 1.0f / (float)height;

	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dy), float2(0.0f,  -dy), float2(dx,  -dy),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dy), float2(0.0f,  +dy), float2(dx,  +dy)
	};

	//变黑0.005使得顶点不于阴影图重叠，导致的交错
	percentLit = gShadowMap.SampleCmpLevelZero(gsamShadow,
		shadowPosH.xy, depth - 0.005).r;

	//???
	if (shadowPosH.x >= 0.80 || shadowPosH.x < 0.15)
	{
		percentLit = 1;
	}

	if (shadowPosH.y >= 0.80 || shadowPosH.y < 0.15)
	{
		percentLit = 1;
	}

	//percentLit = percentLit * step(0, 1.0 - shadowPosH.x) * step(0, 1.0 - shadowPosH.y);


	return percentLit;
	//[unroll]
	//for (int i = 0; i < 9; ++i)
	//{
	//	//变黑0.005使得顶点不于阴影图重叠，导致的交错
	//	percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
	//		shadowPosH.xy + offsets[i], depth - 0.005).r;
	//}
	//return percentLit / 9.0f;
}


VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	// Fetch the material data.
	MaterialData matData = gMaterialData[gMaterialIndex];

	// Transform to world space.
	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosW = posW.xyz;

	// Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

	vout.TangentW = mul(vin.TangentU, (float3x3)gWorld);

	// Transform to homogeneous clip space.
	vout.PosH = mul(posW, gViewProj);

	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
	vout.TexC = mul(texC, matData.MatTransform).xy;

	// Generate projective tex-coords to project shadow map onto scene.
	//将点投影到阴影图中
	vout.ShadowPosH = mul(posW, gShadowTransform);
	vout.MaterialId = vin.MaterialId;
	return vout;
}

ps_output PS(VertexOut pin)
{
	ps_output OUT;
	MaterialData matData = gMaterialData[gMaterialIndex];
	uint textureMapIndex = matData.DiffuseMapIndex;
	uint normalMapIndex = matData.NormalMapIndex;

	//Basecolor
	float4 BaseColor = gTextureMaps[textureMapIndex].Sample(gsamPointWrap, pin.TexC);//baseColor
	//float4 BaseColor = float4(matData.LightColor.r, matData.LightColor.g, matData.LightColor.b, 1);

	//Normal
	pin.NormalW = normalize(pin.NormalW);
	//float4 normalMapSample = gTextureMaps[normalMapIndex].Sample(gsamPointWrap, pin.TexC);//normal
	float3 WorldNormal = pin.NormalW;// NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);

	//Shadowfactors
	float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
	shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH);//shadow

	//WorldPosition
	float3 WorldPosition = pin.PosW;

	//float Mat_id_store = gMaterialIndex & 0xff;
	OUT.GBufferA = BaseColor;
	OUT.GBufferB = float4(WorldNormal, gMaterialIndex);
	OUT.GBufferC = float4(shadowFactor, 1.0f);//profilefactor
	OUT.GBufferD = float4(WorldPosition, 1.0f);
	OUT.GBufferE = float4(1.0f, 1.0f, 1.0f, 1.0f);

	return OUT;
}


