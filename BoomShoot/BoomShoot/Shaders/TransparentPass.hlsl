#include "Common.hlsl"
#include "MathFunction.hlsl"



StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

Texture2D LightMap: register(t1);

Texture2D WaterFlowMap1 : register(t2);

Texture2D WaterFlowMap2 : register(t3);

Texture2D GBuffer[5] : register(t4);



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

//struct VertexOut
//{
//	float4 PosH    : SV_POSITION;
//	float3 PosW    : POSITION1;
//	float3 NormalW : NORMAL;
//	float3 TangentW : TANGENT;
//	float2 TexC    : TEXCOORD;
//	uint2 MaterialId : MATERIALID;
//};
struct VertexOutput {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : TEXCOORD1;  // world normal
	float3 tangent : TEXCOORD2;
	float3 bitangent : TEXCOORD3;
	float3 worldPos : TEXCOORD4;
	float4 projPos : TEXCOORD5;
	float timer : TEXCOORD6;
	float4 wind : TEXCOORD7; // xy = normalized wind, zw = wind multiplied with timer
};


VertexOutput VS(VertexIn vin)
{
	VertexOutput o = (VertexOutput)0;

	float2 windDir = float2(-0.5,-0.6);
	float windSpeed = 5.0;
	float waveSteepness = 0.72;
	float waveTiling = 1.0;
	float4 waveAmplitude = float4(0.13,0.3,0.1,0.05);
	float waveAmplitudeFactor = 0.25;
	float4 wavesIntensity = float4(5,3,2,1.3);
	float4 wavesNoise = float4(0.15,0.32,0.15,0.15);
	float _heightIntensity = 0.3;
	float _normalIntensity = 0.52;
	float textureTiling = 3.5;
	float timer = gTotalTime;


	float4 Pos = float4(vin.PosL, 1.0);

	float3 worldPos = mul(Pos, gWorld);

	half3 normal = half3(0, 1, 0);

	float cameraDistance = length(gEyePosW.xyz - worldPos);

	float2 noise = GetNoise(worldPos.xz, timer * windDir * 0.5);

	half3 tangent;

	float4 waveSettings = float4(windDir, waveSteepness, waveTiling);
	float4 waveAmplitudes = waveAmplitude * waveAmplitudeFactor;

	worldPos = ComputeDisplacement(worldPos, cameraDistance, noise, timer,
		waveSettings, waveAmplitudes, wavesIntensity, wavesNoise,
		normal, tangent);

	float heightIntensity = _heightIntensity * (1.0 - cameraDistance / 100.0) * waveAmplitude;

	float2 texCoord = worldPos.xz * 0.05 * textureTiling;

	if (heightIntensity > 0.02)
	{
		float height = ComputeNoiseHeight(WaterFlowMap1, wavesIntensity, wavesNoise,
			texCoord, noise, timer);
		worldPos.y += height * heightIntensity;
	}

	o.tangent = tangent;
	o.bitangent = cross(normal, tangent);
	o.timer = timer;
	o.wind.xy = windDir;
	o.wind.zw = windDir * timer;
	float2 uv = worldPos.xz;
	o.uv = uv * 0.05 * textureTiling;
	o.pos = mul(float4(worldPos, 1.0f), gViewProj);
	o.worldPos = worldPos;
	o.projPos = ClipToScreenPos(o.pos);

	o.normal = normal;

	return o;
}

float4 PS(VertexOutput pin) : SV_TARGET
{
	MaterialData mat = gMaterialData[gMaterialIndex];

	float windSpeed = 1.0;
	float waveSteepness = 0.72;
	float waveTiling = 1.0;
	float4 waveAmplitude = float4(0.13, 0.3, 0.1, 0.05);
	float waveAmplitudeFactor = 0.25;
	float4 wavesIntensity = float4(5, 3, 2, 1.3);
	float4 wavesNoise = float4(0.15, 0.32, 0.15, 0.15);
	float _heightIntensity = 0.3;
	float _normalIntensity = 0.52;
	float textureTiling = 3.5;

	float timer = pin.timer;
	float2 windDir = pin.wind.xy;
	float2 timedWindDir = pin.wind.zw;
	float2 ndcPos = float2(pin.projPos.xy / pin.projPos.w);
	float3 eyeDir = normalize(gEyePosW.xyz - pin.worldPos);
	float3 surfacePosition = pin.worldPos;
	half3 lightColor = mat.LightColor.rgb;

	float3 normal = ComputeNormal(WaterFlowMap2, surfacePosition.xz, pin.uv,
		pin.normal, 0, 0, wavesNoise, wavesIntensity, timedWindDir);

	normal = normalize(lerp(pin.normal, normalize(normal), _normalIntensity));



	return float4(normal.x, normal.y, normal.z,1.0);
}



//
//struct VS_CONTROL_POINT_OUTPUT
//{
//	float3 pos : MODELPOS;
//	float2 texCoord : TEXCOORD;
//};
//
////useless
//VertexIn Water(VertexIn vin)
//{
//	VertexIn watervout = vin;
//
//	float waveParticleSpeedScale = 0.5;
//	float timeScale = 1.0;
//	float time = 1.0;
//
//	float2 pos = float2(vin.PosL.xy);
//	float2 direction = normalize(vin.TexC);
//	float height = vin.PosL.z;
//	float speed = waveParticleSpeedScale * vin.NormalL.z;
//
//	pos = pos + speed * timeScale * float(time) * direction;
//
//	float2 posTemp = abs(pos);
//
//	if (posTemp.x > 1.0 || posTemp.y > 1.0)
//	{
//		float2 offset = float2(0, 0);
//		int2 posI = posTemp;
//		float2 posF = posTemp - posI;
//
//		if (posTemp.x > 1.0)
//		{
//			offset.x = (posI.x - 1) % 2 + posF.x;
//			pos.x = sign(pos.x) * offset.x + sign(pos.x) * -1;
//		}
//
//		if (posTemp.y > 1.0)
//		{
//			offset.y = (posI.y - 1) % 2 + posF.y;
//			pos.y = sign(pos.y) * offset.y + sign(pos.y) * -1;
//		}
//	}
//
//	watervout.PosL = float3(pos, watervout.PosL.z);
//
//	return watervout;
//}
//
//VS_CONTROL_POINT_OUTPUT VS(VertexIn vin)
//{
//	//VertexOut vout = (VertexOut)0.0f;
//	//// Fetch the material data.
//	//MaterialData matData = gMaterialData[gMaterialIndex];
//	////vin = Water(vin);
//	//// Transform to world space.
//	//float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
//	//vout.PosW = posW.xyz;
//	//// Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
//	//vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);
//	//vout.TangentW = mul(vin.TangentU, (float3x3)gWorld);
//	//// Transform to homogeneous clip space.
//	//vout.PosH = mul(posW, gViewProj);
//	//// Output vertex attributes for interpolation across triangle.
//	//float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
//	//vout.TexC = mul(texC, matData.MatTransform).xy;
//	//// Generate projective tex-coords to project shadow map onto scene.
//	////将点投影到阴影图中
//	//vout.MaterialId = vin.MaterialId;
//
//	VS_CONTROL_POINT_OUTPUT output;
//	output.pos = vin.PosL;
//	output.texCoord = vin.TexC;
//
//	return output;
//}
//
//struct DS_OUTPUT
//{
//	float4 pos : SV_POSITION;
//	float2 texCoord : TEXCOORD;
//	float3 PosW : POSITION;
//};
//
//float4 PS(DS_OUTPUT pin) : SV_TARGET
//{
//	return float4(pin.PosW.x,pin.PosW.y,pin.PosW.z,0.5);
//}
//
//#define NUM_CONTROL_POINTS_INPUT 4
//#define NUM_CONTROL_POINTS_OUTPUT 4
//#define PI 3.14159265359
//#define HALF_PI 1.57079632679
//#define EPSILON 0.00024414;
//
//struct HS_CONTROL_POINT_OUTPUT
//{
//	float3 pos : MODELPOS;
//	float2 texCoord : TEXCOORD;
//};
//
//struct HS_CONSTANT_DATA_OUTPUT
//{
//	float EdgeTessFactor[4] : SV_TessFactor;
//	float InsideTessFactor[2] : SV_InsideTessFactor;
//};
//
//// Patch Constant Function
//HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
//	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS_INPUT> ip,
//	uint PatchID : SV_PrimitiveID)
//{
//	HS_CONSTANT_DATA_OUTPUT Output;
//
//	// Insert code to compute Output here
//	Output.EdgeTessFactor[0] =
//		Output.EdgeTessFactor[1] =
//		Output.EdgeTessFactor[2] =
//		Output.EdgeTessFactor[3] = 7;
//	Output.InsideTessFactor[0] =
//		Output.InsideTessFactor[1] = 5;
//
//	return Output;
//}
//
//
//
//float2 BLERP2(float2 v00, float2 v01, float2 v10, float2 v11, float2 uv)
//{
//	return lerp(lerp(v00, v01, float2(uv.y, uv.y)), lerp(v10, v11, float2(uv.y, uv.y)), float2(uv.x, uv.x));
//}
//
//float3 BLERP3(float3 v00, float3 v01, float3 v10, float3 v11, float2 uv)
//{
//	return lerp(lerp(v00, v01, float3(uv.y, uv.y, uv.y)), lerp(v10, v11, float3(uv.y, uv.y, uv.y)), float3(uv.x, uv.x, uv.x));
//}
//
//float4 Flow(in float2 uv, in float time, in Texture2D flowT, in SamplerState flowS, in Texture2D flowedT, in SamplerState flowedS)
//{
//	float timeInt = float(time) / (1 * 2); //interval is always 1
//	float2 fTime = frac(float2(timeInt, timeInt + .5));
//	float4 flowMap = flowT.SampleLevel(flowS, uv, 0);
//	float2 flowDir = -flowMap.xy;//(flowMap.xy - float2(0.5, 0.5)) * 2;
//	float2 flowUV1 = uv - (flowDir / 2) + fTime.x * flowDir.xy;
//	float2 flowUV2 = uv - (flowDir / 2) + fTime.y * flowDir.xy;
//	float4 tx1 = flowedT.SampleLevel(flowedS, flowUV1, 0);
//	float4 tx2 = flowedT.SampleLevel(flowedS, flowUV2, 0);
//	return lerp(tx1, tx2, abs(2 * frac(timeInt) - 1));
//}
//
//[domain("quad")]
//[partitioning("integer")]
//[outputtopology("triangle_ccw")]
//[outputcontrolpoints(NUM_CONTROL_POINTS_OUTPUT)]
//[patchconstantfunc("CalcHSPatchConstants")]
//HS_CONTROL_POINT_OUTPUT HS(
//	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS_INPUT> ip,
//	uint i : SV_OutputControlPointID,
//	uint PatchID : SV_PrimitiveID)
//{
//	HS_CONTROL_POINT_OUTPUT Output;
//
//	// Insert code to compute Output here
//	Output.pos = ip[i].pos;
//	Output.texCoord = ip[i].texCoord;
//
//	return Output;
//}
//
//
//[domain("quad")]
//DS_OUTPUT DS(
//	HS_CONSTANT_DATA_OUTPUT input,
//	float2 domain : SV_DomainLocation, // float2 for quad
//	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS_OUTPUT> patch)
//{
//	DS_OUTPUT Output;
//
//	float3 pos = BLERP3(patch[0].pos, patch[1].pos, patch[3].pos, patch[2].pos, domain);
//
//	float2 texCoord = BLERP2(patch[0].texCoord, patch[1].texCoord, patch[3].texCoord, patch[2].texCoord, domain);
//	//float3 nor = float3(0, 1, 0); //PER VERTEX NORMAL
//
//	float time = 1.0;
//	float timeScale = 1.0;
//	float flowSpeed = 1.0;
//
//		float4 deviation = Flow(texCoord, time * timeScale * flowSpeed, WaterFlowMap1/*flowmap*/, gsamPointWrap, WaterFlowMap2/*verticalFilter1*/, gsamPointWrap);
//		//float4 deviation = FlowHeightWithNormal(texCoord, time * timeScale * flowSpeed, t1, s2, t2, s2, nor); //PER VERTEX NORMAL
//		pos.y += deviation.y;
//		pos.x += deviation.x;
//		pos.z += deviation.z;
//
//	Output.PosW = mul(float4(pos, 1.0f), gWorld);
//	Output.pos = mul(Output.PosW, gViewProj);
//	Output.texCoord = texCoord;
//	
//	//Output.nor = nor; //PER VERTEX NORMAL
//
//	return Output;
//}
//
