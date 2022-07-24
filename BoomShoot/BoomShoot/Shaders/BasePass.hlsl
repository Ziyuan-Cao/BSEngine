#include "Common.hlsl"
// Put in space1, so the texture array does not overlap with these resources.  
// The texture array will occupy registers t0, t1, ..., t3 in space0. 
StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

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

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light gLights[MaxLights];
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
	//float4 BaseColor = gTextureMaps[textureMapIndex].Sample(gsamPointWrap, pin.TexC);//baseColor
	float4 BaseColor = float4(0.9, gMaterialIndex * 0.12, 0.9, 1);

	//Normal
	pin.NormalW = normalize(pin.NormalW);
	//float4 normalMapSample = gTextureMaps[normalMapIndex].Sample(gsamPointWrap, pin.TexC);//normal
	float3 WorldNormal = pin.NormalW;// NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);

	//Shadowfactors
	float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
	//shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH);//shadow

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

