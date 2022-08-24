
SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);


float3 mod289(float3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
float2 mod289(float2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
float3 permute(float3 x) { return mod289(((x * 34.0) + 1.0) * x); }

float snoise(float2 v)
{
	// Precompute values for skewed triangular grid
	const float4 C = float4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
		0.366025403784439,	// 0.5*(sqrt(3.0)-1.0)
		-0.577350269189626, // -1.0 + 2.0 * C.x
		0.024390243902439	// 1.0 / 41.0
		);


	// First corner (x0)
	float2 i = floor(v + dot(v, C.yy));
	float2 x0 = v - i + dot(i, C.xx);

	// Other two corners (x1, x2)
	float2 i1;
	i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
	float2 x1 = x0.xy + C.xx - i1;
	float2 x2 = x0.xy + C.zz;

	// Do some permutations to avoid
	// truncation effects in permutation
	i = mod289(i);
	float3 p = permute(
		permute(i.y + float3(0.0, i1.y, 1.0))
		+ i.x + float3(0.0, i1.x, 1.0));

	float3 m = max(0.5 - float3(
		dot(x0, x0),
		dot(x1, x1),
		dot(x2, x2)
		), 0.0);

	m = m * m;
	m = m * m;

	// Gradients: 
	//  41 pts uniformly over a line, mapped onto a diamond
	//  The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

	float3 x = 2.0 * frac(p * C.www) - 1.0;
	float3 h = abs(x) - 0.5;
	float3 ox = floor(x + 0.5);
	float3 a0 = x - ox;

	// Normalise gradients implicitly by scaling m
	// Approximation of: m *= inversesqrt(a0*a0 + h*h);
	m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);

	// Compute final noise value at P
	float3 g;
	g.x = a0.x * x0.x + h.x * x0.y;
	g.yz = a0.yz * float2(x1.x, x2.x) + h.yz * float2(x1.y, x2.y);
	return 130.0 * dot(m, g);
}

float2 GetNoise(in float2 position, in float2 timedWindDir)
{
	float2 noise;
	noise.x = snoise(position * 0.015 + timedWindDir * 0.0005); // large and slower noise 
	noise.y = snoise(position * 0.1 + timedWindDir * 0.002); // smaller and faster noise
	return saturate(noise);
}

/////////////water



float3 GerstnerWaveValues(float2 position, float2 D, float amplitude, float wavelength, float Q, float timer)
{
	float w = 2 * 3.14159265 / wavelength;
	float dotD = dot(position, D);
	float v = w * dotD + timer;
	return float3(cos(v), sin(v), w);
}

half3 GerstnerWaveNormal(float2 D, float A, float Q, float3 vals)
{
	half C = vals.x;
	half S = vals.y;
	half w = vals.z;
	half WA = w * A;
	half WAC = WA * C;
	half3 normal = half3(-D.x * WAC, 1.0 - Q * WA * S, -D.y * WAC);
	return normalize(normal);
}

half3 GerstnerWaveTangent(float2 D, float A, float Q, float3 vals)
{
	half C = vals.x;
	half S = vals.y;
	half w = vals.z;
	half WA = w * A;
	half WAS = WA * S;
	half3 normal = half3(Q * -D.x * D.y * WAS, D.y * WA * C, 1.0 - Q * D.y * D.y * WAS);
	return normalize(normal);
}

float3 GerstnerWaveDelta(float2 D, float A, float Q, float3 vals)
{
	float C = vals.x;
	float S = vals.y;
	float QAC = Q * A * C;
	return float3(QAC * D.x, A * S, QAC * D.y);
}

void GerstnerWave(float2 windDir, float tiling, float amplitude, float wavelength, float Q, float timer, inout float3 position, out half3 normal)
{
	float2 D = windDir;
	float3 vals = GerstnerWaveValues(position.xz * tiling, D, amplitude, wavelength, Q, timer);
	normal = GerstnerWaveNormal(D, amplitude, Q, vals);
	position += GerstnerWaveDelta(D, amplitude, Q, vals);
}

float3 SineWaveValues(float2 position, float2 D, float amplitude, float wavelength, float timer)
{
	float w = 2 * 3.14159265 / wavelength;
	float dotD = dot(position, D);
	float v = w * dotD + timer;
	return float3(cos(v), sin(v), w);
}

half3 SineWaveNormal(float2 D, float A, float3 vals)
{
	half C = vals.x;
	half w = vals.z;
	half WA = w * A;
	half WAC = WA * C;
	half3 normal = half3(-D.x * WAC, 1.0, -D.y * WAC);
	return normalize(normal);
}

half3 SineWaveTangent(float2 D, float A, float3 vals)
{
	half C = vals.x;
	half w = vals.z;
	half WAC = w * A * C;
	half3 normal = half3(0.0, D.y * WAC, 1.0);
	return normalize(normal);
}

float SineWaveDelta(float A, float3 vals)
{
	return vals.y * A;
}

void SineWave(float2 windDir, float tiling, float amplitude, float wavelength, float timer, inout float3 position, out half3 normal)
{
	float2 D = windDir;
	float3 vals = SineWaveValues(position.xz * tiling, D, amplitude, wavelength, timer);
	normal = SineWaveNormal(D, amplitude, vals);
	position.y += SineWaveDelta(amplitude, vals);
}

void AdjustWavesValues(in float2 noise, inout half4 wavesNoise, inout half4 wavesIntensity)
{
	wavesNoise = wavesNoise * half4(noise.y * 0.25, noise.y * 0.25, noise.x + noise.y, noise.y);
	wavesIntensity = wavesIntensity + half4(saturate(noise.y - noise.x), noise.x, noise.y, noise.x + noise.y);
	wavesIntensity = clamp(wavesIntensity, 0.01, 10);
}

float ComputeNoiseHeight(Texture2D heightTexture, float4 wavesIntensity, float4 wavesNoise, float2 texCoord, float2 noise, float2 timedWindDir)
{
	AdjustWavesValues(noise, wavesNoise, wavesIntensity);

	float2 texCoords[4] = { texCoord * 1.6 + timedWindDir * 0.064 + wavesNoise.x,
							texCoord * 0.8 + timedWindDir * 0.032 + wavesNoise.y,
							texCoord * 0.5 + timedWindDir * 0.016 + wavesNoise.z,
							texCoord * 0.3 + timedWindDir * 0.008 + wavesNoise.w };
	float height = 0;
	for (int i = 0; i < 4; ++i)
	{
		float4 HT = heightTexture.SampleLevel(gsamPointClamp, texCoords[i], 0);
		height += HT.x * wavesIntensity[i];
	}

	return height;
}

float3 ComputeDisplacement(float3 worldPos, float cameraDistance, float2 noise, float timer,
	float4 waveSettings, float4 waveAmplitudes, float4 wavesIntensity, float4 waveNoise,
	out half3 normal, out half3 tangent)
{
	float2 windDir = waveSettings.xy;
	float waveSteepness = waveSettings.z;
	float waveTiling = waveSettings.w;

	//TODO: improve motion/simulation instead of just noise
	//TODO: fix UV due to wave distortion

	wavesIntensity = normalize(wavesIntensity);
	waveNoise = half4(noise.x - noise.x * 0.2 + noise.y * 0.1, noise.x + noise.y * 0.5 - noise.y * 0.1, noise.x, noise.x) * waveNoise;
	half4 wavelengths = half4(1, 4, 3, 6) + waveNoise;
	half4 amplitudes = waveAmplitudes + half4(0.5, 1, 4, 1.5) * waveNoise;

	// reduce wave intensity base on distance to reduce aliasing
	wavesIntensity *= 1.0 - saturate(half4(cameraDistance / 120.0, cameraDistance / 150.0, cameraDistance / 170.0, cameraDistance / 400.0));

	// compute position and normal from several sine and gerstner waves
	tangent = normal = half3(0, 1, 0);
	float2 timers = float2(timer * 0.5, timer * 0.25);
	for (int i = 2; i < 4; ++i)
	{
		float A = wavesIntensity[i] * amplitudes[i];
		float3 vals = SineWaveValues(worldPos.xz * waveTiling, windDir, A, wavelengths[i], timer);
		normal += wavesIntensity[i] * SineWaveNormal(windDir, A, vals);
		tangent += wavesIntensity[i] * SineWaveTangent(windDir, A, vals);
		worldPos.y += SineWaveDelta(A, vals);
	}

	// using normalized wave steepness, tranform to Q
	float2 Q = waveSteepness / ((2 * 3.14159265 / wavelengths.xy) * amplitudes.xy);
	for (int j = 0; j < 2; ++j)
	{
		float A = wavesIntensity[j] * amplitudes[j];
		float3 vals = GerstnerWaveValues(worldPos.xz * waveTiling, windDir, A, wavelengths[j], Q[j], timer);
		normal += wavesIntensity[j] * GerstnerWaveNormal(windDir, A, Q[j], vals);
		tangent += wavesIntensity[j] * GerstnerWaveTangent(windDir, A, Q[j], vals);
		worldPos += GerstnerWaveDelta(windDir, A, Q[j], vals);
	}

	normal = normalize(normal);
	tangent = normalize(tangent);
	if (length(wavesIntensity) < 0.01)
	{
		normal = half3(0, 1, 0);
		tangent = half3(0, 0, 1);
	}

	return worldPos;
}

// Returns the surface normal using screen-space partial derivatives of world position.
// Will result in hard shading normals.
float3 UnpackNormal(float4 n)
{
	n.xyz = n.xyz * 2.0 - 1.0;
	return n.xyz;
}

float3 ComputeSurfaceNormal(float3 normal, float3 tangent, float3 bitangent, Texture2D tex, float2 uv)
{
	float3x3 tangentFrame = float3x3(normalize(bitangent), normalize(tangent), normal);

	float2 duv1 = ddx(uv) * 2;
	float2 duv2 = ddy(uv) * 2;

	normal = UnpackNormal(tex.SampleGrad(gsamPointClamp, uv, duv1, duv2));

	return normalize(mul(normal, tangentFrame));
}

float3 ComputeNormal(Texture2D normalTexture, float2 worldPos, float2 texCoord,
	half3 normal, half3 tangent, half3 bitangent,
	half4 wavesNoise, half4 wavesIntensity, float2 timedWindDir)
{
	float2 noise = GetNoise(worldPos, timedWindDir * 0.5);
	AdjustWavesValues(noise, wavesNoise, wavesIntensity);

	float2 texCoords[4] = { texCoord * 1.6 + timedWindDir * 0.064 + wavesNoise.x,
		texCoord * 0.8 + timedWindDir * 0.032 + wavesNoise.y,
		texCoord * 0.5 + timedWindDir * 0.016 + wavesNoise.z,
		texCoord * 0.3 + timedWindDir * 0.008 + wavesNoise.w };

	float3 wavesNormal = float3(0, 1, 0);

	normal = normalize(normal);
	tangent = normalize(tangent);
	bitangent = normalize(bitangent);


	return ComputeSurfaceNormal(normal, tangent, bitangent, normalTexture, texCoords[0]);
	//????
	//for (int i = 0; i < 4; ++i)
	//{
	//	  wavesNormal += ComputeSurfaceNormal(normal, tangent, bitangent, normalTexture, texCoords[i]) * wavesIntensity[i];
	//}
	//return wavesNormal;
}

float4 ClipToScreenPos(float4 pos)
{
	float4 o = pos * 0.5f;
	o.xy += o.w;
	o.zw = pos.zw;
	return o;
}
